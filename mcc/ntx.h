#ifndef MCC_NTX_H
#define MCC_NTX_H

#include <netinet/in.h>

struct ntx *ntx_connect(struct sockaddr_in *addr, unsigned long vers);

void ntx_close(struct ntx *ntx);

int ntx_sockname(struct ntx *ntx, struct sockaddr_in *addr);

int ntx_peername(struct ntx *ntx, struct sockaddr_in *addr);

void ntx_send(struct ntx *ntx, uint16_t *val, int len);

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
