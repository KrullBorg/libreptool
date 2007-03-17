#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([libreptool], [0.0.1], [azagli@inwind.it])
AC_CONFIG_SRCDIR([src/rptprint.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_LIBTOOL_WIN32_DLL

AC_CANONICAL_SYSTEM

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
GTK_DOC_CHECK

# Checks for libraries.
PKG_CHECK_MODULES([REPTOOL], [glib-2.0 >= 2.6.0
                              gobject-2.0 >= 2.6.0
                              libxml-2.0 >= 2.6.0
                              libgda >= 1.2.3
                              cairo >= 1.0.0
                              pangocairo >= 1.12])

AC_SUBST(REPTOOL_CFLAGS)
AC_SUBST(REPTOOL_LIBS)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.

AC_CONFIG_FILES([
  libreptool.pc
  Makefile
  src/Makefile
  tests/Makefile
  docs/Makefile
  docs/reference/Makefile
  docs/reference/version.xml
])
AC_OUTPUT
