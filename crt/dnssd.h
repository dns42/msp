#ifndef CRT_DNSSD_H
#define CRT_DNSSD_H

#include <crt/evtloop.h>

int dnssd_plug(struct evtloop *loop);

void dnssd_unplug(void);

int dnssd_plugged(void);

int dnssd_register(void);

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
