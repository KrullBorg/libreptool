LIBS = $(REPTOOL_LIBS)

AM_CPPFLAGS = $(REPTOOL_CFLAGS) \
              -I$(top_builddir)/src

libreptool = $(top_builddir)/src/libreptool.la

noinst_PROGRAMS = test_rptprint \
                  test_rptreport

LDADD = $(libreptool)
