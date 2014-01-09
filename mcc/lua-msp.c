#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/lua.h>
#include <mcc/lua-timer.h>
#include <mcc/mcc.h>

#include <crt/list.h>

#include <msp/msp.h>
#include <msp/meta.h>

#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

struct lua_MSP {
    struct msp *m;
    struct tty *t;
    struct list pending;
    struct timeval timeo;
};

struct lua_MSP_Cmd {
    struct lua_State *L;
    const struct msp_cmd_info *info;
    struct list entry;
    int resref;
    int funref;
    int mspref;
    int cmdref;
    int err;
};

static struct lua_MSP *
lua_msp_check(struct lua_State *L, int n)
{
    struct lua_MSP *M;

    M = luaL_checkudata(L, n, "MSP");
    luaL_argcheck(L, M != NULL, n, "`MSP' expected");

    return M;
}

struct lua_msp_decode {
    struct lua_State *L;
    const struct msp_hdr *hdr;
    const void *data;
    size_t len;
};

static void
lua_msp_decode_check(struct lua_msp_decode *lmd,
                    size_t len)
{
    if (lmd->len < len)
        luaL_error(lmd->L, "reading beyond message end");
}

static void lua_msp_decode_data(struct lua_msp_decode *lmd,
                                const struct msp_data *info);

static void
lua_msp_decode_number(struct lua_msp_decode *lmd,
                      const struct msp_data *info)
{
    lua_Number num;
    union {
        uint8_t *u8;
        uint16_t *u16;
        uint32_t *u32;
        const void *p;
    } pos;

    pos.p = lmd->data;

    switch (info->type) {
    case MSP_DATA_T_UINT8:
        lua_msp_decode_check(lmd, sizeof(*pos.u8));
        num = *pos.u8++;
        break;
    case MSP_DATA_T_UINT16:
        lua_msp_decode_check(lmd, sizeof(*pos.u16));
        num = *pos.u16++;
        break;
    case MSP_DATA_T_UINT32:
        lua_msp_decode_check(lmd, sizeof(*pos.u32));
        num = *pos.u32++;
        break;
    default:
        break;
    }

    lmd->len -= pos.p - lmd->data;
    lmd->data += pos.p - lmd->data;

    if (info->vals) {
        const struct msp_int_value *val;

        msp_for_each_value_info(val, info->vals)
            switch (val->type) {
            case MSP_VALUE_T_ENUM:
                if (num == val->value) {
                    lua_pushstring(lmd->L, val->name);
                    return;
                }
            default:
                break;
            }
    }

    lua_pushnumber(lmd->L, num);
}

static void
lua_msp_decode_struct(struct lua_msp_decode *lmd,
                      const struct msp_data *info)
{
    const struct msp_struct_field *field;

    lua_newtable(lmd->L);

    msp_for_each_field_info(field, info->fields) {
        lua_msp_decode_data(lmd, &field->data);
        lua_setfield(lmd->L, -2, field->name);
    }
}

static void
lua_msp_decode_array(struct lua_msp_decode *lmd,
                     const struct msp_data *info)
{
    int n;

    lua_newtable(lmd->L);

    for (n = 1;
         info->elem->cnt == MSP_SIZE_EOM
             ? lmd->len
             : n <= info->elem->cnt;
         n++) {
        luaL_checktype(lmd->L, -1, LUA_TTABLE);
        lua_msp_decode_data(lmd, &info->elem->data);
        lua_rawseti(lmd->L, -2, n);
    }
}

static void
lua_msp_decode_names(struct lua_msp_decode *lmd,
                     const struct msp_data *info)
{
    const char *pos, *prv, *end;
    int n;

    lua_newtable(lmd->L);

    n = 1;
    end = lmd->data + lmd->len;

    for (prv = pos = lmd->data; pos < end; pos++)
        if (*pos == ';') {
            lua_pushlstring(lmd->L, prv, pos - prv);
            lua_rawseti(lmd->L, -2, n++);
            prv = pos + 1;
        }

    lmd->data += lmd->len;
    lmd->len = 0;
}

static void
lua_msp_decode_string(struct lua_msp_decode *lmd,
                      const struct msp_data *info)
{
    lua_pushlstring(lmd->L, lmd->data, lmd->len);

    lmd->data += lmd->len;
    lmd->len = 0;
}

