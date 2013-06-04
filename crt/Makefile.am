
AM_CFLAGS  = -Wmissing-prototypes
AM_CFLAGS += -Wno-format-zero-length

AM_CPPFLAGS  = -I$(top_srcdir)
AM_CPPFLAGS += -I$(top_builddir)

noinst_LTLIBRARIES  = libcrt.la

libcrt_la_SOURCES  = defs.h
libcrt_la_SOURCES += list.h
libcrt_la_SOURCES += timer.c
libcrt_la_SOURCES += timer.h
libcrt_la_SOURCES += timer-internal.h
libcrt_la_SOURCES += evtloop.c
libcrt_la_SOURCES += evtloop.h
libcrt_la_SOURCES += evtloop-internal.h
libcrt_la_SOURCES += tty.c
libcrt_la_SOURCES += tty.h
libcrt_la_SOURCES += tty-internal.h
libcrt_la_SOURCES += log.c
libcrt_la_SOURCES += log.h
libcrt_la_SOURCES += log-internal.h
libcrt_la_SOURCES += log-stdio.c
libcrt_la_SOURCES += log-stdio-internal.h
libcrt_la_SOURCES += log-syslog.c
libcrt_la_SOURCES += log-syslog-internal.h

