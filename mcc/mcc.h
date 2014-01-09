#ifndef MCC_MCC_H
#define MCC_MCC_H

#include <crt/evtloop.h>
#include <crt/event.h>

extern struct mcc *g_mcc;

struct mcc * mcc_create(void);

void mcc_destroy(struct mcc *);

struct signal *mcc_link(struct mcc *mcc, const char *event);

int mcc_run(struct mcc *, struct evtloop *);

void mcc_stop(struct mcc *, int status, const char *reason, ...);

int mcc_stopped(struct mcc *mcc);

const char * mcc_reason(struct mcc *mcc);

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
