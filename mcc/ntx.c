#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/ntx-internal.h>
#include <mcc/nrx_rpc.h>

#include <crt/log.h>

#include <stdlib.h>
#include <unistd.h>

struct ntx *
ntx_open(struct sockaddr_in *addr, unsigned long vers)
{
    struct ntx *ntx;
    struct timeval wait;
    int rc;

    ntx = NULL;

    rc = expected(addr->sin_family == AF_INET) ? 0 : -1;
    if (rc) {
        errno = EINVAL;
        goto out;
    }

    vers = vers ? : NRX_V1;

    ntx = calloc(1, sizeof(*ntx));

    rc = expected(ntx) ? 0 : -1;
    if (rc)
        goto out;

    ntx->sock = socket(addr->sin_family, SOCK_DGRAM, 0);

    rc = expected(ntx->sock >= 0) ? 0 : -1;
    if (rc)
        goto out;

    rc = connect(ntx->sock, (struct sockaddr *) addr, sizeof(*addr));
    if (rc) {
        perror("connect");
        goto out;
    }

    wait = (struct timeval) { 0, 0 };

    ntx->clnt = clntudp_create(addr,
                               NRX_PROGRAM, vers,
                               wait, &ntx->sock);

    rc = expected(ntx->clnt) ? 0 : -1;
out:
    if (ntx && rc) {
        ntx_close(ntx);
        ntx = NULL;
    }

    return ntx;
}

void
ntx_close(struct ntx *ntx)
{
    if (ntx->clnt)
        clnt_destroy(ntx->clnt);

    if (ntx->sock >= 0)
        close(ntx->sock);

    free(ntx);
}

int
ntx_sockname(struct ntx *ntx, struct sockaddr_in *addr)
{
    socklen_t alen;

    alen = sizeof(*addr);

    return getsockname(ntx->sock, (struct sockaddr *) addr, &alen);
}

int
ntx_peername(struct ntx *ntx, struct sockaddr_in *addr)
{
    socklen_t alen;

    alen = sizeof(*addr);

    return getpeername(ntx->sock, (struct sockaddr *) addr, &alen);
}

void
ntx_send(struct ntx *ntx, uint16_t *val, int len)
{
    void *arg;
    struct timeval timeo;
    enum clnt_stat cs;
    int rc;

    arg = &(NRXv1_RCarg) {
        .NRXv1_RCarg_len = len,
        .NRXv1_RCarg_val = val,
    };

    timeo = (struct timeval) { 0, 0 };

    cs = clnt_call(ntx->clnt, NRX1_RC,
                   (xdrproc_t) xdr_NRXv1_RCarg, arg,
                   NULL, NULL, timeo);

    rc = expected(cs == RPC_TIMEDOUT) ? 0 : -1;
    if (rc)
        warn("unexpected clnt_stat %d", cs);
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
