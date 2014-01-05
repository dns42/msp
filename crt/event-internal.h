#ifndef CRT_EVENT_INTERNAL_H
#define CRT_EVENT_INTERNAL_H

#include <crt/event.h>
#include <crt/list.h>

struct event {
    struct list signals;
    struct event **link;
};

struct signal {
    struct event *event;
    struct list entry;
    signal_fn fn;
    void *data;
};

#define event_tab_foreach(_info, _tab)                              \
    for (_info = (_tab);                                            \
         _info = (_info)->name ? (_info) : NULL, (_info) != NULL;   \
         _info++)

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