static void
lua_msp_decode_data(struct lua_msp_decode *lmd,
                    const struct msp_data *info)
{
    switch (info->type) {
    case MSP_DATA_T_UINT8:
    case MSP_DATA_T_UINT16:
    case MSP_DATA_T_UINT32:
        lua_msp_decode_number(lmd, info);
        break;

    case MSP_DATA_T_STRUCT:
        lua_msp_decode_struct(lmd, info);
        break;

    case MSP_DATA_T_ARRAY:
        lua_msp_decode_array(lmd, info);
        break;

    case MSP_DATA_T_NAMES:
        lua_msp_decode_names(lmd, info);
        break;

    case MSP_DATA_T_STRING:
        lua_msp_decode_string(lmd, info);
        break;
    }
}

static void
lua_msp_decode(struct lua_State *L,
               const struct msp_hdr *hdr, const void *data,
               const struct msp_data *info)
{
    struct lua_msp_decode _lmd = {
        .L = L,
        .hdr = hdr,
        .data = data,
        .len = hdr->len,
    };

    if (info->type)
        lua_msp_decode_data(&_lmd, info);

    if (_lmd.len != 0)
        warn("cmd %d: %zu/%d bytes undecoded",
             hdr->cmd, _lmd.len, hdr->len);
}

struct lua_msp_encode {
    struct lua_State *L;
    void *data;
    size_t pos;
    size_t usz;
};

static void
lua_msp_encode_data(struct lua_msp_encode *lme,
                    const struct msp_data *info);

static void *
lua_msp_encode_grow(struct lua_msp_encode *lme,
                    size_t len)
{

    if (lme->pos + len > lme->usz) {
        void *data;

        data = realloc(lme->data, lme->pos + len);

        if (!expected(data))
            luaL_error(lme->L, "out of memory");

        lme->data = data;
        lme->usz = malloc_usable_size(data);
    }

    return lme->data + lme->pos;
}

static void
lua_msp_encode_number(struct lua_msp_encode *lme,
                      const struct msp_data *info)
{
    int val;
    union {
        uint8_t *u8;
        uint16_t *u16;
        uint32_t *u32;
        void *p;
    } pos;

    val = luaL_checknumber(lme->L, -1);

    switch (info->type) {
    case MSP_DATA_T_UINT8:
        pos.p = lua_msp_encode_grow(lme, sizeof(*pos.u8));
        *pos.u8++ = val;
        break;
    case MSP_DATA_T_UINT16:
        pos.p = lua_msp_encode_grow(lme, sizeof(*pos.u16));
        *pos.u16++ = val;
        break;
    case MSP_DATA_T_UINT32:
        pos.p = lua_msp_encode_grow(lme, sizeof(*pos.u32));
        *pos.u32++ = val;
        break;
    default:
        assert(0);
    }

    lme->pos = pos.p - lme->data;

    lua_pop(lme->L, 1);
}

static void
lua_msp_encode_array(struct lua_msp_encode *lme,
                     const struct msp_data *info)
{
    int n;

    for (n = 1;
         info->elem->cnt == MSP_SIZE_EOM
             ? 1
             : n <= info->elem->cnt;
         n++) {

        lua_pushinteger(lme->L, n);
        lua_gettable(lme->L, -2);

        if (lua_isnil(lme->L, -1) &&
            info->elem->cnt == MSP_SIZE_EOM) {
            lua_pop(lme->L, 1);
            break;
        }

        lua_msp_encode_data(lme, &info->elem->data);
    }

    lua_pop(lme->L, 1);
}

static void
lua_msp_encode_struct(struct lua_msp_encode *lme,
                     const struct msp_data *info)
{
    const struct msp_struct_field *field;

    msp_for_each_field_info(field, info->fields) {
        luaL_checktype(lme->L, -1, LUA_TTABLE);
        lua_getfield(lme->L, -1, field->name);
        lua_msp_encode_data(lme, &field->data);
    }

    lua_pop(lme->L, 1);
}

