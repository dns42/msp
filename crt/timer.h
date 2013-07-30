#ifndef CRT_TIMER_H
#define CRT_TIMER_H

#include <sys/time.h>
struct evtloop;

typedef void (*timer_fn)(const struct timeval *timeo, void *data);

struct timer *__timer_create(timer_fn fn, void *priv);

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
