#ifndef MCC_MCC_INTERNAL_H
#define MCC_MCC_INTERNAL_H

#include <mcc/mcc.h>

struct mcc {
    int stop;
    int exit;
    char *reason;

    struct mcc_events {
        struct event *start;
        struct event *stop;
        struct event *destroy;
    } events;
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
