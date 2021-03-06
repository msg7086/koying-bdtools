#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>

#if defined( __MINGW32__ )
#   undef  lseek
#   define lseek  _lseeki64
#   undef  fseeko
#   define fseeko fseeko64
#   undef  ftello
#   define ftello ftello64
#   define flockfile(...)
#   define funlockfile(...)
#   define getc_unlocked getc
#   undef  off_t
#   define off_t off64_t
#   undef  stat
#   define stat  _stati64
  #ifndef fstat
#   define fstat _fstati64
  #endif
#   define wstat _wstati64
#endif

#define X_FREE(X) { if (X) free(X); }

typedef struct
{
    char * buf;
    int    alloc;
    int    len;
} str_t;

void bdt_hex_dump(uint8_t *buf, int count);

void str_append_sub(str_t *str, char *append, int start, int app_len);
str_t* str_substr(char *str, int start, int len);
void str_append(str_t *str, char *append);
void str_printf(str_t *str, const char *fmt, ...);
void str_free(str_t *str);
void hex_dump(uint8_t *buf, int count);
void indent_printf(int level, char *fmt, ...);
