#ifndef CRT_LOG_STDIO_INTERNAL_H
#define CRT_LOG_STDIO_INTERNAL_H

#include <crt/log-internal.h>

struct log_stdio {
    FILE *f;
};

extern const struct log_iface log_stdio_iface;

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
