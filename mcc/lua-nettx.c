#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/lua.h>
#include <mcc/lua-rcvec.h>
#include <mcc/ntx.h>

#include <crt/log.h>
#include <crt/list.h>

#include <stdlib.h>
#include <assert.h>
#include <netdb.h>
#include <sys/socket.h>

struct lua_NetTX {
    struct ntx *c;
};

static struct lua_NetTX *
lua_nettx_check(struct lua_State *L, int n)
{
    struct lua_NetTX *T;

    T = luaL_checkudata(L, n, "NetTX");
    luaL_argcheck(L, T != NULL, n, "`NetTX' expected");

    return T;
}

static int
lua_nettx_new(struct lua_State *L, struct ntx *ntx)
{
    struct lua_NetTX *T;

    T = lua_newuserdata(L, sizeof(*T));
    luaL_getmetatable(L, "NetTX");
    lua_setmetatable(L, -2);

    T->c = ntx;

    return 1;
}

static int
lua_nettx_connect(struct lua_State *L)
{
    const char *astr;
    struct sockaddr_in addr;
    int t, vers;
    struct ntx *ntx;

    astr = NULL;
    vers = 0;

    t = lua_gettop(L);
    switch (t) {
    default:
    case 2:
        vers = luaL_checknumber(L, t--);
    case 1:
        astr = lua_isnil(L, t)
            ? t--, NULL
            : luaL_checkstring(L, t--);
    case 0:
        break;
    }

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

    ntx = ntx_open(&addr, vers);

    return ntx ? lua_nettx_new(L, ntx) : 0;
}

static int
lua_nettx_close(struct lua_State *L)
{
    struct lua_NetTX *T;

    T = lua_nettx_check(L, 1);

    if (T->c) {
        ntx_close(T->c);
        T->c = NULL;
    }

    return 0;
}

static int
lua_nettx_pushaddr(struct lua_State *L,
                   const struct sockaddr_in *addr)
{
    char host[40];
    int rc;

    rc = getnameinfo((struct sockaddr *) addr, sizeof(*addr),
                     host, sizeof(host),
                     NULL, 0, NI_NUMERICHOST);
    if (rc) {
        warn("getnameinfo: code %d", rc);
        return 0;
    }

    lua_pushfstring(L, "%s:%d", host, ntohs(addr->sin_port));

    return 1;
}

static int
lua_nettx_sockname(struct lua_State *L)
{
    struct lua_NetTX *T;
    struct sockaddr_in addr;
    int rc;

    T = lua_nettx_check(L, 1);

    if (!T->c)
        return 0;

    rc = ntx_sockname(T->c, &addr);

    return unexpected(rc)
        ? 0
        : lua_nettx_pushaddr(L, &addr);
}

static int
lua_nettx_peername(struct lua_State *L)
{
    struct lua_NetTX *T;
    struct sockaddr_in addr;
    int rc;

    T = lua_nettx_check(L, 1);

    if (!T->c)
        return 0;

    rc = ntx_peername(T->c, &addr);

    return unexpected(rc)
        ? 0
        : lua_nettx_pushaddr(L, &addr);
}

static int
lua_nettx_send(struct lua_State *L)
{
    struct lua_NetTX *T;
    struct lua_RCVec *V;

    T = lua_nettx_check(L, 1);
    V = lua_rcvec_check(L, 2);

    if (!T->c)
        return 0;

    ntx_send(T->c, V->val, V->len);

    return 0;
}

static int
lua_nettx_tostring(struct lua_State *L)
{
    struct lua_NetTX *T;

    T = lua_nettx_check(L, 1);

    if (T->c) {
        const char *name;
        const char *peer;

        lua_nettx_sockname(L);

        name = lua_tostring(L, -1);
        if (name) {
            lua_nettx_peername(L);
            peer = lua_tostring(L, -1);
            if (peer)
                lua_pushfstring(L, "NetTX ('%s' -> '%s')", name, peer);
        }
    } else
        lua_pushstring(L, "NetTX (closed)");

    return 1;
}

static const struct luaL_reg lua_nettx_fn [] = {
    { "connect", lua_nettx_connect },
    { NULL, NULL }
};

static const struct luaL_reg lua_nettx_class [] = {
    { "sockname", lua_nettx_sockname },
    { "peername", lua_nettx_peername },
    { "send", lua_nettx_send },
    { "close", lua_nettx_close },
    { NULL, NULL }
};

static const struct luaL_reg lua_nettx_meta [] = {
    { "__tostring", lua_nettx_tostring },
    { "__gc", lua_nettx_close },
    { NULL, NULL }
};

int
luaopen_nettx(struct lua_State *L)
{
    int rc;

    rc = luaL_loadbuffer(L,
                         MCC_LUA_START(nettx), MCC_LUA_LEN(nettx),
                         "nettx.lua");
    assert(!rc);
    lua_call(L, 0, 0);

    luaL_openlib(L, "NetTX", lua_nettx_fn, 0);
    lua_pop(L, 1);

    lua_object_classinit(L, "NetTX",
                         lua_nettx_class,
                         lua_nettx_meta);

    return 0;
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
