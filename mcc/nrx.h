#ifndef MCC_NRX_H
#define MCC_NRX_H

#include <netinet/in.h>
#include <stdint.h>
#include <crt/evtloop.h>

struct nrx *nrx_bind(int nchn,
                     struct sockaddr_in *addr,
                     unsigned long vers);

void nrx_close(struct nrx *tx);

int nrx_sockname(struct nrx *nrx, struct sockaddr_in *addr);

int nrx_plugged(struct nrx *nrx);

int nrx_plug(struct nrx *nrx, struct evtloop *loop);

void nrx_unplug(struct nrx *nrx);

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
