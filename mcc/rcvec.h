#ifndef MCC_RCVEC_H
#define MCC_RCVEC_H

#include <lua.h>
#include <stdint.h>

struct lua_RCVec {
    int len;
    uint16_t val[0];
};

struct lua_RCVec * lua_rcvec_check(struct lua_State *L, int idx);

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
