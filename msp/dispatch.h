#ifndef MSP_DISPATCH_H
#define MSP_DISPATCH_H

#include <msp/msg.h>
#include <msp/msp.h>
#include <crt/list.h>

typedef void (*msp_msg_fn)(struct msp_hdr *hdr, void *data);

struct msp_msg_handler {
    msp_msg_fn fn;
    void *data;
};

struct msp_dispatch * msp_dispatch_create(struct msp *msp);

void msp_dispatch_destroy(struct msp_dispatch *dsp);

int msp_dispatch_add(struct msp_dispatch *dsp,
                     msp_cmd_t cmd, struct msp_msg_handler *hnd);

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