static void
lua_msp_encode_data(struct lua_msp_encode *lme,
                    const struct msp_data *info)
{
    switch (info->type) {
    case MSP_DATA_T_UINT8:
    case MSP_DATA_T_UINT16:
    case MSP_DATA_T_UINT32:
        lua_msp_encode_number(lme, info);
        break;

    case MSP_DATA_T_STRUCT:
        lua_msp_encode_struct(lme, info);
        break;

    case MSP_DATA_T_ARRAY:
        lua_msp_encode_array(lme, info);
        break;

    case MSP_DATA_T_NAMES:
    case MSP_DATA_T_STRING:
        luaL_error(lme->L,
                   "unsupported encoding type (%d)",
                   info->type);
    }
}

static void
lua_msp_encode(struct lua_State *L,
               const struct msp_data *info,
               void **data, size_t *len)
{
    struct lua_msp_encode _lme = {
        .L = L,
        .data = NULL,
        .pos = 0,
        .usz = 0,
    };

    if (info->type)
        lua_msp_encode_data(&_lme, info);

    *data = _lme.data;
    *len = _lme.pos;
}

static struct lua_MSP_Cmd *
lua_msp_cmd_check(struct lua_State *L, int n)
{
    struct lua_MSP_Cmd *C;

    C = luaL_checkudata(L, n, "MSP.Cmd");
    luaL_argcheck(L, C != NULL, n, "`MSP.Cmd' expected");

    return C;
}

static int
__lua_msp_cmd_returned(struct lua_MSP_Cmd *C)
{
    return C->mspref == LUA_NOREF;
}

static void
__lua_msp_cmd_callback(struct lua_MSP_Cmd *C)
{
    struct lua_State *L;

    L = C->L;

    if (C->funref != LUA_REFNIL) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, C->funref);

        lua_rawgeti(L, LUA_REGISTRYINDEX, C->cmdref);
        lua_pushinteger(L, C->err);
        lua_rawgeti(L, LUA_REGISTRYINDEX, C->resref);

        lua_call(L, 3, 0);

        luaL_unref(L, LUA_REGISTRYINDEX, C->funref);
        C->funref = LUA_REFNIL;
    }
}

static void
__lua_msp_cmd_return(int err,
                     const struct msp_hdr *hdr, void *data,
                     void *priv)
{
    struct lua_MSP_Cmd *C;
    struct lua_State *L;

    C = priv;
    L = C->L;

    assert(C->cmdref != LUA_NOREF);
    assert(C->mspref != LUA_NOREF);

    C->err = err;
    C->resref = LUA_REFNIL;

    if (!err) {
        assert(hdr->cmd == C->info->cmd);

        if (hdr->len) {
            lua_msp_decode(L, hdr, data, &C->info->rsp);
            C->resref = luaL_ref(L, LUA_REGISTRYINDEX);
        }

        free(data);
    }

    list_remove(&C->entry);

    luaL_unref(L, LUA_REGISTRYINDEX, C->mspref);
    C->mspref = LUA_NOREF;

    __lua_msp_cmd_callback(C);

    luaL_unref(L, LUA_REGISTRYINDEX, C->cmdref);
    C->cmdref = LUA_NOREF;
}

static int
lua_msp_cmd_issue(struct lua_State *L)
{
    struct msp_cmd_info *info;
    struct lua_MSP *M;
    struct lua_MSP_Cmd *C;
    void *args;
    size_t len;
    int rc;

    info = lua_touserdata(L, lua_upvalueindex(1));
    args = NULL;
    len = 0;

    M = lua_msp_check(L, 1);

    luaL_argcheck(L, lua_isnone(L, 2) || info->req.type,
                  2, "cannot encode cmd args");

    luaL_argcheck(L, !info->req.type || !lua_isnone(L, 2),
                  2, "missing cmd args");

    if (!lua_isnone(L, 2))
        lua_msp_encode(L, &info->req, &args, &len);

    C = lua_newuserdata(L, sizeof(*C));
    luaL_getmetatable(L, "MSP.Cmd");
    lua_setmetatable(L, -2);

    C->L = L;
    C->info = info;
    C->resref = LUA_NOREF;
    C->funref = LUA_REFNIL;
    C->err = 0;

    rc = msp_call(M->m,
                  C->info->cmd, args, len,
                  __lua_msp_cmd_return, C,
                  &M->timeo);
    if (rc)
        luaL_error(L,
                   "MSP.%s: %s",
                   C->info->name, strerror(errno));

    lua_pushvalue(L, -1);
    C->cmdref = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_pushvalue(L, 1);
    C->mspref = luaL_ref(L, LUA_REGISTRYINDEX);

    list_insert_tail(&M->pending, &C->entry);

    return 1;
}

