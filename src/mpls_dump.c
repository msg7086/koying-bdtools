#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include "mpls_parse.h"
#include "util.h"

static int verbose;

typedef struct {
    int value;
    char *str;
} value_map_t;

value_map_t codec_map[] = {
    {0x01, "MPEG-1 Video"},
    {0x02, "MPEG-2 Video"},
    {0x03, "MPEG-1 Audio"},
    {0x04, "MPEG-2 Audio"},
    {0x80, "LPCM"},
    {0x81, "AC-3"},
    {0x82, "DTS"},
    {0x83, "TrueHD"},
    {0x84, "AC-3 Plus"},
    {0x85, "DTS-HD"},
    {0x86, "DTS-HD Master"},
    {0xea, "VC-1"},
    {0x1b, "H.264"},
    {0x90, "Presentation Graphics"},
    {0x91, "Interactive Graphics"},
    {0x92, "Text Subtitle"},
    {0, NULL}
};

value_map_t video_format_map[] = {
    {0, "Reserved"},
    {1, "480i"},
    {2, "576i"},
    {3, "480p"},
    {4, "1080i"},
    {5, "720p"},
    {6, "1080p"},
    {7, "576p"},
    {0, NULL}
};

value_map_t video_rate_map[] = {
    {0, "Reserved1"},
    {1, "23.976"},
    {2, "24"},
    {3, "25"},
    {4, "29.97"},
    {5, "Reserved2"},
    {6, "50"},
    {7, "59.94"},
    {0, NULL}
};

value_map_t audio_format_map[] = {
    {0, "Reserved1"},
    {1, "Mono"},
    {2, "Reserved2"},
    {3, "Stereo"},
    {4, "Reserved3"},
    {5, "Reserved4"},
    {6, "Multi Channel"},
    {12, "Combo"},
    {0, NULL}
};

value_map_t audio_rate_map[] = {
    {0, "Reserved1"},
    {1, "48 Khz"},
    {2, "Reserved2"},
    {3, "Reserved3"},
    {4, "96 Khz"},
    {5, "192 Khz"},
    {12, "48/192 Khz"},
    {14, "48/96 Khz"},
    {0, NULL}
};

char*
_lookup_str(value_map_t *map, int val)
{
    int ii;

    for (ii = 0; map[ii].str; ii++) {
        if (val == map[ii].value) {
            return map[ii].str;
        }
    }
    return "?";
}

static void
_show_marks(char *prefix, MPLS_PL *pl)
{
    int level = 0;
    int ii;
    char current_clip_id[6] = {0};
    int reset_timestamp = 0;
    int current_timestamp = 0;
    static int item_id = 1;
    int chapter_id = 1;
    FILE* fp = NULL;

    for (ii = 0; ii < pl->mark_count; ii++) {
        MPLS_PI *pi;
        MPLS_PLM *plm;
        str_t *clip_id;
        int hour, min;
        double sec;
        int p_hour, p_min;
        double p_sec;

        plm = &pl->play_mark[ii];
        indent_printf(level, "PlayMark %d", ii);
        if (plm->play_item_ref < pl->list_count) {
            pi = &pl->play_item[plm->play_item_ref];
            clip_id = str_substr(pi->clip_id, 0, 5);
            if(current_clip_id[0] == 0 || strncmp(clip_id->buf, current_clip_id, 5) != 0) {
                strncpy(current_clip_id, clip_id->buf, 5);
                reset_timestamp = 1;
            }
            indent_printf(level+1, "PlayItem: %s", clip_id->buf);
            str_free(clip_id);
            free(clip_id);
        } else {
            indent_printf(level+1, "PlayItem: Invalid reference");
        }

        if (reset_timestamp) {
            if (fp)
                fclose(fp);
            if (*prefix) {
                char filename[128];
                strncpy(filename, prefix, 63);
                sprintf(filename + strlen(filename), "_%02d_%sm2ts.txt", item_id, current_clip_id);
                printf("Opening %s\n", filename);
                fp = fopen(filename, "wb");
                if (!fp) {
                    printf("ERROR: unable to open file %s\n", filename);
                    return;
                }
            }
            current_timestamp = plm->abs_start;
            reset_timestamp = 0;
            item_id++;
            chapter_id = 1;
        }

        p_hour = plm->abs_start / (45000*60*60);
        p_min = plm->abs_start / (45000*60) % 60;
        p_sec = (double)(plm->abs_start % (45000 * 60)) / 45000;

        uint32_t abs_start = plm->abs_start - current_timestamp;
        hour = abs_start / (45000*60*60);
        min = abs_start / (45000*60) % 60;
        sec = (double)(abs_start % (45000 * 60)) / 45000;
        indent_printf(level+1, "Abs Time (mm:ss.ms): %02d:%02d:%06.3f (%02d:%02d:%06.3f)", p_hour, p_min, p_sec, hour, min, sec);
        printf("\n");
        fprintf(fp, "CHAPTER%02d=%02d:%02d:%06.3f\nCHAPTER%02dNAME=\n", chapter_id, hour, min, sec, chapter_id);
        chapter_id++;
    }
}

