#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <crt/log-internal.h>

#include <stdlib.h>
#include <stdio.h>

static void
log_stdio_close(void *priv)
{
    struct log_stdio *s = priv;

    if (s->f)
        fclose(s->f);

    free(s);
}

static int
log_stdio_open(const char *arg, struct evtloop *loop, void **priv)
{
    struct log_stdio *s;
    int rc;

    rc = -1;

    s = calloc(1, sizeof(*s));
    if (!s)
        goto out;

    if (arg && arg[0]) {
        s->f = fopen(arg, "w");
        if (unlikely(!s->f)) {
            log_perror("%s", arg);
            goto out;
        }
    } else
        s->f = stdout;

    rc = 0;
    *priv = s;
out:
    if (rc && s) {
        log_stdio_close(s);
        s = NULL;
    }

    return rc;
}

static void
log_stdio_write(void *priv, int level, const char *msg)
{
    struct log_stdio *s = priv;

    fputs(msg, s->f);
    putc('\n', s->f);
}

const struct log_iface log_stdio_iface = {
    .open = log_stdio_open,
    .write = log_stdio_write,
    .close = log_stdio_close,
};

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
