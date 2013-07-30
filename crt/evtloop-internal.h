#ifndef CRT_EVTLOOP_INTERNAL_H
#define CRT_EVTLOOP_INTERNAL_H

#include <crt/evtloop.h>
#include <crt/timer-internal.h>
#include <crt/list.h>

#include <sys/time.h>

struct evtloop {
    struct timerwheel *timers;
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

void evtloop_schedule_timer(struct evtloop *loop,
                            struct timer *timer);

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
