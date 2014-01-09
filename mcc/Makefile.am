
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
libmcc_lua_la_SOURCES += lua-bit32.c
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
libmcc_lua_la_SOURCES += lua-timer.c

include $(top_srcdir)/mk/lua.mk

noinst_LIBRARIES = libmcc-lbc.a

libmcc_lbc_a_SOURCES  =

libmcc_lbc_a_LUA  = object.lua
libmcc_lbc_a_LUA += event.lua
libmcc_lbc_a_LUA += timer.lua
libmcc_lbc_a_LUA += joystick.lua
libmcc_lbc_a_LUA += rcvec.lua
libmcc_lbc_a_LUA += nettx.lua
libmcc_lbc_a_LUA += netrx.lua
libmcc_lbc_a_LUA += msp.lua
#libmcc_lbc_a_LUA += multiwii.lua
libmcc_lbc_a_LUA += rmi.lua

libmcc_lbc_a_LIBADD  = $(patsubst %.lua, %.lbc.o, $(libmcc_lbc_a_LUA))

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
EXTRA_DIST += $(libmcc_lbc_a_LUA)

CLEANFILES  = *.lbc.o
CLEANFILES += $(nodist_libnrxrpc_la_SOURCES)

