#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <msp/dispatch-internal.h>
#include <msp/defs.h>

#include <stdlib.h>

int
msp_dispatch_add(struct msp_dispatch *dsp,
                 msp_cmd_t cmd, struct msp_msg_handler *hnd)
{
    int rc;

    rc = -1;

    if (unexpected(cmd < 0) ||
        unexpected(cmd >= array_size(dsp->tab))) {
        errno = EINVAL;
        goto out;
    }

    list_insert_tail(&dsp->tab[cmd], &hnd->entry);

    rc = 0;
out:
    return rc;
}

static void
msp_dispatch_del(struct msp_dispatch *dsp,
                 struct msp_msg_handler *hnd)
{
    list_remove(&hnd->entry);
}

struct msp_dispatch *
msp_dispatch_create(struct msp *msp)
{
    struct msp_dispatch *dsp;
    int i;

    dsp = calloc(1, sizeof(*dsp));
    if (!expected(dsp))
        goto out;

    for (i = 0; i < array_size(dsp->tab); i++)
        dsp->tab[i] = LIST(&dsp->tab[i]);

out:
    return dsp;
}

void
msp_dispatch_destroy(struct msp_dispatch *dsp)
{
    int i;

    for (i = 0; i < array_size(dsp->tab); i++) {
        struct msp_msg_handler *hnd, *next;

        list_for_each_entry_safe(&dsp->tab[i], hnd, next, entry)
            msp_dispatch_del(dsp, hnd);
    }

    free(dsp);
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