static int
lua_msp_cmd_async(struct lua_State *L)
{
    struct lua_MSP_Cmd *C;

    C = lua_msp_cmd_check(L, 1);

    luaL_argcheck(L,
                  lua_type(L, 2) == LUA_TFUNCTION ||
                  lua_type(L, 2) == LUA_TNIL,
                  2,
                  "function or nil expected");

    if (C->funref != LUA_REFNIL)
        luaL_unref(L, LUA_REGISTRYINDEX, C->funref);

    if (lua_type(L, 2) == LUA_TFUNCTION) {
        C->funref = luaL_ref(L, LUA_REGISTRYINDEX);

        if (__lua_msp_cmd_returned(C))
            __lua_msp_cmd_callback(C);
    }

    return 1;
}

static int
lua_msp_cmd_sync(struct lua_State *L)
{
    struct lua_MSP_Cmd *C;
    struct lua_MSP *M;

    C = lua_msp_cmd_check(L, 1);

    if (!__lua_msp_cmd_returned(C)) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, C->mspref);
        assert(lua_isuserdata(L, -1));
        M = lua_touserdata(L, -1);
        msp_sync(M->m, C->info->cmd);
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, C->resref);

    return 1;
}

static int
lua_msp_cmd_poll(struct lua_State *L)
{
    struct lua_MSP_Cmd *C;

    C = lua_msp_cmd_check(L, 1);

    lua_pushboolean(L, __lua_msp_cmd_returned(C));

    return 1;
}

#if 0
static int
lua_msp_cmd_index(struct lua_State *L)
{
    struct lua_MSP_Cmd *C;
    const char *k;

    C = lua_msp_cmd_check(L, 1);
    k = luaL_checkstring(L, 2);

    switch (k[0]) {
    case 'e':
        if (!strcmp(k, "err")) {
            lua_pushinteger(L, C->err);
            return 1;
        }
    }

    return 0;
}
#endif

static int
lua_msp_cmd_tostring(struct lua_State *L)
{
    struct lua_MSP_Cmd *C;
    const char *status;

    C = lua_msp_cmd_check(L, 1);

    status = __lua_msp_cmd_returned(C) ? "complete" : "pending";

    lua_pushfstring(L, "MSP.%s (%d), %s, err %d",
                    C->info->name, C->info->cmd, status, C->err);

    return 1;
}

static int
lua_msp_newindex(struct lua_State *L)
{
    struct lua_MSP *M;
    const char *k;

    M = lua_msp_check(L, 1);
    k = luaL_checkstring(L, 2);

    switch (k[0]) {
    case 't':
        if (!strcmp(k, "timeout")) {
            M->timeo = lua_ntotime(luaL_checknumber(L, 3));
            lua_pushnumber(L, lua_timeton(&M->timeo));
            return 1;
        }
    }

    return 0;
}

static int
lua_msp_tostring(struct lua_State *L)
{
    struct lua_MSP *M;

    M = lua_msp_check(L, 1);
    lua_pushfstring(L, "MSP %p", M);

    return 1;
}

static int
lua_msp_cmd_gc(struct lua_State *L)
{
    struct lua_MSP_Cmd *C;

    C = lua_msp_cmd_check(L, 1);

    if (!mcc_stopped(g_mcc)) {
        assert(C->resref != LUA_NOREF);
        assert(C->cmdref == LUA_NOREF);
        assert(C->mspref == LUA_NOREF);
        assert(C->funref == LUA_REFNIL);
    }

    luaL_unref(L, LUA_REGISTRYINDEX, C->resref);

    return 0;
}

static int
lua_msp_new(struct lua_State *L,
            struct msp *msp, struct tty *tty)
{
    struct lua_MSP *M;

