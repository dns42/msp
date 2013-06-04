#ifdef RPC_XDR
%#include "nrxrpc.h"
#endif

typedef uint16_t NRXv1_RCarg<>;

program NRX_PROGRAM {
    version NRXv1 {
        void NRXv1_RC(NRXv1_RCarg) = 1;
    } = 1;
} = 1;

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
