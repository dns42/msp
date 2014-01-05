
AM_CFLAGS  = -Wmissing-prototypes

AM_CPPFLAGS  = -I$(top_srcdir)
AM_CPPFLAGS += -I$(top_builddir)
AM_CPPFLAGS += $(LUA_CFLAGS)

bin_PROGRAMS  = mcc

mcc_SOURCES  = main.c
mcc_SOURCES += js.c
mcc_SOURCES += js.h
mcc_SOURCES += js-internal.h
mcc_SOURCES += ntx.c
mcc_SOURCES += ntx.h
mcc_SOURCES += ntx-internal.h
mcc_SOURCES += nrx.c
mcc_SOURCES += nrx.h
mcc_SOURCES += nrx-internal.h
mcc_SOURCES += mcc.c
mcc_SOURCES += mcc.h
mcc_SOURCES += mcc-internal.h

mcc_LDADD  = $(builddir)/libmcc-rpc.la
mcc_LDADD += $(builddir)/libmcc-lua.la
mcc_LDADD += $(builddir)/libmcc-lbc.a
mcc_LDADD += $(top_builddir)/msp/libmsp.la
mcc_LDADD += $(top_builddir)/crt/libcrt.la
mcc_LDADD += $(LUA_LIBS)

noinst_LTLIBRARIES  = libmcc-rpc.la
noinst_LTLIBRARIES += libmcc-lua.la

libmcc_lua_la_SOURCES  = lua.c
libmcc_lua_la_SOURCES += lua.h
libmcc_lua_la_SOURCES += lua-object.c
libmcc_lua_la_SOURCES += lua-joystick.c
libmcc_lua_la_SOURCES += lua-rcvec.c
libmcc_lua_la_SOURCES += lua-rcvec.h
libmcc_lua_la_SOURCES += lua-nettx.c
libmcc_lua_la_SOURCES += lua-netrx.c
libmcc_lua_la_SOURCES += lua-msp.c
libmcc_lua_la_SOURCES += lua-rmi.c
libmcc_lua_la_SOURCES += lua-mcc.c
libmcc_lua_la_SOURCES += lua-event.c
libmcc_lua_la_SOURCES += lua-event.h

include $(top_srcdir)/mk/lua.mk

noinst_LIBRARIES = libmcc-lbc.a

libmcc_lbc_a_SOURCES  =

libmcc_lbc_a_LIBADD  = object.lbc.o
libmcc_lbc_a_LIBADD += event.lbc.o
libmcc_lbc_a_LIBADD += joystick.lbc.o
libmcc_lbc_a_LIBADD += rcvec.lbc.o
libmcc_lbc_a_LIBADD += nettx.lbc.o
libmcc_lbc_a_LIBADD += netrx.lbc.o
libmcc_lbc_a_LIBADD += msp.lbc.o
libmcc_lbc_a_LIBADD += rmi.lbc.o

include $(top_srcdir)/mk/rpc.mk

nodist_libmcc_rpc_la_SOURCES  = nrx_rpc.h
nodist_libmcc_rpc_la_SOURCES += nrx_rpc_xdr.c
nodist_libmcc_rpc_la_SOURCES += rmi_rpc.h
nodist_libmcc_rpc_la_SOURCES += rmi_rpc_xdr.c

libmcc_rpc_la_CFLAGS  = $(AM_CFLAGS)
libmcc_rpc_la_CFLAGS += -Wno-unused-variable

libmcc_rpc_la_CPPFLAGS  = $(AM_CPPFLAGS)
libmcc_rpc_la_CPPFLAGS += -I$(builddir)

EXTRA_DIST  = nrx_rpc.x
EXTRA_DIST += rc.lua
EXTRA_DIST += tx.lua
EXTRA_DIST += js.lua
EXTRA_DIST += object.lua
EXTRA_DIST += msp.lua

CLEANFILES  = *.lbc.o
CLEANFILES += $(nodist_libnrxrpc_la_SOURCES)

