LIBS = $(REPTOOL_LIBS) \
       -lfl

AM_CPPFLAGS = $(REPTOOL_CFLAGS)

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

include_HEADERS = \
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
