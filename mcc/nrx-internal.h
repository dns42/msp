#ifndef MCC_NRX_INTERNAL_H
#define MCC_NRX_INTERNAL_H

#include <mcc/nrx.h>

#include <crt/list.h>
#include <crt/event.h>

#include <rpc/svc.h>
#include <sys/select.h>

struct nrx {
    int sock;

    SVCXPRT *xprt;
    struct pollevt *evt;
    fd_set rfds;

    struct list entry;

    struct event *rcupdate;

    int nchn;
    uint16_t chn[0];
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
