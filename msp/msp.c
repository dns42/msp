#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <msp/msp-internal.h>

#include <msp/msg.h>
#include <msp/str.h>
#include <msp/defs.h>

#include <crt/defs.h>
#include <crt/log.h>

#include <stdlib.h>
#include <assert.h>

static void msp_tty_return(struct msp *);

void
msp_close(struct msp *msp)
{
    free(msp);
}

struct msp *
msp_open(struct tty *tty, struct evtloop *loop)
{
    struct msp *msp;
    int rc;

    rc = -1;

    msp = calloc(1, sizeof(*msp));
    if (!expected(msp))
        goto out;

    msp->tty = tty;
    msp->loop = loop;

    msp_tty_return(msp);

    rc = 0;
out:
    if (rc) {
        int err = errno;

        msp_close(msp);
        msp = NULL;

        errno = err;
    }

    return msp;
}

static struct msp_call **
msp_call_entry(struct msp *msp, msp_cmd_t cmd)
{
    assert(cmd >= MSP_CMD_MIN);
    assert(cmd <= MSP_CMD_MAX);

    return &msp->tab[MSP_TAB_IDX(cmd)];
}

struct msp_call *
msp_call_get(struct msp *msp, msp_cmd_t cmd)
{
    return *msp_call_entry(msp, cmd);
}

static void
msp_call_set(struct msp *msp, msp_cmd_t cmd, struct msp_call *call)
{
    *msp_call_entry(msp, cmd) = call;
}

static void
msp_call_clr(struct msp *msp, msp_cmd_t cmd)
{
    msp_call_set(msp, cmd, NULL);
}

static void
__msp_call_timeo(const struct timeval *timeo, void *data)
{
    struct msp_call **tab = data, *call = *tab;

    call->rfn(ETIMEDOUT, NULL, NULL, call->priv);

    timer_destroy(call->timer);
    call->timer = NULL;
}

static void
__msp_call_destroy(struct msp_call *call)
{
    if (call->timer)
        timer_destroy(call->timer);
    free(call);
}

static void
msp_call_exit(struct msp *msp, msp_cmd_t cmd)
{
    struct msp_call *call;

    call = msp_call_get(msp, cmd);
    if (call != NULL);

    msp_call_clr(msp, cmd);

    __msp_call_destroy(call);
}

static struct msp_call *
msp_call_init(struct msp *msp, msp_cmd_t cmd,
              msp_call_retfn rfn, void *priv,
              const struct timeval *timeo)
{
    struct msp_call *call;
    struct timeval _timeo;
    int rc;

    call = msp_call_get(msp, cmd);

    rc = unexpected(call) ? -1 : 0;
    if (rc) {
        errno = EBUSY;
        goto out;
    }

    call = calloc(1, sizeof(*call));

    rc = expected(call) ? 0 : -1;
    if (rc)
        goto out;

    call->rfn = rfn;
    call->priv = priv;

    gettimeofday(&_timeo, NULL);
    timeradd(&_timeo, timeo, &_timeo);

    call->timer = evtloop_create_timer(msp->loop, &_timeo,
                                       __msp_call_timeo,
                                       msp_call_entry(msp, cmd));

    rc = expected(call->timer) ? 0 : -1;
    if (rc)
        goto out;

    msp_call_set(msp, cmd, call);
out:
    if (rc && call) {
        __msp_call_destroy(call);
        call = NULL;
    }

    return call;
}

static void
msp_tty_recv_data(struct tty *tty, int err, void *priv)
{
    struct msp *msp;
    const struct msp_hdr *hdr;
    struct msp_call *call;
    msp_call_retfn rfn;
    void *data;
    int rc;

    msp = priv;
    hdr = &msp->hdr;

    data = NULL;
    call = msp_call_get(msp, hdr->cmd);

    rc = 0;

    assert(call != NULL);

    if (hdr->len) {
        data = msp->iov[0].iov_base;

        msp->cks ^= msp_msg_checksum(hdr, data);

        rc = msp->cks ? -1 : 0;
        if (rc) {
            errno = unexpected(EBADMSG);
            goto out;
        }

        rc = msp_msg_decode_rsp(hdr, data);
    }

out:
    rfn = call->rfn;
    priv = call->priv;

    msp_call_exit(msp, hdr->cmd);

    if (rc) {
        hdr = NULL;
        data = NULL;
    }

    rfn(rc ? errno : 0, hdr, data, priv);

    msp_tty_return(msp);
}

