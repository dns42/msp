#ifndef CRT_TTY_INTERNAL_H
#define CRT_TTY_INTERNAL_H

#include <crt/tty.h>
#include <stdint.h>

struct tty {
    int fd;
    struct pollevt *evt;

    const struct iovec *iov;
    int cnt;
    size_t off;

    tty_rx_fn rfn;
    void *priv;
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
