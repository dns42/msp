#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/mcc-internal.h>

#include <crt/log.h>
#include <crt/event.h>

#include <stdlib.h>
#include <stdio.h>

struct mcc *
mcc_create(void)
{
    struct mcc *mcc;
    int rc;

    mcc = calloc(1, sizeof(*mcc));

    rc = expected(mcc) ?  0 : -1;
    if (!rc)
        goto out;

out:
    if (rc && mcc) {
        mcc_destroy(mcc);
        mcc = NULL;
    }

    return mcc;
}

static const struct event_info mcc_events_tab[] = {
    EVENT_INFO(struct mcc_events, start),
    EVENT_INFO(struct mcc_events, stop),
    EVENT_INFO(struct mcc_events, destroy),
    EVENT_INFO_NULL,
};

void
mcc_destroy(struct mcc *mcc)
{
    free(mcc->reason);
    event_unlink_tab(&mcc->events, mcc_events_tab);
    free(mcc);
}

struct signal *
mcc_link(struct mcc *mcc, const char *event)
{
    return event_lookup(&mcc->events, event, mcc_events_tab);
}

void
mcc_stop(struct mcc *mcc, int status, const char *reason, ...)
{
    mcc->stop = 1;
    mcc->exit = status;

    if (reason && !mcc->reason) {
        va_list ap;

        va_start(ap, reason);
        vasprintf(&mcc->reason, reason, ap);
        va_end(ap);
    }
}

const char *
mcc_reason(struct mcc *mcc)
{
    return mcc->reason;
}

int
mcc_run(struct mcc *mcc,
        struct evtloop *loop)
{
    int rc;

    rc = 0;

    while (!mcc_stopped(mcc)) {

        rc = evtloop_iterate(loop);

        if (rc && unexpected(errno != EINTR))
            mcc_stop(mcc, EXIT_FAILURE, strerror(errno));
    }

    return mcc->exit == EXIT_SUCCESS ? 0 : -1;
}

int
mcc_stopped(struct mcc *mcc)
{
    return mcc->stop;
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
