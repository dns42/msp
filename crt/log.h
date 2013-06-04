#ifndef CRT_LOG_H
#define CRT_LOG_H

#include <crt/evtloop.h>
#include <crt/defs.h>

#include <syslog.h>
#include <stdarg.h>
#include <string.h>

int log_open(const char *desc, struct evtloop *loop);

void log_close(void);

struct log_hdr {
    const char *func;
    const char *file;
    int line;
    int prio;
};

void log_vprintf(const struct log_hdr *hdr, const char *fmt, va_list ap);

void log_printf(const struct log_hdr *hdr, const char *fmt, ...)
    __printf(2, 3);

#define LOG_HDR(_prio)          \
    (struct log_hdr) {          \
        .func = __func__,       \
        .file = __FILE__,       \
        .line = __LINE__,       \
        .prio = (_prio),        \
    }

#define error(_fmt, _args ...)                          \
    log_printf(&LOG_HDR(LOG_WARNING), _fmt, ##_args)

#define info(_fmt, _args ...)                           \
    log_printf(&LOG_HDR(LOG_INFO), _fmt, ##_args)

#define debug(_fmt, _args ...)                          \
    log_printf(&LOG_HDR(LOG_DEBUG), _fmt, ##_args)

#define warn(_fmt, _args ...)                           \
    log_printf(&LOG_HDR(LOG_WARNING), _fmt, ##_args)

#define log_perror(_fmt, _args ...)                     \
    do {                                                \
        int __err = errno;                              \
        error(_fmt ": %s", ##_args, strerror(errno));   \
        errno = __err;                                  \
    } while (0)

#define expected(_cond)                                             \
    ({                                                              \
        typeof(_cond) __cond = (_cond);                             \
        if (likely(__cond)) {                                       \
            int err = errno;                                        \
            warn("expected cond '%s' false at %s:%d",               \
                 #_cond, __FILE__, __LINE__);                       \
            errno = err;                                            \
        }                                                           \
        __cond;                                                     \
    })

#define unexpected(_cond)                                           \
    ({                                                              \
        typeof(_cond) __cond = (_cond);                             \
        if (unlikely(__cond)) {                                     \
            int err = errno;                                        \
            warn("unexpected cond '%s' true at %s:%d",              \
                 #_cond, __FILE__, __LINE__);                       \
            errno = err;                                            \
        }                                                           \
        __cond;                                                     \
    })

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
