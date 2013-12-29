
AM_CFLAGS  = -Wmissing-prototypes

AM_CPPFLAGS  = -I$(top_srcdir)
AM_CPPFLAGS += -I$(top_builddir)
AM_CPPFLAGS += $(LUA_CFLAGS)

bin_PROGRAMS  = mcc

mcc_SOURCES  = main.c
mcc_SOURCES += lua.c
mcc_SOURCES += lua.h
mcc_SOURCES += object-lua.c
mcc_SOURCES += js.c
mcc_SOURCES += js.h
mcc_SOURCES += js-internal.h
mcc_SOURCES += js-lua.c
mcc_SOURCES += ntx.c
mcc_SOURCES += ntx.h
mcc_SOURCES += ntx-internal.h
mcc_SOURCES += ntx-lua.c
mcc_SOURCES += nrx.c
mcc_SOURCES += nrx.h
mcc_SOURCES += nrx-internal.h
mcc_SOURCES += nrx-lua.c
mcc_SOURCES += mcc.c
mcc_SOURCES += mcc.h
mcc_SOURCES += mcc-internal.h
mcc_SOURCES += mcc-lua.c

include $(top_srcdir)/mk/lua.mk

mcc_LDADD  = object.lbc.o
mcc_LDADD += js.lbc.o
mcc_LDADD += nrx.lbc.o
mcc_LDADD += ntx.lbc.o
mcc_LDADD += $(top_builddir)/crt/libcrt.la
mcc_LDADD += $(LUA_LIBS)
mcc_LDADD += $(noinst_LTLIBRARIES)

include $(top_srcdir)/mk/rpc.mk

noinst_LTLIBRARIES  = libnrxrpc.la

nodist_libnrxrpc_la_SOURCES  = nrxrpc.h
nodist_libnrxrpc_la_SOURCES += nrxrpc_xdr.c

libnrxrpc_la_CFLAGS  = $(AM_CFLAGS)
libnrxrpc_la_CFLAGS += -Wno-unused-variable

libnrxrpc_la_CPPFLAGS  = $(AM_CPPFLAGS)
libnrxrpc_la_CPPFLAGS += -I$(builddir)

EXTRA_DIST  = nrxrpc.x
EXTRA_DIST += rc.lua
EXTRA_DIST += tx.lua
EXTRA_DIST += js.lua
EXTRA_DIST += object.lua

CLEANFILES  = *.lbc.o
CLEANFILES += $(nodist_libnrxrpc_la_SOURCES)

