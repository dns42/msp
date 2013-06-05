#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <crt/evtloop-internal.h>
#include <crt/defs.h>

#include <stdlib.h>
#include <assert.h>

static void evtloop_schedule_timerevt(struct evtloop *, struct timerevt *,
                                      const struct timeval *);

static void
pollevt_unregister(struct pollevt *evt)
{
    if (evt->loop) {
        list_remove(&evt->entry);
        evt->loop = NULL;
    }
}

static void
pollevt_register(struct pollevt *evt, struct evtloop *loop)
{
    assert(!evt->loop);

    list_insert_tail(&loop->pollevts, &evt->entry);
    evt->loop = loop;
}

void
pollevt_destroy(struct pollevt *evt)
{
    pollevt_unregister(evt);
    free(evt);
}

static struct pollevt *
pollevt_create(int fd, pollevt_fn fn, void *data)
{
    struct pollevt *evt;

    evt = calloc(1, sizeof(*evt));
    if (!expected(evt))
        goto out;

    evt->entry = LIST(&evt->entry);
    evt->fd = fd;
    evt->fn = fn;
    evt->data = data;
out:
    return evt;
}

void
pollevt_select(struct pollevt *evt, short events)
{
    evt->events = events;
}

static void
timerevt_unregister(struct timerevt *timer)
{
    if (timer->loop) {
        list_remove(&timer->entry);
        timer->loop = NULL;
    }
}

static void
timerevt_register(struct timerevt *timer, struct evtloop *loop)
{
    struct timeval now;

    assert(!timer->loop);
    timer->loop = loop;

    gettimeofday(&now, NULL);
    evtloop_schedule_timerevt(loop, timer, &now);
}

void
timerevt_destroy(struct timerevt *timer)
{
    timerevt_unregister(timer);
    free(timer);
}

static struct timerevt *
timerevt_create(struct timeval itv, timerevt_fn fn, void *data)
{
    struct timerevt *timer;

    timer = calloc(1, sizeof(*timer));
    if (!expected(timer))
        goto out;

    timer->entry = LIST(&timer->entry);
    timer->itv = itv;
    timer->fn = fn;
    timer->data = data;
out:
    return timer;
}

struct timerevt *
evtloop_add_timer(struct evtloop *loop,
                  struct timeval itv,
                  timerevt_fn fn, void *data)
{
    struct timerevt *timer;

    timer = timerevt_create(itv, fn, data);
    if (timer)
        timerevt_register(timer, loop);

    return timer;
}

static void
evtloop_schedule_timerevt(struct evtloop *loop, struct timerevt *timer,
                        const struct timeval *now)
{
    struct timerevt *next;

    assert(timer->loop == loop);

    list_remove(&timer->entry);

    timeradd(now, &timer->itv, &timer->timeo);

    list_for_each_entry(&loop->timerevts, next, entry)
        if (timercmp(&next->timeo, &timer->timeo, >)) {
            list_insert_before(&next->entry, &timer->entry);
            return;
        }

    list_insert_tail(&loop->timerevts, &timer->entry);
}

static void
evtloop_run_timerevts(struct evtloop *loop, struct timeval *now)
{
    struct timerevt *timer, *next;

    list_for_each_entry_safe(&loop->timerevts, timer, next, entry) {
        if (timercmp(now, &timer->timeo, <))
            gettimeofday(now, NULL);

        if (timercmp(now, &timer->timeo, <))
            break;

        if (timer->fn)
            timer->fn(&timer->timeo, timer->data);

        if (__list_prev_entry(next, entry) == timer)
            evtloop_schedule_timerevt(loop, timer, &timer->timeo);
    }
}

struct pollevt *
evtloop_add_pollfd(struct evtloop *loop,
                   int fd, pollevt_fn fn, void *data)
{
    struct pollevt *evt;

    evt = pollevt_create(fd, fn, data);
    if (evt)
        pollevt_register(evt, loop);

    return evt;
}

int
evtloop_iterate(struct evtloop *loop)
{
    struct pollevt *evt;
    struct timerevt *timer;
    struct timeval now, _tv, *timeo;
    fd_set rfds, wfds;
    int rc, nfds;

    rc = -1;
    gettimeofday(&now, NULL);

    timeo = NULL;
    timer = list_first_entry(&loop->timerevts, struct timerevt, entry);
    if (timer) {
        if (timercmp(&timer->timeo, &now, <=))
            goto timeout;

        timeo = &_tv;
        timersub(&timer->timeo, &now, timeo);
    }

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);

    nfds = -1;
    list_for_each_entry(&loop->pollevts, evt, entry) {

        if (evt->events & POLLIN)
            FD_SET(evt->fd, &rfds);

        if (evt->events & POLLOUT)
            FD_SET(evt->fd, &wfds);

        nfds = evt->fd > nfds ? evt->fd : nfds;
    }

    rc = select(nfds + 1, &rfds, &wfds, NULL, timeo);
    if (rc < 0)
        goto out;

    if (rc > 0) {
        nfds = rc;

        list_for_each_entry(&loop->pollevts, evt, entry) {
            int revents = 0;

            if (FD_ISSET(evt->fd, &rfds))
                revents |= POLLIN;

            if (FD_ISSET(evt->fd, &wfds))
                revents |= POLLOUT;

            assert((evt->events | revents) == evt->events);

            if (revents)
                evt->fn(revents, evt->data);

            if (!--nfds)
                break;
        }

        assert(!nfds);
    }

timeout:
    evtloop_run_timerevts(loop, &now);
out:
    return rc;
}

void
evtloop_destroy(struct evtloop *loop)
{
    struct pollevt *evt, *nevt;
    struct timerevt *timer, *ntimer;

    list_for_each_entry_safe(&loop->timerevts, timer, ntimer, entry)
        timerevt_unregister(timer);

    list_for_each_entry_safe(&loop->pollevts, evt, nevt, entry)
        pollevt_unregister(evt);

    free(loop);
}

struct evtloop *
evtloop_create(void)
{
    struct evtloop *loop;

    loop = calloc(1, sizeof(*loop));
    if (!expected(loop))
        goto out;

    loop->timerevts = LIST(&loop->timerevts);
    loop->pollevts = LIST(&loop->pollevts);
out:
    return loop;
}


/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
