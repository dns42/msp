#ifndef CRT_TIMER_INTERNAL_H
#define CRT_TIMER_INTERNAL_H

#include <crt/timer.h>
#include <crt/list.h>
#include <sys/time.h>

struct timer {
    struct timeval timeo;
    timer_fn fn;
    void *data;
    struct evtloop *loop;
    struct list entry;
};

struct timerwheel {
    struct list list;
};

struct timerwheel *timerwheel_create(void);

void timerwheel_destroy(struct timerwheel *wheel);

void timerwheel_insert(struct timerwheel *wheel, struct timer *timer,
                       const struct timeval *timeo);

void timerwheel_run(struct timerwheel *wheel, struct timeval *now);

int timerwheel_timeo(struct timerwheel *wheel, struct timeval *now,
                     struct timeval **timeo);

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
