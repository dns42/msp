#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <crt/event-internal.h>

#include <stdlib.h>
#include <string.h>

struct event *
event_splice(struct event **link)
{
    struct event *evt;

    evt = malloc(sizeof(*evt));
    if (!evt)
        goto out;

    evt->link = *link ? (*link)->link : link;
out:
    return evt;
}

struct event *
event_lookup(void *obj, const char *name,
             const struct event_info *tab)
{
    const struct event_info *info;
    struct event *evt, **link;

    evt = NULL;

    for (info = tab;
         info = info->name ? info : NULL, info != NULL;
         info++)

        if (!strcmp(name, info->name))
            break;

    if (!info) {
        errno = ESRCH;
        goto out;
    }

    link = obj - info->offset;

    evt = event_splice(link);
out:
    return evt;
}

void
event_destroy(struct event *evt)
{
    if (list_is_empty(&evt->list))

        *evt->link = NULL;

    else {
        struct event *head = *evt->link;

        if (evt == head) {
            *evt->link = event_next(evt);

            list_remove(&evt->list);
        }
    }

    evt->link = NULL;

    free(evt);
}

void
event_connect(struct event *evt, event_fn fn, void *data)
{
    struct event *head;

    head = *evt->link;

    evt->fn = fn;
    evt->data = data;

    if (head)
        list_insert_tail(&head->list, &evt->list);
    else {
        list_init(&evt->list);

        *evt->link = evt;
    }
}

void
event_emitv(struct event *evt, va_list ap)
{
    while (evt) {
        struct event *next;

        next = __list_next_entry(evt, list);

        evt->fn(evt->data, ap);

        evt = next;
    }
}

void
event_emit(struct event *evt, ...)
{
    va_list ap;

    va_start(ap, evt);
    event_emitv(evt, ap);
    va_end(ap);
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
