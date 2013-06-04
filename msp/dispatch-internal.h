#ifndef MSP_DISPATCH_INTERNAL_H
#define MSP_DISPATCH_INTERNAL_H

#include <msp/msg.h>
#include <msp/dispatch.h>
#include <crt/list.h>

struct msp_dispatch_info {
    void (*encode)(struct msp_hdr *hdr, void *data);
    void (*decode)(struct msp_hdr *hdr, void *data);
    void (*output)(struct msp_hdr *hdr, void *data, void *priv);
};

struct msp_dispatch {
    struct msp_dispatch_info tab[MSP_CMD_MAX];
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
