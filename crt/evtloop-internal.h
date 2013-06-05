#ifndef CRT_EVTLOOP_INTERNAL_H
#define CRT_EVTLOOP_INTERNAL_H

#include <crt/evtloop.h>
#include <crt/list.h>

#include <sys/time.h>

struct evtloop {
    struct list timerevts;
    struct list pollevts;
};

struct pollevt {
    int fd;
    int events;
    pollevt_fn fn;
    void *data;
    struct evtloop *loop;
    struct list entry;
};

struct timerevt {
    struct timeval itv;
    struct timeval timeo;
    timerevt_fn fn;
    void *data;
    struct evtloop *loop;
    struct list entry;
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
