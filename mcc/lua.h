#ifndef MCC_LUA_H
#define MCC_LUA_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

extern struct evtloop *g_loop;

struct lua_State * lua_create(void);

void lua_destroy(struct lua_State *);

#define TOP() do {                                                  \
        int top = lua_gettop(L);                                    \
        int typ = lua_type(L, -1);                                  \
        const void *ptr = lua_topointer(L, -1);                     \
        debug("%s:%d: top %d type %d addr %p", __func__, __LINE__,  \
              top, typ, ptr);                                       \
    } while (0)

#define MCC_LUA_SYM(_mod, _name)                \
    _binary_ ## _mod ## _lbc_ ## _name

#define MCC_LUA_STARTSYM(_mod)                  \
    MCC_LUA_SYM(_mod, start)

#define MCC_LUA_ENDSYM(_mod)                    \
    MCC_LUA_SYM(_mod, end)

#define MCC_LUA_SIZESYM(_mod)                   \
    MCC_LUA_SYM(_mod, size)

#define MCC_LUA_START(_mod) ({                                          \
            extern const char MCC_LUA_STARTSYM(_mod);                   \
            (void *) &MCC_LUA_STARTSYM(_mod);                           \
        })

#define MCC_LUA_END(_mod) ({                                            \
            extern const char MCC_LUA_ENDSYM(_mod);                     \
            (void *) &MCC_LUA_ENDSYM(_mod);                             \
        })

#define MCC_LUA_LEN(_mod)                                   \
    ((size_t)(MCC_LUA_END(_mod) - MCC_LUA_START(_mod)))

struct mcc_init_luaopen {
    lua_CFunction func;
    const char *name;
};

void
lua_object_classinit(struct lua_State *,
                     const char *tname,
                     const struct luaL_reg *reg_class,
                     const struct luaL_reg *reg_meta);
int luaopen_object(struct lua_State *);

int luaopen_js(struct lua_State *);
int luaopen_tx(struct lua_State *);
int luaopen_nrx(struct lua_State *);
int luaopen_ntx(struct lua_State *);

#define MCC_LUA_INIT(_entry)                                    \
    __attribute__((section("lua_init")))                        \
    struct mcc_init_luaopen __mcc_init_ ## _entry = {           \
        .func = _entry,                                         \
        .name = #_entry                                         \
    }

#endif

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
