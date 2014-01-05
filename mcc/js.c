#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/js-internal.h>

#include <crt/defs.h>
#include <crt/log.h>
#include <crt/evtloop.h>
#include <crt/event.h>

#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

static const struct event_info js_axis_event_tab[] = {
    EVENT_INFO(struct js_axis, changed),
    EVENT_INFO_NULL,
};

static const struct event_info js_button_event_tab[] = {
    EVENT_INFO(struct js_button, pressed),
    EVENT_INFO(struct js_button, released),
    EVENT_INFO(struct js_button, changed),
    EVENT_INFO_NULL,
};

static const struct event_info js_event_tab[] = {
    EVENT_INFO(struct js, destroy),
    EVENT_INFO_NULL,
};

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
js_update_button(struct js *js, int idx, int val)
{
    struct js_button *btn;

    btn = &js->buttons[idx];
    assert(idx < js->nbuttons);

    btn->val = val;

    if (val)
        event_emit(btn->pressed, JS_BUTTON, idx, val);
    else
        event_emit(btn->released, JS_BUTTON, idx, val);

    event_emit(btn->changed, JS_BUTTON, idx, val);
}

static void
js_update_axis(struct js *js, int idx, int val)
{
    struct js_axis *axis;

    axis = &js->axes[idx];
    assert(idx < js->naxes);

    axis->val = val;

    event_emit(axis->changed, JS_AXIS, idx, val);
}

static void
js_update(struct js *js, enum js_ctl_type type, int idx, int val)
{
    switch (type) {
    case JS_BUTTON:
        js_update_button(js, idx, val);
        break;

    case JS_AXIS:
        js_update_axis(js, idx, val);
        break;

    default:
        abort();
    }
}

static void
js_reset(struct js *js)
{
    int idx;

    js_foreach_button(js, idx)
        js_update_button(js, idx, JS_VAL_NIL);

    js_foreach_axis(js, idx)
        js_update_axis(js, idx, JS_VAL_NIL);
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

        js_update(js, type, jse.number, jse.value);
    } while (1);
}

int
js_value(struct js *js, enum js_ctl_type type, int idx)
{
    int val;

    val = JS_VAL_NIL;

    switch (type) {
    case JS_BUTTON:
        val = js_button(js, idx);
        break;

    case JS_AXIS:
        val = js_axis(js, idx);
        break;

    default:
        errno = EINVAL;
    }

    return val;
}

int
js_axis(struct js *js, int idx)
{
    int val;

    val = JS_VAL_NIL;

    if (!js_plugged(js)) {
        errno = ENODEV;
        goto out;
    }

    if (idx >= js->naxes) {
        errno = ERANGE;
        goto out;
    }

    val = js->axes[idx].val;
out:
    return val;
}

int
js_button(struct js *js, int idx)
{
    int val;

    val = JS_VAL_NIL;

    if (!js_plugged(js)) {
        errno = ENODEV;
        goto out;
    }

    if (idx >= js->nbuttons) {
        errno = ERANGE;
        goto out;
    }

    val = js->buttons[idx].val;
out:
    return val;
}

static void
js_pollevt(int revents, void *data)
{
    struct js *js = data;

    if (revents & POLLIN)
        js_read_events(js);
}

struct js *
js_open(const char *path)
{
    struct js *js;
    __u8 cnt;
    int rc;

    js = calloc(1, sizeof(*js));

    rc = expected(js) ? 0 : -1;
    if (rc)
        goto out;

    js->fd = open(path, O_RDONLY | O_NONBLOCK);

    rc = js->fd < 0 ? -1 : 0;
    if (rc)
        goto out;

    rc = ioctl(js->fd, JSIOCGAXES, &cnt);
    if (unexpected(rc)) {
        log_perror("JSIOCGAXES");
        goto out;
    }

    js->naxes = cnt;
    js->axes = calloc(cnt, sizeof(*js->axes));

    debug("axes %p", js->axes);

    rc = expected(js->axes) ? 0 : -1;
    if (rc)
        goto out;

    rc = ioctl(js->fd, JSIOCGBUTTONS, &cnt);
    if (unexpected(rc)) {
        perror("JSIOCGBUTTONS");
        goto out;
    }

    js->nbuttons = cnt;
    js->buttons = calloc(cnt, sizeof(*js->buttons));

    debug("buttons %p", js->buttons);

    rc = expected(js->buttons) ? 0 : -1;
    if (rc)
        goto out;

    js_reset(js);
    js_read_events(js);
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
    int idx;

    js_unplug(js);

    js_foreach_button(js, idx) {
        event_unlink(js->buttons[idx].pressed);
        event_unlink(js->buttons[idx].released);
        event_unlink(js->buttons[idx].changed);
    }

    js_foreach_axis(js, idx)
        event_unlink(js->axes[idx].changed);

    if (js->name)
        free(js->name);

    if (js->fd >= 0)
        close(js->fd);

    free(js->axes);
    free(js->buttons);

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

static struct signal *
js_link_axis(struct js *js, int idx, const char *change)
{
    struct signal *sig;

    sig = NULL;

    if (unexpected(idx < 0) ||
        unexpected(idx >= js->naxes)) {
        errno = EINVAL;
        goto out;
    }

    sig = event_lookup(&js->axes[idx], change,
                       js_axis_event_tab);
out:
    return sig;
}

static struct signal *
js_link_button(struct js *js, int idx, const char *change)
{
    struct signal *sig;

    sig = NULL;

    if (unexpected(idx < 0) ||
        unexpected(idx >= js->nbuttons)) {
        errno = EINVAL;
        goto out;
    }

    sig = event_lookup(&js->buttons[idx], change,
                       js_button_event_tab);
out:
    return sig;
}

struct signal *
js_ctl_link(struct js *js,
            enum js_ctl_type type, int idx,
            const char *change)
{
    struct signal *sig;

    sig = NULL;

    switch (type) {
    case JS_AXIS:
        sig = js_link_axis(js, idx, change);
        break;

    case JS_BUTTON:
        sig = js_link_button(js, idx, change);
        break;

    default:
        errno = EINVAL;
    }

    return sig;
}

struct signal *
js_link(struct js *js, const char *change)
{
    return event_lookup(js, change, js_event_tab);
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
