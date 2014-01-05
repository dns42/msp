#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/lua.h>
#include <msp/msp.h>
#include <msp/meta.h>

#include <stdlib.h>
#include <assert.h>

struct lua_MSP {
    struct msp *c;
    char mtab;
};

struct lua_MSP_Cmd {
    struct lua_State *L;
    struct lua_MSP *M;
    int mspr; /* msp ref */
    int fctr; /* callback ref */
    int ctxr; /* data ref */
};

static struct lua_MSP *
lua_msp_check(struct lua_State *L, int n)
{
    struct lua_MSP *M;

    M = luaL_checkudata(L, n, "MSP");
    luaL_argcheck(L, M != NULL, n, "`MSP' expected");

    return M;
}

static void
lua_msp_decode_table(struct lua_State *L,
                     void **_data, size_t len,
                     const struct msp_msg_field *info)
{
    void *data;

    data = *_data;

    lua_newtable(L);

    for (; info->name != NULL; info++) {
        lua_Number num;

        switch (info->type) {
        case FIELD_T_UINT8:
        case FIELD_T_UINT16:
        case FIELD_T_UINT32:

            switch (info->type) {
            case FIELD_T_UINT8:
                num = * (uint8_t *) data;
                data += sizeof(uint8_t);
                break;
            case FIELD_T_UINT16:
                num = * (uint16_t *) data;
                data += sizeof(uint16_t);
                break;
            case FIELD_T_UINT32:
                num = * (uint32_t *) data;
                data += sizeof(uint32_t);
                break;
            default:
                break;
            }

            lua_pushnumber(L, num);
            break;

        case FIELD_T_STRUCT:
            lua_msp_decode_table(L,
                                 &data, data - *_data,
                                 info->elem);
            break;

        default:
            abort();
        }

        lua_setfield(L, -2, info->name);
    }

    *_data = data;
}

static void
lua_msp_decode_names(struct lua_State *L,
                     const char *data, size_t len)
{
    const char *prv, *pos;
    int n;

    lua_newtable(L);

    n = 0;

    for (prv = pos = data; pos < data + len; pos++)
        if (*pos == ';') {
            lua_pushlstring(L, prv, pos - prv);
            lua_rawseti(L, -2, n++);
            prv = pos;
        }
}

static int
lua_msp_decode(struct lua_State *L,
               const struct msp_hdr *hdr, void *data)
{
    const struct msp_cmd_info *cmd_info;
    const struct msp_msg_info *msg_info;

    msp_info_for_each_cmd(cmd_info)
        if (cmd_info->cmd == hdr->cmd)
            break;

    assert(cmd_info != NULL);

    msg_info = &cmd_info->rsp;

    switch (msg_info->type) {
    case MSG_T_STRUCT:
        lua_msp_decode_table(L, &data, hdr->len, msg_info->data);
        break;
    case MSG_T_NAMES:
        lua_msp_decode_names(L, data, hdr->len);
        break;
    case MSG_T_STRING:
        lua_pushlstring(L, data, hdr->len);
        break;
    }

    return 1;
}

static int
lua_msp_retfn(int err,
              const struct msp_hdr *hdr, void *data,
              void *priv)
{
    struct lua_State *L;
    struct lua_MSP *M;

    L = priv;

    lua_getfield(L, LUA_REGISTRYINDEX, &M->mtab);
    lua_rawgeti(L, -1, hdr->cmd);

    assert(lua_type(L, -1) == LUA_TFUNCTION);
}

static int
lua_cmd(struct lua_State *L)
{
    struct lua_MSP *M;
    struct lua_MSP_Cmd *C;
    msp_cmd_t cmd;

    M = lua_msp_check(L, 1);

    C = lua_newuserdata(L, sizeof(*C));
    C->L = L;
    C->M = M;

    C->ctxr = luaL_ref(L, LUA_REGISTRYINDEX);
    C->mspr = luaL_ref(L, LUA_REGISTRYINDEX);

    (void) cmd;

    return 1;
}

static int
lua_msp_new(struct lua_State *L, struct msp *msp)
{
    struct lua_MSP *M;

    M = lua_newuserdata(L, sizeof(*M));
    luaL_getmetatable(L, "MSP");
    lua_setmetatable(L, -2);

    M->c = msp;

    return 1;
}

static int
lua_msp_open(struct lua_State *L)
{
    const char *path;
    struct tty *tty;
    struct msp *msp;
    speed_t speed;

    speed = B115200;

    path = luaL_checkstring(L, 1);

    tty = tty_open(path, speed);

    msp = msp_open(tty, g_loop);

    (void) msp;

    (void) lua_msp_decode;
    (void) lua_msp_retfn;
    (void) lua_cmd;
    (void) lua_msp_new;

    return 1;
}

static int
lua_msp_close(struct lua_State *L)
{
    struct lua_MSP *M;

    M = lua_msp_check(L, 1);

    if (M->c) {
        msp_close(M->c);
        M->c = NULL;
    }

    return 0;
}

static const struct luaL_reg lua_msp_fn [] = {
    { "open", lua_msp_open },
    { NULL, NULL }
};

static const struct luaL_reg lua_msp_class [] = {
    { "close", lua_msp_close },
    { NULL, NULL }
};

static const struct luaL_reg lua_msp_meta [] = {
    { NULL, NULL }
};

int
luaopen_msp(struct lua_State *L)
{
    int rc;

    rc = luaL_loadbuffer(L,
                         MCC_LUA_START(msp), MCC_LUA_LEN(msp),
                         "msp.lua");
    assert(!rc);
    lua_call(L, 0, 0);

    luaL_openlib(L, "MSP", lua_msp_fn, 0);
    lua_pop(L, 1);

    lua_object_classinit(L, "MSP", lua_msp_class, lua_msp_meta);

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
