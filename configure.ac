#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([libreptool], [0.5.0], [azagli@libero.it])
AC_CONFIG_SRCDIR([src/rptprint.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE(-Wall)

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AM_MAINTAINER_MODE

AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_SYSTEM

LT_INIT

AC_LIBTOOL_WIN32_DLL

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LEX
AC_PROG_LIBTOOL
GTK_DOC_CHECK

dnl ******************************
dnl glib-genmarshal
dnl ******************************
AC_PATH_PROG(GLIB_GENMARSHAL, glib-genmarshal)

# Checks for libraries.
PKG_CHECK_MODULES([REPTOOL], [gtk+-3.0 >= 3.0.0
                              libxml-2.0 >= 2.6.0
                              libgda-5.0 >= 5.0.0])

AC_SUBST(REPTOOL_CFLAGS)
AC_SUBST(REPTOOL_LIBS)

# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_STDC
AC_CHECK_HEADERS([inttypes.h libintl.h malloc.h stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_CHECK_FUNCS([memset strcasecmp strstr strtol])

AC_CONFIG_FILES([
	libreptool.pc
	Makefile
	src/Makefile
	tests/Makefile
	docs/Makefile
	docs/reference/Makefile
	docs/reference/version.xml
	data/Makefile
])
AC_OUTPUT
