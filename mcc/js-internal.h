#ifndef MCC_JS_INTERNAL_H
#define MCC_JS_INTERNAL_H

#include <mcc/js.h>

struct js_button {
    int val;
    struct event *pressed;
    struct event *released;
    struct event *changed;
};

struct js_axis {
    int val;
    struct event *changed;
} *axes;

struct js {
    int fd;
    char *name;
    struct pollevt *evt;

    int naxes;
    struct js_axis *axes;

    int nbuttons;
    struct js_button *buttons;

    struct event *destroy;
};

#define js_foreach_ctl(_js, _type, _idx)        \

#define js_foreach_button(_js, _idx)            \
    for (_idx = 0; _idx < (_js)->nbuttons; _idx++)

#define js_foreach_axis(_js, _idx)              \
    for (_idx = 0; _idx < (_js)->naxes; _idx++)

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
