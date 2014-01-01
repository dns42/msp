#ifndef CRT_EVENT_INTERNAL_H
#define CRT_EVENT_INTERNAL_H

#include <crt/event.h>
#include <crt/list.h>

struct event {
    struct list list;
    struct event **link;
    event_fn fn;
    void *data;
};

#define event_next(_evt) \
    __list_next_entry(_evt, list)

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
