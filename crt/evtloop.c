#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <crt/evtloop-internal.h>
#include <crt/timer-internal.h>
#include <crt/defs.h>
#include <crt/log.h>

#include <stdlib.h>
#include <assert.h>

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

struct timer *
evtloop_create_timer(struct evtloop *loop,
                     timer_fn fn, void *data)
{
    return __timer_create(fn, data, loop->timers);
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
    struct timeval now, delta, *timeo;;
    fd_set rfds, wfds;
    int rc, nfds;

    gettimeofday(&now, NULL);

    nfds = 0;

    rc = timerwheel_timeo(loop->timers, &now, &timeo);
    if (rc)
        goto out;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);

    nfds = -1;
    list_for_each_entry(&loop->pollevts, evt, entry) {
        if (!evt->events)
            continue;

        if (evt->events & POLLIN)
            FD_SET(evt->fd, &rfds);

        if (evt->events & POLLOUT)
            FD_SET(evt->fd, &wfds);

        nfds = max(nfds, evt->fd);
    }

    if (timeo) {
        timersub(timeo, &now, &delta);
        timeo = &delta;
    }

    nfds = select(nfds + 1, &rfds, &wfds, NULL, timeo);
out:
    rc = nfds < 0 ? -1 : 0;

    if (nfds == 0) {
        gettimeofday(&now, NULL);
        timerwheel_run(loop->timers, &now);
    }

    if (nfds > 0) {
        list_for_each_entry(&loop->pollevts, evt, entry) {
            int revents = 0;

            if (FD_ISSET(evt->fd, &rfds))
                revents |= POLLIN;

            if (FD_ISSET(evt->fd, &wfds))
                revents |= POLLOUT;

            assert((evt->events | revents) == evt->events);

            if (revents) {
                evt->fn(revents, evt->data);

                if (!--nfds)
                    break;
            }
        }
        assert(!nfds);
    }

    return rc;
}

void
evtloop_destroy(struct evtloop *loop)
{
    struct pollevt *evt, *nevt;

    timerwheel_destroy(loop->timers);

    list_for_each_entry_safe(&loop->pollevts, evt, nevt, entry)
        pollevt_unregister(evt);

    free(loop);
}

struct evtloop *
evtloop_create(void)
{
    struct evtloop *loop;
    int rc;

    rc = -1;

    loop = calloc(1, sizeof(*loop));
    if (!expected(loop))
        goto out;

    loop->timers = timerwheel_create();
    if (!loop->timers)
        goto out;

    loop->pollevts = LIST(&loop->pollevts);

    rc = 0;
out:
    if (rc && loop) {
        evtloop_destroy(loop);
        loop = NULL;
    }

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
