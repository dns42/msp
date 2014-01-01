#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/lua.h>
#include <mcc/nrx.h>

#include <crt/log.h>
#include <crt/list.h>

#include <stdlib.h>
#include <assert.h>
#include <netdb.h>
#include <sys/socket.h>

struct lua_NetRX {
    struct nrx *c;
    struct list events;
};

static struct lua_NetRX *
lua_netrx_check(struct lua_State *L, int n)
{
    struct lua_NetRX *R;

    R = luaL_checkudata(L, n, "NetRX");
    luaL_argcheck(L, R != NULL, n, "`NetRX' expected");

    return R;
}

static int
lua_netrx_new(struct lua_State *L, struct nrx *nrx)
{
    struct lua_NetRX *R;

    R = lua_newuserdata(L, sizeof(*R));
    luaL_getmetatable(L, "NetRX");
    lua_setmetatable(L, -2);

    R->c = nrx;
    list_init(&R->events);

    nrx_plug(nrx, g_loop);

    return 1;
}

static int
lua_netrx_bind(struct lua_State *L)
{
    const char *astr;
    struct sockaddr_in addr;
    int nchn, vers, t;
    struct nrx *nrx;

    nchn = 8;
    astr = NULL;
    vers = 0;

    t = lua_gettop(L);
    switch (t) {
    default:
    case 3:
        vers = luaL_checknumber(L, t--);
    case 2:
        astr = lua_isnil(L, t)
            ? t--, NULL
            : luaL_checkstring(L, t--);
    case 1:
        nchn = luaL_checknumber(L, 1);
    case 0:
        break;
    }

    addr = (struct sockaddr_in) {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = 0,
    };

    if (astr) {
        char *host, *port = strdup(astr);
        struct addrinfo *ai;
        int rc;

        host = strsep(&port, ":");
        luaL_argcheck(L, host && port, 2,
                      "'<host>:<port> tuple expected");

        rc = getaddrinfo(host, port, NULL, &ai);
        if (rc)
            luaL_error(L, "getaddrinfo(%s): %s",
                       astr, gai_strerror(rc));

        assert(ai != NULL);
        assert(ai->ai_addrlen == sizeof(addr));

        addr = * (struct sockaddr_in *) ai->ai_addr;

        freeaddrinfo(ai);
        free(host);
    }

    nrx = nrx_bind(nchn, &addr, vers);

    return nrx ? lua_netrx_new(L, nrx) : 0;
}

static int
lua_netrx_close(struct lua_State *L)
{
    struct lua_NetRX *R;

    R = lua_netrx_check(L, 1);

    if (R->c) {
        nrx_close(R->c);
        R->c = NULL;
    }

    return 0;
}

static int
lua_netrx_sockname(struct lua_State *L)
{
    struct lua_NetRX *R;
    struct sockaddr_in addr;
    char host[40];
    int rc;

    R = lua_netrx_check(L, 1);

    if (!R->c)
        return 0;

    rc = nrx_sockname(R->c, &addr);

    if (unexpected(rc))
        return 0;

    rc = getnameinfo((struct sockaddr *) &addr, sizeof(addr),
                     host, sizeof(host),
                     NULL, 0, NI_NUMERICHOST);
    if (unexpected(rc))
        return 0;

    lua_pushfstring(L, "%s:%d", host, ntohs(addr.sin_port));

    return 1;
}

static int
lua_netrx_plugged(struct lua_State *L)
{
    struct lua_NetRX *R;

    R = lua_netrx_check(L, 1);

    lua_pushboolean(L, R->c && nrx_plugged(R->c));

    return 1;
}

static int
lua_netrx_tostring(struct lua_State *L)
{
    struct lua_NetRX *R;

    R = lua_netrx_check(L, 1);

    if (R->c) {
        const char *name;

        lua_netrx_sockname(L);
        name = lua_tostring(L, -1);
        lua_pushfstring(L, "NetRX ('%s')", name);
    } else
        lua_pushstring(L, "NetRX (closed)");

    return 1;
}

static const struct luaL_reg lua_netrx_fn [] = {
    { "bind", lua_netrx_bind },
    { NULL, NULL }
};

static const struct luaL_reg lua_netrx_class [] = {
    { "sockname", lua_netrx_sockname },
    { "close", lua_netrx_close },
    { "plugged", lua_netrx_plugged },
    { NULL, NULL }
};

static const struct luaL_reg lua_netrx_meta [] = {
    { "__tostring", lua_netrx_tostring },
    { "__gc", lua_netrx_close },
    { NULL, NULL }
};

int
luaopen_nrx(struct lua_State *L)
{
    int rc;

    rc = luaL_loadbuffer(L,
                         MCC_LUA_START(nrx), MCC_LUA_LEN(nrx),
                         "nrx.lua");
    assert(!rc);
    lua_call(L, 0, 0);

    luaL_openlib(L, "NetRX", lua_netrx_fn, 0);
    lua_pop(L, 1);

    lua_object_classinit(L, "NetRX",
                         lua_netrx_class,
                         lua_netrx_meta);

    return 0;
}

MCC_LUA_INIT(luaopen_nrx);

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
