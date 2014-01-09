#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <crt/timer-internal.h>
#include <crt/log.h>

#include <stdlib.h>
#include <assert.h>

struct timer *
__timer_create(timer_fn fn, void *data,
               struct timerwheel *wheel)
{
    struct timer *timer;

    timer = calloc(1, sizeof(*timer));
    if (!expected(timer))
        goto out;

    timer->entry = LIST(&timer->entry);
    timer->fn = fn;
    timer->data = data;
    timer->wheel = wheel;
out:
    return timer;
}

void
timer_destroy(struct timer *timer)
{
    timer_stop(timer);
    free(timer);
}

void
timer_interval(struct timer *timer, const struct timeval *rel)
{
    struct timeval now;

    gettimeofday(&now, NULL);

    timer_restart(timer, &now, rel);
}

void
timer_restart(struct timer *timer,
              const struct timeval *now,
              const struct timeval *rel)
{
    struct timeval timeo;

    timeradd(now, rel, &timeo);

    timer_timeout(timer, &timeo);
}

void
timer_timeout(struct timer *timer, const struct timeval *abs)
{
    timer_stop(timer);

    timer->timeo = *abs;

    timerwheel_insert(timer->wheel, timer);
}

void
timer_stop(struct timer *timer)
{
    list_remove_init(&timer->entry);
}

struct timerwheel *
timerwheel_create(void)
{
    struct timerwheel *wheel;

    wheel = calloc(1, sizeof(*wheel));
    if (!expected(wheel))
        goto out;

    list_init(&wheel->list);

out:
    return wheel;
}

void
timerwheel_destroy(struct timerwheel *wheel)
{
    struct timer *timer, *next;

    list_for_each_entry_safe(&wheel->list, timer, next, entry)
        timer_stop(timer);

    free(wheel);
}

void
timerwheel_insert(struct timerwheel *wheel, struct timer *timer)
{
    struct timer *next;

    timer_stop(timer);

    list_for_each_entry(&wheel->list, next, entry)
        if (timercmp(&next->timeo, &timer->timeo, >)) {
            list_insert_before(&next->entry, &timer->entry);
            return;
        }

    list_insert_tail(&wheel->list, &timer->entry);
}

void
timerwheel_run(struct timerwheel *wheel, struct timeval *now)
{
    struct timer *timer, *next;

    list_for_each_entry_safe(&wheel->list, timer, next, entry) {
        if (timercmp(now, &timer->timeo, <))
            gettimeofday(now, NULL);

        if (timercmp(now, &timer->timeo, <))
            break;

        timer_stop(timer);

        if (timer->fn)
            timer->fn(&timer->timeo, timer->data);
    }
}

int
timerwheel_timeo(struct timerwheel *wheel,
                 struct timeval *now, struct timeval **timeo)
{
    struct timer *timer;

    timer = list_first_entry(&wheel->list, struct timer, entry);
    if (!timer) {
        *timeo = NULL;
        return 0;
    }

    if (timercmp(now, &timer->timeo, <)) {
        *timeo = &timer->timeo;
        return 0;
    }

    errno = ETIMEDOUT;
    return -1;
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
