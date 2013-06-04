#ifndef MCC_MCC_H
#define MCC_MCC_H

#include <crt/evtloop.h>

struct mcc * mcc_create(void);

void mcc_destroy(struct mcc *);

int mcc_run(struct mcc *, struct evtloop *);

void mcc_stop(struct mcc *);

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
