#ifndef MCC_JS_INTERNAL_H
#define MCC_JS_INTERNAL_H

#include <mcc/js.h>

struct js_sig {
    js_evt_fn fn;
    void *data;
    int pending;
};

struct js_ctlv {
    int cnt;
    int *val;
    struct js_sig *sig;
};

struct js {
    int fd;
    char *name;
    struct pollevt *evt;
    struct js_ctlv ctls[JS_N_CTL_TYPES];
};

#define js_foreach_ctl(_js, _type, _idx)        \
    for (_idx = 0; _idx < (_js)->ctls[_type].cnt; _idx++)

#define js_foreach_button(_js, _idx)            \
    js_foreach_ctl(_js, JS_BUTTON, _idx)

#define js_foreach_axis(_js, _idx)              \
    js_foreach_ctl(_js, JS_BUTTON, _idx)

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
