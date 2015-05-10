## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(BDTOOLS_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

lib_LTLIBRARIES = libbd-1.0.la

libbd_1_0_la_SOURCES = \
	mpls_parse.c \
	clpi_parse.c \
	util.h \
	bits.h \
	mpls_parse.h \
	clpi_parse.h

libbd_1_0_la_LDFLAGS = -version-info $(LT_VERSION_INFO)

library_includedir=$(includedir)/bdtools-$(BDTOOLS_API_VERSION)/bdtools
library_include_HEADERS = util.h mpls_parse.h clpi_parse.h

bdtools_configdir = $(libdir)/bdtools-$(BDTOOLS_API_VERSION)/include

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = bdtools-$(BDTOOLS_API_VERSION).pc

