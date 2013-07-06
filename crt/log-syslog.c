#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <crt/log-internal.h>

#include <syslog.h>

static int
log_syslog_open(const char *arg, struct evtloop *loop, void **priv)
{
    openlog(arg, 0, LOG_USER);

    return 0;
}

static void
log_syslog_write(void *priv, int prio, const char *s)
{
    syslog(prio, "%s", s);
}

static void
log_syslog_close()
{
    closelog();
}

const struct log_iface log_syslog_iface = {
    .open = log_syslog_open,
    .write = log_syslog_write,
    .close = log_syslog_close,
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
