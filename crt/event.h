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

#define EVENT_NULL                              \
    (struct event_info) {                       \
        .name = NULL,                           \
        .offset = 0,                            \
    }

struct event *event_lookup(void *obj, const char *name,
                           const struct event_info *tab);

struct event * event_splice(struct event **link);

void event_destroy(struct event *evt);

typedef void (*event_fn)(void *data, va_list ap);

void event_connect(struct event *evt, event_fn fn, void *data);

void event_emitv(struct event *evt, va_list ap);

void event_emit(struct event *evt, ...);

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
