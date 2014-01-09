#ifndef MCC_LUA_RCVEC_H
#define MCC_LUA_RCVEC_H

#include <lua.h>
#include <stdint.h>

struct lua_RCVec {
    int len;
    uint16_t val[8];
};

struct lua_RCVec * lua_rcvec_check(struct lua_State *L, int idx);

int __lua_rcvec_new(struct lua_State *L,
                    const uint16_t *val, int len);

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
