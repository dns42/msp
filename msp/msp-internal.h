#ifndef MSP_MSP_INTERNAL_H
#define MSP_MSP_INTERNAL_H

#include <msp/msp.h>

#include <crt/tty.h>
#include <crt/evtloop.h>

#define MSP_TAB_SIZE (MSP_CMD_MAX - MSP_CMD_MIN)
#define MSP_TAB_IDX(_cmd) (_cmd - MSP_CMD_MIN)

struct msp_call {
    msp_call_retfn rfn;
    void *priv;
    struct timer *timer;
};

struct msp {
    struct tty *tty;
    struct evtloop *loop;
    struct msp_call *tab[MSP_TAB_SIZE];
    struct iovec iov[2];
    struct msp_hdr hdr;
    uint8_t cks;
};

struct msp_call *msp_call_get(struct msp *, msp_cmd_t);

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
