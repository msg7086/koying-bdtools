## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(BDTOOLS_CFLAGS) -I$(top_srcdir)

BDTOOLS_LIB = $(top_builddir)/bdtools/libbd-1.0.la

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = bdsplice mpls_dump clpi_dump 

bdsplice_SOURCES = \
	bdsplice.c util.c

bdsplice_LDADD = $(BDTOOLS_LIBS) $(BDTOOLS_LIB)

mpls_dump_SOURCES = \
	mpls_dump.c util.c

mpls_dump_LDADD = $(BDTOOLS_LIBS) $(BDTOOLS_LIB)

clpi_dump_SOURCES = \
	clpi_dump.c util.c

clpi_dump_LDADD = $(BDTOOLS_LIBS) $(BDTOOLS_LIB)

