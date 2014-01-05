#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/mcc-internal.h>

#include <crt/log.h>
#include <crt/event.h>

#include <stdlib.h>

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
    event_unlink_tab(&mcc->events, mcc_events_tab);
    free(mcc);
}

struct signal *
mcc_link(struct mcc *mcc, const char *event)
{
    return event_lookup(&mcc->events, event, mcc_events_tab);
}

void
mcc_stop(struct mcc *mcc)
{
    mcc->stop = 1;
}

int
mcc_run(struct mcc *mcc,
        struct evtloop *loop)
{
    int rc;

    rc = 0;

    while (!mcc->stop) {

        rc = evtloop_iterate(loop);

        if (rc && unexpected(errno != EINTR))
            break;
    }

    mcc->stop = 0;

    return rc;
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