static int
_filter_dup(MPLS_PL *pl_list[], int count, MPLS_PL *pl)
{
    int ii, jj;

    for (ii = 0; ii < count; ii++) {
        if (pl->list_count != pl_list[ii]->list_count ||
            pl->duration != pl_list[ii]->duration) {
            continue;
        }
        for (jj = 0; jj < pl->list_count; jj++) {
            MPLS_PI *pi1, *pi2;

            pi1 = &pl->play_item[jj];
            pi2 = &pl_list[ii]->play_item[jj];

            if (memcmp(pi1->clip_id, pi2->clip_id, 5) != 0 ||
                pi1->in_time != pi2->in_time ||
                pi1->out_time != pi2->out_time) {
                break;
            }
        }
        if (jj != pl->list_count) {
            continue;
        }
        return 0;
    }
    return 1;
}

static int
_find_repeats(MPLS_PL *pl, const char *m2ts)
{
    int ii, count = 0;

    for (ii = 0; ii < pl->list_count; ii++) {
        MPLS_PI *pi;
        str_t *m2ts_file;

        pi = &pl->play_item[ii];
        m2ts_file = str_substr(pi->clip_id, 0, 5);
        // Ignore titles with repeated segments
        if (strcmp(m2ts_file->buf, m2ts) == 0) {
            count++;
        }
        str_free(m2ts_file);
        free(m2ts_file);
    }
    return count;
}

static int
_filter_short(MPLS_PL *pl, int seconds)
{
    // Ignore short playlists
    if (pl->duration / 45000 <= seconds) {
        return 0;
    }
    return 1;
}

static int
_filter_repeats(MPLS_PL *pl, int repeats)
{
    int ii;

    for (ii = 0; ii < pl->list_count; ii++) {
        MPLS_PI *pi;
        str_t *m2ts_file;

        pi = &pl->play_item[ii];
        m2ts_file = str_substr(pi->clip_id, 0, 5);
        // Ignore titles with repeated segments
        if (_find_repeats(pl, m2ts_file->buf) > repeats) {
            return 0;
        }
        str_free(m2ts_file);
        free(m2ts_file);
    }
    return 1;
}

static void
_make_path(str_t *path, char *root, char *dir)
{
    struct stat st_buf;
    char *base;

    base = basename(root);
    if (strcmp(base, dir) == 0) {
        str_printf(path, "%s", root);
    } else if (strcmp(base, "BDMV") != 0) {
        str_printf(path, "%s/BDMV/%s", root, dir);
    } else {
        str_printf(path, "%s/%s", root, dir);
    }

    if (stat(path->buf, &st_buf) || !S_ISDIR(st_buf.st_mode)) {
        str_free(path);
    }
}

static int repeats = 0, seconds = 0, dups = 0;

static MPLS_PL*
_process_file(char *prefix, char *name, MPLS_PL *pl_list[], int pl_count)
{
    MPLS_PL *pl;

    pl = calloc(1, sizeof(MPLS_PL));
    pl = mpls_parse(name, verbose);
    if (pl == NULL) {
        fprintf(stderr, "Parse failed: %s\n", name);
        return NULL;
    }
    if (seconds) {
        if (!_filter_short(pl, seconds)) {
            mpls_free(&pl);
            return NULL;
        }
    }
    if (repeats) {
        if (!_filter_repeats(pl, repeats)) {
            mpls_free(&pl);
            return NULL;
        }
    }
    if (dups) {
        if (!_filter_dup(pl_list, pl_count, pl)) {
            mpls_free(&pl);
            return NULL;
        }
    }
    _show_marks(prefix, pl);
    return pl;
}

