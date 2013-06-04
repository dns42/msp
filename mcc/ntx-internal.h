#ifndef MCC_NTX_INTERNAL_H
#define MCC_NTX_INTERNAL_H

#include <mcc/ntx.h>
#include <rpc/clnt.h>

struct ntx {
    int sock;
    CLIENT *clnt;
};

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
