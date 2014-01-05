#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <crt/event-internal.h>
#include <crt/log.h>

#include <stdlib.h>
#include <string.h>

static struct event *
event_get(struct event **link)
{
    struct event *evt;

    evt = *link;

    if (!evt) {
        evt = malloc(sizeof(*evt));

        if (expected(evt)) {
            list_init(&evt->signals);
            evt->link = link;

            *link = evt;
        }
    }

    return evt;
}

static void
event_put(struct event *evt)
{
    if (list_is_empty(&evt->signals)) {

        if (evt->link)
            *evt->link = NULL;

        free(evt);
    }
}

void
event_unlink(struct event *evt)
{
    if (evt)
        evt->link = NULL;
}

void
event_unlink_tab(void *obj, const struct event_info *tab)
{
    const struct event_info *info;

    event_tab_foreach(info, tab) {
        struct event **link = obj + info->offset;

        event_unlink(*link);
    }
}

struct signal *
event_splice(struct event **link)
{
    struct signal *sig;
    struct event *evt;
    int rc;

    sig = calloc(1, sizeof(*sig));

    rc = sig ? 0 : -1;
    if (rc)
        goto out;

    evt = event_get(link);

    rc = evt ? 0 : -1;
    if (rc)
        goto out;

    list_init(&sig->entry);
    sig->event = evt;
out:
    if (rc) {
        if (sig) {
            signal_destroy(sig);
            sig = NULL;
        }

        if (evt)
            event_put(evt);
    }

    return sig;
}

struct signal *
event_lookup(void *obj, const char *name,
             const struct event_info *tab)
{
    const struct event_info *info;
    struct signal *sig;
    struct event **link;

    sig = NULL;

    for (info = tab;
         info = info->name ? info : NULL, info != NULL;
         info++)

        if (!strcmp(name, info->name))
            break;

    if (!info) {
        errno = ESRCH;
        goto out;
    }

    link = obj + info->offset;

    sig = event_splice(link);
out:
    return sig;
}

void
signal_destroy(struct signal *sig)
{
    signal_disconnect(sig);
    free(sig);
}

void
signal_connect(struct signal *sig, signal_fn fn, void *data)
{
    signal_disconnect(sig);

    sig->fn = fn;
    sig->data = data;

    list_insert_tail(&sig->event->signals, &sig->entry);
}

void
signal_disconnect(struct signal *sig)
{
    if (!list_is_empty(&sig->entry)) {

        list_remove_init(&sig->entry);

        event_put(sig->event);
    }
}

void
event_emitv(struct event *evt, va_list ap)
{
    struct signal *sig, *next;

    if (evt) {
        list_for_each_entry_safe(&evt->signals, sig, next, entry) {
            va_list cp;

            va_copy(cp, ap);

            sig->fn(sig->data, cp);
        }
    }
}

void
event_emit(struct event *evt, ...)
{
    va_list ap;

    if (evt) {
        va_start(ap, evt);
        event_emitv(evt, ap);
        va_end(ap);
    }
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
