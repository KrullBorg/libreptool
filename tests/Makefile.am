LIBS = $(REPTOOL_LIBS)

AM_CPPFLAGS = $(REPTOOL_CFLAGS) \
              -I$(top_srcdir)/src

libreptool = $(top_builddir)/src/libreptool.la

noinst_PROGRAMS = \
                  rptreport \
                  rptprint \
                  creation \
                  liststore \
                  gtktreeview

LDADD = $(libreptool)

EXTRA_DIST = \
             report.rpt \
             db.rpt \
             mm.rptr \
             newline.rpt \
             db_test.db \
             gnome-globe.png
