#ifndef CRT_LOG_INTERNAL_H
#define CRT_LOG_INTERNAL_H

#include <crt/log.h>

struct log_iface {
    int (*open)(const char *arg, struct evtloop *, void **priv);
    void (*write)(void *priv, int level, const char *s);
    void (*close)(void *priv);
};

struct log_target {
    const struct log_iface *iface;
    void *priv;
};

#include <crt/log-stdio-internal.h>
#include <crt/log-syslog-internal.h>

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
