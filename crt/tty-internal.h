#ifndef CRT_TTY_INTERNAL_H
#define CRT_TTY_INTERNAL_H

#include <crt/tty.h>
#include <stdint.h>

struct tty {
    int fd;
    struct pollevt *evt;

    void *buf;
    size_t max;

    unsigned int prod;
    unsigned int cons;

    struct {
        size_t cnt;
        tty_rxfn fn;
        void *data;
    } rxcall;

    int err;
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
