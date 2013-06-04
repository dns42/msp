#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/nrx-internal.h>
#include <mcc/nrxrpc.h>

#include <crt/log.h>

#include <stdlib.h>
#include <unistd.h>

static struct list nrx_list = LIST(&nrx_list);

static struct nrx *
nrx_find(SVCXPRT *xprt)
{
    struct nrx *nrx;

    list_for_each_entry(&nrx_list, nrx, entry)
        if (nrx->xprt == xprt)
            break;

    return &nrx->entry == &nrx_list ? NULL : nrx;
}

static void
nrx_reset(struct nrx *nrx)
{
    int i;

    for (i = 0; i < nrx->nchn; i++)
        nrx->chn[i] = 0;
}

static void
nrx_error(struct nrx *nrx)
{
    nrx_unplug(nrx);
    nrx_reset(nrx);
}

static void *
nrx_1_rc_svc(struct nrx *nrx,
             NRXv1_RCarg *arg, struct svc_req *rq)
{
    uint16_t *val;
    int len, i;

    len = arg->NRXv1_RCarg_len;
    val = arg->NRXv1_RCarg_val;

    for (i = 0; i < nrx->nchn && i < len; i++)
        nrx->chn[i] = val[i];

    for (; i < nrx->nchn; i++)
        nrx->chn[i] = 0;

    return NULL;
}

static void
nrx_prog_1(struct svc_req *rq, SVCXPRT *xprt)
{
    union {
        NRXv1_RCarg nrxv1_rc_1_arg;
    } _arg;
    void *arg, *res;
    xdrproc_t _xdr_arg, _xdr_res;
    typedef char *(*nrx_svc_t)(struct nrx *,
                               void *, struct svc_req *);
    nrx_svc_t _nrx_svc;
    struct nrx *nrx;
    bool_t ok;

    arg = NULL;
    res = NULL;

    nrx = nrx_find(xprt);

    if (!expected(nrx)) {
        svcerr_systemerr(xprt);
        return;
    }

    switch (rq->rq_proc) {
    case NULLPROC:
        svc_sendreply(xprt, (xdrproc_t) xdr_void, NULL);
        return;

    case NRXv1_RC:
        _xdr_arg = (xdrproc_t) xdr_NRXv1_RCarg;
        _xdr_res = (xdrproc_t) NULL;
        _nrx_svc = (nrx_svc_t) nrx_1_rc_svc;
        break;

    default:
        svcerr_noproc(xprt);
        return;
    }

    memset(&_arg, 0, sizeof(_arg));

    arg = svc_getargs(xprt, _xdr_arg, (caddr_t) &_arg)
        ? &_arg
        : NULL;

    if (!arg) {
        svcerr_decode(xprt);
        return;
    }

    res = _nrx_svc(nrx, arg, rq);

    if (unlikely(res)) {

        ok = !svc_sendreply(xprt, _xdr_res, res);

        if (!expected(ok))
            svcerr_systemerr(xprt);
    }

    ok = svc_freeargs(xprt, _xdr_arg, (caddr_t) &arg);

    if (!expected(ok))
        nrx_error(nrx);
}

static void
nrx_recv(struct nrx *nrx)
{
    FD_SET(nrx->sock, &nrx->rfds);

    svc_getreqset(&nrx->rfds);
}

struct nrx *
nrx_bind(int nchn, struct sockaddr_in *addr, unsigned long vers)
{
    struct nrx *nrx;
    bool_t ok;
    int rc;

    nrx = NULL;

    rc = expected(addr->sin_family == AF_INET) ? 0 : -1;
    if (rc) {
        errno = EINVAL;
        goto out;
    }

    vers = vers ? : NRXv1;

    nrx = calloc(1, offsetof(struct nrx, chn[nchn]));

    rc = expected(nrx) ? 0 : -1;
    if (rc)
        goto out;

    nrx->nchn = nchn;
    nrx->sock = socket(addr->sin_family, SOCK_DGRAM, 0);

    rc = expected(nrx->sock >= 0) ? 0 : -1;
    if (rc)
        goto out;

    rc = bind(nrx->sock, (struct sockaddr *) addr, sizeof(*addr));
    if (rc) {
        perror("bind");
        goto out;
    }

    nrx->xprt = svcudp_create(nrx->sock);

    rc = expected(nrx->xprt) ? 0 : -1;
    if (rc)
        goto out;

    ok = svc_register(nrx->xprt,
                      NRX_PROGRAM, vers,
                      nrx_prog_1, 0);

    rc = expected(ok) ? 0 : -1;
    if (rc) {
        errno = EFAULT;
        goto out;
    }

    list_insert_tail(&nrx_list, &nrx->entry);
out:
    if (nrx && rc) {
        nrx_close(nrx);
        nrx = NULL;
    }

    return nrx;
}

void
nrx_close(struct nrx *nrx)
{
    list_remove(&nrx->entry);

    if (nrx->xprt)
        svc_destroy(nrx->xprt);

    if (nrx->sock >= 0)
        close(nrx->sock);

    free(nrx);
}

int
nrx_sockname(struct nrx *nrx, struct sockaddr_in *addr)
{
    socklen_t alen;

    alen = sizeof(*addr);

    return getsockname(nrx->sock, (struct sockaddr *) addr, &alen);
}

static void
nrx_emit(struct nrx *nrx)
{
}

int
nrx_plugged(struct nrx *nrx)
{
    return nrx->evt != NULL;
}

static void
nrx_pollevt(int revents, void *data)
{
    struct nrx *nrx = data;

    if (revents & POLLIN) {
        nrx_recv(nrx);
        nrx_emit(nrx);
    }
}

int
nrx_plug(struct nrx *nrx, struct evtloop *loop)
{
    int rc;

    nrx->evt = evtloop_add_pollfd(loop, nrx->sock,
                                  nrx_pollevt, nrx);

    rc = expected(nrx->evt) ? 0 : -1;
    if (rc)
        goto out;

    pollevt_select(nrx->evt, POLLIN);
out:
    return rc;
}

void
nrx_unplug(struct nrx *nrx)
{
    if (nrx->evt) {
        pollevt_destroy(nrx->evt);
        nrx->evt = NULL;
    }
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
