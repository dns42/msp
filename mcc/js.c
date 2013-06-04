#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/js-internal.h>

#include <crt/defs.h>
#include <crt/log.h>
#include <crt/evtloop.h>

#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

int
js_stat(struct js *js, struct stat *st)
{
    return fstat(js->fd, st);
}

const char *
js_name(struct js *js)
{

    if (!js->name) {
        size_t len;
        char *name;

        len = 32;
        name = NULL;

        do {
            char *tmp;
            ssize_t cnt;

            tmp = realloc(name, len);
            if (!tmp)
                break;

            name = tmp;
            memset(name, 0, len);

            cnt = ioctl(js->fd, JSIOCGNAME(len), name);

            if (unexpected(cnt < 0))
                break;

            if (cnt < len) {
                js->name = realloc(name, cnt + 1);
                name = NULL;
                break;
            }

            len *= 2;
        } while (1);

        free(name);
    }

    return js->name;
}

const char *
js_ctl_type_name(enum js_ctl_type type)
{
    const char *name;

    switch (type) {
    case JS_BUTTON:
        name = "button";
        break;
    case JS_AXIS:
        name = "axis";
        break;
    default:
        unexpected(name = NULL);
    }

    return name;
}

static void
js_store(struct js *js, enum js_ctl_type type, int idx, int val)
{
    struct js_ctlv *ctls;
    struct js_sig *sig;

    ctls = &js->ctls[type];

    assert(type < JS_N_CTL_TYPES);
    assert(idx < ctls->cnt);

    ctls->val[idx] = val;

    sig = &ctls->sig[idx];
    if (sig->fn)
        sig->pending = 1;
}

static void
js_reset(struct js *js)
{
    int idx;

    js_foreach_button(js, idx)
        js_store(js, JS_BUTTON, idx, JS_VAL_NIL);

    js_foreach_axis(js, idx)
        js_store(js, JS_BUTTON, idx, JS_VAL_NIL);
}

static void
js_error(struct js *js)
{
    js_unplug(js);
    js_reset(js);
}

static void
js_read_events(struct js *js)
{
    do {
        struct js_event jse;
        enum js_ctl_type type;
        ssize_t n;

        n = read(js->fd, &jse, sizeof(jse));
        if (n < 0) {
            if (errno != EAGAIN) {
                perror("read");
                js_error(js);
            }
            break;
        }

        switch (jse.type & ~JS_EVENT_INIT) {
        case JS_EVENT_BUTTON:
            type = JS_BUTTON;
            break;
        case JS_EVENT_AXIS:
            type = JS_AXIS;
            break;
        default:
            error("jse.type %x", jse.type);
            js_error(js);
            return;
        }

        js_store(js, type, jse.number, jse.value);
    } while (1);
}

int
js_value(struct js *js, enum js_ctl_type type, int idx)
{
    int val;
    struct js_ctlv *ctls;

    val = JS_VAL_NIL;

    if (!js_plugged(js)) {
        errno = ENODEV;
        goto out;
    }

    ctls = &js->ctls[type];

    if (idx >= ctls->cnt) {
        errno = EINVAL;
        goto out;
    }

    val = ctls->val[idx];
out:
    return val;
}

int
js_button(struct js *js, int idx)
{
    return js_value(js, JS_BUTTON, idx);
}

int
js_axis(struct js *js, int idx)
{
    return js_value(js, JS_BUTTON, idx);
}

static void
js_emit_type(struct js *js, enum js_ctl_type type)
{
    struct js_ctlv *ctls;
    int i;

    ctls = &js->ctls[type];

    for (i = 0; i < ctls->cnt; i++) {
        struct js_sig *sig = &ctls->sig[i];

        if (sig->pending) {
            sig->fn(js, type, i, ctls->val[i], sig->data);
            sig->pending = 0;
        }
    }
}

static void
js_emit_events(struct js *js)
{
    js_emit_type(js, JS_BUTTON);
    js_emit_type(js, JS_AXIS);
}

