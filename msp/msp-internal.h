#ifndef MSP_MSP_INTERNAL_H
#define MSP_MSP_INTERNAL_H

#include <msp/msp.h>

#include <crt/tty.h>
#include <crt/evtloop.h>

#define MSP_TIMEOUT (struct timeval) { 1, 0 }

struct msp {
    struct tty *tty;
    struct evtloop *loop;
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
