#ifndef CRT_EVENT_H
#define CRT_EVENT_H

#include <stddef.h>
#include <stdarg.h>

struct event_info {
    const char *name;
    ptrdiff_t offset;
};

#define EVENT_INFO(_type, _link)                \
    (struct event_info) {                       \
        .name = #_link,                         \
        .offset = offsetof(_type, _link),       \
    }

#define EVENT_INFO_NULL                         \
    (struct event_info) {                       \
        .name = NULL,                           \
        .offset = 0,                            \
    }

struct event;

struct signal * event_splice(struct event **link);

struct signal * event_lookup(void *obj, const char *name,
                             const struct event_info *tab);

void event_emitv(struct event *evt, va_list ap);

void event_emit(struct event *evt, ...);

void event_unlink(struct event *evt);

void event_unlink_tab(void *obj, const struct event_info *tab);

typedef void (*signal_fn)(void *data, va_list ap);

void signal_connect(struct signal *sig, signal_fn fn, void *data);

void signal_disconnect(struct signal *sig);

void signal_destroy(struct signal *sig);

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