static void
msp_tty_recv_hdr(struct tty *tty, int err, void *priv)
{
    struct msp *msp;
    struct msp_hdr *hdr;
    struct msp_call *call;
    int cnt;

    msp = priv;
    hdr = &msp->hdr;
    call = NULL;

    assert(msp->iov[0].iov_base == hdr);
    assert(msp->iov[0].iov_len == sizeof(*hdr));

    if (unexpected(hdr->tag[0] != '$' || hdr->tag[1] != 'M'))
        goto bad;

    if (unexpected(hdr->dsc != '!' && hdr->dsc != '>'))
        goto bad;

    call = msp_call_get(msp, hdr->cmd);
    if (!expected(call))
        goto bad;

    cnt = 0;

    if (hdr->len) {
        struct iovec *iov = &msp->iov[0];

        *iov = (struct iovec) {
            .iov_base = malloc(hdr->len),
            .iov_len = hdr->len
        };

        if (!expected(iov->iov_base)) {
            err = expected(errno);
            goto out;
        }

        cnt = 1;
    }

    msp->iov[cnt++] = (struct iovec) {
        &msp->cks,
        sizeof(msp->cks)
    };

    tty_setrxbuf(msp->tty, msp->iov, cnt, msp_tty_recv_data, msp);

    return;

bad:
    err = EBADMSG;

    error("%s cmd %d hdr %c%c%c len %u\n",
          msp_cmd_name(hdr->cmd), hdr->cmd,
          hdr->tag[0], hdr->tag[1], hdr->dsc,
          hdr->len);
out:
    tty_rxflush(tty);
}

static void
msp_tty_return(struct msp *msp)
{
    msp->iov[0] = (struct iovec) {
        .iov_base = &msp->hdr,
        .iov_len = sizeof(msp->hdr),
    };

    tty_setrxbuf(msp->tty, msp->iov, 1, msp_tty_recv_hdr, msp);
}

int
msp_call(struct msp *msp,
         msp_cmd_t cmd, void *args, size_t len,
         msp_call_retfn rfn, void *priv, const struct timeval *timeo)
{
    struct msp_hdr hdr;
    struct iovec iov[3];
    struct msp_call *call;
    uint8_t cks;
    int rc, cnt;

    cnt = 0;
    hdr = MSP_REQ_HDR(cmd, len);

    call = msp_call_init(msp, cmd, rfn, priv, timeo);

    rc = expected(call) ? 0 : -1;
    if (rc)
        goto out;

    iov[cnt++] = (struct iovec) {
        .iov_base = &hdr,
        .iov_len = sizeof(hdr)
    };

    if (len) {
        rc = msp_msg_encode_req(&hdr, args);
        if (unexpected(rc))
            goto out;

        iov[cnt++] = (struct iovec) {
            .iov_base = args,
            .iov_len = len,
        };
    }

    cks = msp_msg_checksum(&hdr, args);

    iov[cnt++] = (struct iovec) {
        .iov_base = &cks,
        .iov_len = sizeof(cks)
    };

    rc = tty_sendv(msp->tty, iov, cnt);
out:
    if (rc) {
        int err = errno;

        if (call) {
            msp_call_exit(msp, cmd);
            call = NULL;
        }

        errno = err;
    }

    return rc;
}

void
msp_sync(struct msp *msp, msp_cmd_t cmd)
{
    do {
        struct msp_call *call;
        int rc;

        call = msp_call_get(msp, cmd);
        if (!call)
            break;

        if (!call->timer)
            break;

        rc = evtloop_iterate(msp->loop);
        if (rc)
            break;

    } while (1);
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
