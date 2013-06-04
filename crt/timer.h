#ifndef CRT_TIMER_H
#define CRT_TIMER_H

#include <sys/time.h>

struct timer;
struct timerwheel;

typedef void (*timer_fn)(const struct timeval *timeo, void *data);

struct timer *__timer_create(timer_fn fn, void *priv,
                             struct timerwheel *wheel);

void timer_start(struct timer *timer,
                 const struct timeval *interval);

void timer_restart(struct timer *timer,
                   const struct timeval *now,
                   const struct timeval *interval);

void timer_start_at(struct timer *timer,
                    const struct timeval *timeo);

void timer_stop(struct timer *timer);

void timer_destroy(struct timer *timer);

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
