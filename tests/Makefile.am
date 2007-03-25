LIBS = $(REPTOOL_LIBS)

AM_CPPFLAGS = $(REPTOOL_CFLAGS) \
              -I$(top_srcdir)/src

libreptool = $(top_builddir)/src/libreptool.la

noinst_PROGRAMS = test_rptprint \
                  test_rptreport

LDADD = $(libreptool)

EXTRA_DIST = \
             test_report.rpt \
             test_report_db.rpt \
             test_rptprint.rptr