static void
js_pollevt(int revents, void *data)
{
    struct js *js = data;

    if (revents & POLLIN) {
        js_read_events(js);
        js_emit_events(js);
    }
}

struct js *
js_open(const char *path)
{
    struct js *js;
    struct js_ctlv *ctls;
    __u8 cnt;
    int rc;

    rc = -1;

    js = calloc(1, sizeof(*js));
    if (!expected(js))
        goto out;

    js->fd = open(path, O_RDONLY | O_NONBLOCK);
    if (js->fd < 0)
        goto out;

    rc = ioctl(js->fd, JSIOCGAXES, &cnt);
    if (unexpected(rc)) {
        log_perror("JSIOCGAXES");
        goto out;
    }

    rc = -1;
    ctls = &js->ctls[JS_AXIS];

    ctls->cnt = cnt;
    ctls->val = calloc(cnt, sizeof(*ctls->val));
    ctls->sig = calloc(cnt, sizeof(*ctls->sig));
    if (!expected(ctls->val) ||
        !expected(ctls->sig))
        goto out;

    rc = ioctl(js->fd, JSIOCGBUTTONS, &cnt);
    if (unexpected(rc)) {
        perror("JSIOCGBUTTONS");
        goto out;
    }

    rc = -1;
    ctls = &js->ctls[JS_BUTTON];

    ctls->cnt = cnt;
    ctls->val = calloc(cnt, sizeof(*ctls->val));
    ctls->sig = calloc(cnt, sizeof(*ctls->sig));
    if (!expected(ctls->val) ||
        !expected(ctls->sig))
        goto out;

    js_reset(js);
    js_read_events(js);

    rc = 0;
out:
    if (rc) {
        int err = errno;

        js_close(js);
        js = NULL;

        errno = err;
    }

    return js;
}

void
js_close(struct js *js)
{
    js_unplug(js);

    if (js->name)
        free(js->name);

    if (js->fd >= 0)
        close(js->fd);

    free(js->ctls[JS_AXIS].val);
    free(js->ctls[JS_AXIS].sig);

    free(js->ctls[JS_BUTTON].val);
    free(js->ctls[JS_BUTTON].sig);

    free(js);
}

int
js_plugged(struct js *js)
{
    return js->evt != NULL;
}

int
js_plug(struct js *js, struct evtloop *loop)
{
    int rc;

    js->evt = evtloop_add_pollfd(loop, js->fd,
                                 js_pollevt, js);

    rc = expected(js->evt) ? 0 : -1;
    if (rc)
        goto out;

    pollevt_select(js->evt, POLLIN);
out:
    return rc;
}

void
js_unplug(struct js *js)
{
    if (js->evt) {
        pollevt_destroy(js->evt);
        js->evt = NULL;
    }
}

int
js_evt_connect(struct js *js,
               enum js_ctl_type type, int idx,
               js_evt_fn fn, void *data)
{
    struct js_ctlv *ctls;
    int rc;

    rc = -1;

    if (type != JS_BUTTON && type != JS_AXIS) {
        errno = EINVAL;
        goto out;
    }

    ctls = &js->ctls[type];

    if (unexpected(idx < 0) || idx >= ctls->cnt) {
        errno = EINVAL;
        goto out;
    }

    if (ctls->sig[idx].fn) {
        errno = EBUSY;
        goto out;
    }

    ctls->sig[idx] = (struct js_sig) {
        .fn = fn,
        .data = data,
    };

    rc = 0;
out:
    return rc;
}

void
js_evt_disconnect(struct js *js, enum js_ctl_type type, int idx)
{
    struct js_ctlv *ctls;

    if (unexpected(type != JS_BUTTON &&
                   type != JS_AXIS))
        return;

    ctls = &js->ctls[type];

    if (unexpected(idx < 0 || idx >= ctls->cnt))
        return;

    ctls->sig[idx] = (struct js_sig) {
        .fn = NULL,
        .data = NULL,
    };
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
