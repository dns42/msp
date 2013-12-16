#ifndef MSP_MSP_H
#define MSP_MSP_H

#include <msp/msg.h>

#include <crt/evtloop.h>
#include <crt/tty.h>

#include <sys/types.h>
#include <sys/types.h>
#include <termios.h>

struct msp * msp_open(struct tty *tty, struct evtloop *loop);

void msp_close(struct msp *msp);

typedef void (*msp_call_retfn)(int err,
                               const struct msp_hdr *, void *data,
                               void *priv);

int msp_call(struct msp *msp,
             msp_cmd_t cmd, void *args, size_t len,
             msp_call_retfn rfn, void *priv, const struct timeval *timeo);

void msp_sync(struct msp *msp, msp_cmd_t cmd);

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
