#ifndef CRT_EVT_LOOP_H
#define CRT_EVT_LOOP_H

#include <poll.h>
#include <crt/timer.h>

struct evtloop * evtloop_create(void);

void evtloop_destroy(struct evtloop *main);

int evtloop_iterate(struct evtloop *main);

void evtloop_add_timer(struct evtloop *loop,
                       struct timer *timer,
                       const struct timeval *timeo);

struct timer * evtloop_create_timer(struct evtloop *loop,
                                    const struct timeval *timeo,
                                    timer_fn fn, void *data);

typedef void (*pollevt_fn)(int revents, void *data);

struct pollevt * evtloop_add_pollfd(struct evtloop *loop,
                                    int fd, pollevt_fn fn, void *data);

void pollevt_select(struct pollevt *evt, short events);

void pollevt_destroy(struct pollevt *evt);

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
