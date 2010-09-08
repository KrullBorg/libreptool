LIBS = $(REPTOOL_LIBS)

AM_CPPFLAGS = $(REPTOOL_CFLAGS) \
              -DG_LOG_DOMAIN=\"libreptool\"

parser.tab.c parser.tab.h: parser.y
	bison -d parser.y

laxycal.yy.c lexycal.yy.h: lexycal.fl
	flex -o lexycal.yy.c --header-file=lexycal.yy.h lexycal.fl

lib_LTLIBRARIES = libreptool.la

libreptool_la_LDFLAGS = -no-undefined

libreptool_la_SOURCES = \
                        rptobject.c \
                        rptobjecttext.c \
                        rptobjectline.c \
                        rptobjectrect.c \
                        rptobjectellipse.c \
                        rptobjectimage.c \
                        rptreport.c \
                        rptprint.c \
                        rptcommon.c \
                        rptmarshal.c \
                        parser.tab.c \
                        lexycal.yy.c

libreptool_include_HEADERS = \
                  libreptool.h \
                  rptobject.h \
                  rptobjecttext.h \
                  rptobjectline.h \
                  rptobjectrect.h \
                  rptobjectellipse.h \
                  rptobjectimage.h \
                  rptreport.h \
                  rptprint.h \
                  rptcommon.h

noinst_HEADERS = \
                 rptreport_priv.h \
                 rptmarshal.h \
                 parser.tab.h \
                 lexycal.yy.h

EXTRA_DIST = \
             parser.y \
             lexycal.fl

libreptool_includedir = $(includedir)/libreptool
