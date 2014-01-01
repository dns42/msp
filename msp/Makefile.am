
AM_CFLAGS  = -Wall
AM_CFLAGS += -Werror
AM_CFLAGS += -Wmissing-prototypes

AM_CPPFLAGS  = -I$(top_srcdir)
AM_CPPFLAGS += -I$(top_builddir)

lib_LTLIBRARIES  = libmsp.la

libmsp_la_SOURCES  = defs.h
libmsp_la_SOURCES += msg.c
libmsp_la_SOURCES += msg-internal.h
libmsp_la_SOURCES += msp.c
libmsp_la_SOURCES += msp-internal.h
libmsp_la_SOURCES += str.c
libmsp_la_SOURCES += meta.c

libmsp_la_LIBADD  = ../crt/libcrt.la

libmsp_includedir = $(includedir)/msp

libmsp_include_HEADERS  = msg.h
libmsp_include_HEADERS += msp.h
libmsp_include_HEADERS += str.h
libmsp_include_HEADERS += meta.h

bin_PROGRAMS  = msp

msp_SOURCES  = cmd.c
msp_SOURCES += cmd.h
msp_SOURCES += cli.c

msp_LDADD  = libmsp.la
