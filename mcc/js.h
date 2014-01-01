#ifndef MCC_JS_H
#define MCC_JS_H

#include <crt/evtloop.h>
#include <sys/stat.h>

struct js * js_open(const char *path);

void js_close(struct js *js);

const char *js_name(struct js *js);

int js_stat(struct js *js, struct stat *st);

#define JS_VAL_NIL -32768
#define JS_VAL_MIN -32767
#define JS_VAL_MAX  32767

enum js_ctl_type {
    JS_AXIS = 'A',
    JS_BUTTON = 'B',
};

int js_value(struct js *js, enum js_ctl_type type, int idx);

int js_axis(struct js *js, int idx);

int js_button(struct js *js, int idx);

struct event * js_link(struct js *js,
                       enum js_ctl_type, int idx, const char *name);

const char * js_ctl_type_name(enum js_ctl_type type);

int js_plug(struct js *js, struct evtloop *loop);

void js_unplug(struct js *js);

int js_plugged(struct js *js);

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