    M = lua_newuserdata(L, sizeof(*M));
    luaL_getmetatable(L, "MSP");
    lua_setmetatable(L, -2);

    M->m = msp;
    M->t = tty;
    M->pending = LIST(&M->pending);
    M->timeo = (struct timeval) { 1, 0 };

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
    msp = NULL;

    path = luaL_checkstring(L, 1);

    if (lua_gettop(L) >= 2) {
        int val = luaL_checknumber(L, 2);
        speed = tty_itospeed(val);
        luaL_argcheck(L, speed != -1, 2, "invalid port speed");
    }

    tty = tty_open(path, speed);
    if (!tty)
        goto out;

    msp = msp_open(tty, g_loop);
out:
    if (!msp) {
        int err = errno;

        if (tty)
            tty_close(tty);

        luaL_error(L, "MSP.open('%s', %d): %s",
                   path, tty_speedtoi(speed), strerror(err));
    }

    return lua_msp_new(L, msp, tty);
}

static int
lua_msp_close(struct lua_State *L)
{
    struct lua_MSP *M;
    struct lua_MSP_Cmd *C, *next;

    M = lua_msp_check(L, 1);

    if (M->m) {
        msp_close(M->m);
        M->m = NULL;
    }

    if (M->t) {
        tty_close(M->t);
        M->t = NULL;
    }

    list_for_each_entry_safe(&M->pending, C, next, entry)
        __lua_msp_cmd_return(ESHUTDOWN, NULL, NULL, C);

    return 0;
}

static int
lua_msp_pending(struct lua_State *L)
{
    struct lua_MSP *M;
    struct lua_MSP_Cmd *C;

    M = lua_msp_check(L, 1);

    lua_newtable(L);

    do {
        C = list_first_entry(&M->pending,
                             struct lua_MSP_Cmd, entry);
        if (C) {
            lua_rawgeti(L, LUA_REGISTRYINDEX, C->cmdref);
            lua_setfield(L, -1, C->info->name);
            lua_pop(L, 1);
        }
    } while (C);

    return 1;
}

static const struct luaL_reg lua_MSP_class [] = {
    { "open", lua_msp_open },
    { "close", lua_msp_close },
    { "pending", lua_msp_pending },
    { NULL, NULL }
};

static const struct luaL_reg lua_MSP_meta [] = {
    { "__newindex", lua_msp_newindex },
    { "__tostring", lua_msp_tostring },
    { "__gc", lua_msp_close },
    { NULL, NULL }
};

static const struct luaL_reg lua_MSP_Cmd_class [] = {
    { "async", lua_msp_cmd_async },
    { "sync", lua_msp_cmd_sync },
    { "poll", lua_msp_cmd_poll },
    { NULL, NULL }
};

static const struct luaL_reg lua_MSP_Cmd_meta [] = {
    { "__tostring", lua_msp_cmd_tostring },
    { "__gc", lua_msp_cmd_gc },
    { NULL, NULL }
};

static void
lua_msp_reg_cmds(struct lua_State *L)
{
    const struct msp_cmd_info *info;

    lua_object_getclass(L, "MSP");

    msp_for_each_cmd_info(info) {
        lua_pushlightuserdata(L, (void *) info);
        lua_pushcclosure(L, lua_msp_cmd_issue, 1);
        lua_setfield(L, -2, info->name);
    }

    lua_pop(L, 1);
}

int
luaopen_msp(struct lua_State *L)
{
    int rc;

    rc = luaL_loadbuffer(L,
                         MCC_LUA_START(msp), MCC_LUA_LEN(msp),
                         "msp.lua");
    assert(!rc);
    lua_call(L, 0, 0);

    lua_object_initclass(L, "MSP",
                         lua_MSP_class,
                         lua_MSP_meta);

    lua_msp_reg_cmds(L);

    lua_object_initclass(L, "MSP.Cmd",
                         lua_MSP_Cmd_class,
                         lua_MSP_Cmd_meta);

#if 0
    rc = luaL_loadbuffer(L,
                         MCC_LUA_START(multiwii), MCC_LUA_LEN(multiwii),
                         "multiwii.lua");
    assert(!rc);
    lua_call(L, 0, 0);
#endif

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
