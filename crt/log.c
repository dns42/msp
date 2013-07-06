#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <crt/log-internal.h>
#include <crt/evtloop.h>
#include <crt/defs.h>

#include <stdlib.h>
#include <string.h>

#if 0
static char log_prefix[] = {`
    [LOG_ALERT] = 'A',
    [LOG_CRIT] = 'C',
    [LOG_ERR] = 'E',
    [LOG_WARNING] = 'W',
    [LOG_NOTICE] = 'N',
    [LOG_INFO]= 'I',
    [LOG_DEBUG] = 'D',
};
#endif

static struct log_target *log_target = NULL;

int
log_open(const char *desc, struct evtloop *loop)
{
    const struct log_iface *iface;
    struct log_target *log;
    char *dup, *name, *arg;
    int rc;

    rc = -1;
    iface = NULL;

    dup = arg = strdup(desc);
    if (!expected(arg))
        goto out;

    name = strsep(&arg, ":");

    switch (name[0]) {
    case 's':
        if (!strcmp(name, "syslog")) {
            iface = &log_syslog_iface;
            break;
        }
        if (!strcmp(name, "stdio")) {
            iface = &log_stdio_iface;
            break;
        }
        break;
    }

    if (!expected(iface)) {
        errno = ENOENT;
        goto out;
    }

    log = calloc(1, sizeof(*log));
    if (!expected(log))
        goto out;

    log->iface = iface;

    rc = iface->open(arg, loop, &log->priv);
    if (rc)
        goto out;

    log_close();
    log_target = log;

    rc = 0;
out:
    if (dup)
        free(dup);

    return rc;
}

void
log_close(void)
{
    struct log_target *log;

    log = log_target;

    if (log) {
        log->iface->close(log->priv);
        free(log);

        log_target = NULL;
    }
}

void
log_vprintf(const struct log_hdr *hdr, const char *fmt, va_list ap)
{
    struct log_target *log;
    char buf[80];

    log = log_target;

    if (log) {
        int n;

        n = 0;
        if (n < sizeof(buf))
            n += vsnprintf(buf + n, sizeof(buf), fmt, ap);

        if (n >= sizeof(buf))
            strcpy(buf + sizeof(buf) - strlen("...") - 1, "...");

        log->iface->write(log->priv, hdr->prio, buf);
    }
}

void
log_printf(const struct log_hdr *hdr, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    log_vprintf(hdr, fmt, ap);
    va_end(ap);
}

static void __initcall
liblog_init(void)
{
    log_open("stdio:/dev/stderr", NULL);
}

static void __exitcall
liblog_exit(void)
{
    log_close();
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