static void
_usage(char *cmd)
{
    fprintf(stderr, 
"Usage: %s -vli <mpls file> [<mpls file> ...]\n"
"With no options, produces a list of the playlist(s) with durations\n"
"Options:\n"
"    v             - Verbose output.\n"
"    r <N>         - Filter out titles that have >N repeating clips\n"
"    d             - Filter out duplicate titles\n"
"    s <seconds>   - Filter out short titles\n"
"    f             - Filter combination -r2 -d -s120\n"
"    p <prefix>    - chapter output prefix (63 chars max)\n"
, cmd);

    exit(EXIT_FAILURE);
}

#define OPTS "vfr:ds:p:"

static int
_qsort_str_cmp(const void *a, const void *b)
{
    char *stra = *(char**)a;
    char *strb = *(char**)b;

    return strcmp(stra, strb);
}

int
main(int argc, char *argv[])
{
    MPLS_PL *pl;
    int opt;
    int ii, pl_ii;
    MPLS_PL *pl_list[1000];
    struct stat st;
    str_t path = {0,};
    DIR *dir = NULL;
    char prefix[64] = {0};

    do {
        opt = getopt(argc, argv, OPTS);
        switch (opt) {
            case -1: 
                break;

            case 'v':
                verbose = 1;
                break;

            case 'd':
                dups = 1;
                break;

            case 'r':
                repeats = atoi(optarg);
                break;

            case 'f':
                repeats = 2;
                dups = 1;
                seconds = 120;
                break;

            case 's':
                seconds = atoi(optarg);
                break;

            case 'p':
                strncpy(prefix, optarg, sizeof(prefix) - 1);
                break;

            default:
                _usage(argv[0]);
                break;
        }
    } while (opt != -1);

    if (optind >= argc) {
        _usage(argv[0]);
    }

    for (pl_ii = 0, ii = optind; pl_ii < 1000 && ii < argc; ii++) {
        if (stat(argv[ii], &st)) {
            continue;
        }
        dir = NULL;
        if (S_ISDIR(st.st_mode)) {
            printf("Directory: %s:\n", argv[ii]);
            _make_path(&path, argv[ii], "PLAYLIST");
            if (path.buf == NULL) {
                fprintf(stderr, "Failed to find playlist path: %s\n", argv[ii]);
                continue;
            }
            dir = opendir(path.buf);
            if (dir == NULL) {
                fprintf(stderr, "Failed to open dir: %s\n", path.buf);
                str_free(&path);
                continue;
            }
        }
        if (dir != NULL) {
            char **dirlist = calloc(10001, sizeof(char*));
            struct dirent *ent;
            int jj = 0;
            for (ent = readdir(dir); ent != NULL; ent = readdir(dir)) {
                if (ent->d_name != NULL) {
                    dirlist[jj++] = strdup(ent->d_name);
                }
            }
            qsort(dirlist, jj, sizeof(char*), _qsort_str_cmp);
            for (jj = 0; dirlist[jj] != NULL; jj++) {
                str_t name = {0,};
                str_printf(&name, "%s/%s", path.buf, dirlist[jj]);
                free(dirlist[jj]);
                if (stat(name.buf, &st)) {
                    str_free(&name);
                    continue;
                }
                if (!S_ISREG(st.st_mode)) {
                    str_free(&name);
                    continue;
                }
                pl = _process_file(prefix, name.buf, pl_list, pl_ii);
                str_free(&name);
                if (pl != NULL) {
                    pl_list[pl_ii++] = pl;
                }
            } while (ent != NULL);
            free(dirlist);
            str_free(&path);
        } else {
            pl = _process_file(prefix, argv[ii], pl_list, pl_ii);
            if (pl != NULL) {
                pl_list[pl_ii++] = pl;
            }
        }
    }
    // Cleanup
    for (ii = 0; ii < pl_ii; ii++) {
        mpls_free(&pl_list[ii]);
    }
    return 0;
}

