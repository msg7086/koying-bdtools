dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(bdtools, 1.0)

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE
AC_CONFIG_MACRO_DIR([m4])

BDTOOLS_API_VERSION=1.0
LT_VERSION_INFO=1:0:0


AC_SUBST(BDTOOLS_API_VERSION)
AC_SUBST(LT_VERSION_INFO)

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_STDC
AC_HEADER_STDC

AC_SYS_LARGEFILE

# overwrite global variable (used for Makefile generation)
AC_SUBST(GLOBALCXXFLAGS, $CXXFLAGS )
AC_SUBST(GLOBALLDFLAGS, $LDFLAGS )

AM_PROG_LIBTOOL

dnl ------------------------------------------------------------------------
dnl Below are checks to determine how to do shared object support
dnl for making shared libraries and stuff like that.  This has been borrowed
dnl or based on various projects I've seen floating around.
dnl ------------------------------------------------------------------------
CCDLL="${CC}"
tldf=$LDFLAGS
LDFLAGS="${LDFLAGS} -Wl,-export-dynamic"
AC_MSG_CHECKING([for the -export-dynamic ld flag])
AC_TRY_LINK([], [int i;], [found=yes], [found=no])
AC_MSG_RESULT([$found])
if test "$found" = "no" ; then
	LDFLAGS=$tldf
fi

AC_OUTPUT([
Makefile
bdtools/Makefile
bdtools/bdtools-1.0.pc
src/Makefile
])
