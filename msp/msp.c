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
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
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

static struct msp_call *
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

struct msp_sync {
    int err;
    void *data;
    size_t len;
};

static void
msp_sync_retfn(int err,
               const struct msp_hdr *hdr,
               void *data, void *priv)
{
    struct msp_sync *sync = priv;

    assert(err != EAGAIN);
    sync->err = err;

    if (!sync->err) {
        sync->len = hdr->len;
        sync->data = data;
    }
}

static int
msp_req_send(struct msp *msp,
             msp_cmd_t cmd, void *args, size_t len)
{
    struct msp_sync *sync;
    int rc;

    sync = calloc(1, sizeof(*sync));

    rc = expected(sync) ? 0 : -1;
    if (rc)
        goto out;

    rc = msp_call(msp, cmd, args, len,
                  msp_sync_retfn, sync,
                  &MSP_TIMEOUT);

out:
    if (rc && sync)
        free(sync);

    return rc;
}

static int
__msp_rsp_recv(struct msp *msp,
               msp_cmd_t cmd, void *data, size_t *_len)
{
    struct msp_call *call;
    struct msp_sync *sync;
    int rc;

    sync = NULL;
    call = msp_call_get(msp, cmd);

    rc = expected(call) ? 0 : -1;
    if (rc)
        goto out;

    sync = call->priv;

    msp_sync(msp, cmd);

    call = NULL;

    rc = sync->err ? -1 : 0;
    if (rc) {
        errno = sync->err;
        goto out;
    }

    if (data && sync->data) {
        size_t len = min(*_len, sync->len);

        memcpy(data, sync->data, len);

        *_len = len;
    }

out:
    if (sync) {
        if (sync->data)
            free(sync->data);

        free(sync);
    }

    return rc;
}

static int
msp_rsp_recv(struct msp *msp,
             msp_cmd_t cmd, void *data, size_t len)
{
    size_t _len;
    int rc;

    _len = len;

    rc = __msp_rsp_recv(msp, cmd, data, &_len);
    if (rc)
        goto out;

    if (_len != len) {
        errno = EPROTO;
        goto out;
    }

out:
    return rc;
}

int
msp_ident(struct msp *msp, struct msp_ident *ident, size_t *_len)
{
    int rc;

    rc = msp_req_send(msp, MSP_IDENT, NULL, 0);
    if (rc)
        goto out;

    rc = __msp_rsp_recv(msp, MSP_IDENT, ident, _len);
out:
    return rc;
}

int
msp_raw_imu(struct msp *msp, struct msp_raw_imu *imu)
{
    int rc;

    rc = msp_req_send(msp, MSP_RAW_IMU, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_RAW_IMU, imu, sizeof(*imu));
out:
    return rc;
}

int
msp_altitude(struct msp *msp, struct msp_altitude *alt, size_t *_len)
{
    int rc;

    rc = msp_req_send(msp, MSP_ALTITUDE, NULL, 0);
    if (rc)
        goto out;

    rc = __msp_rsp_recv(msp, MSP_ALTITUDE, alt, _len);
out:
    return rc;
}

int
msp_attitude(struct msp *msp, struct msp_attitude *att, size_t *_len)
{
    int rc;

    rc = msp_req_send(msp, MSP_ATTITUDE, NULL, 0);
    if (rc)
        goto out;

    rc = __msp_rsp_recv(msp, MSP_ATTITUDE, att, _len);
out:
    return rc;
}

int
msp_mag_calibration(struct msp *msp)
{
    int rc;

    rc = msp_req_send(msp, MSP_MAG_CALIBRATION, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_MAG_CALIBRATION, NULL, 0);
out:
    return rc;
}

int
msp_acc_calibration(struct msp *msp)
{
    int rc;

    rc = msp_req_send(msp, MSP_ACC_CALIBRATION, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_ACC_CALIBRATION, NULL, 0);
out:
    return rc;
}

int
msp_eeprom_write(struct msp *msp)
{
    int rc;

    rc = msp_req_send(msp, MSP_EEPROM_WRITE, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_EEPROM_WRITE, NULL, 0);
out:
    return rc;
}

int
msp_reset_conf(struct msp *msp)
{
    int rc;

    rc = msp_req_send(msp, MSP_RESET_CONF, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_RESET_CONF, NULL, 0);
out:
    return rc;
}

int
msp_status(struct msp *msp, struct msp_status *st, size_t *_len)
{
    int rc;

    rc = msp_req_send(msp, MSP_STATUS, NULL, 0);
    if (rc)
        goto out;

    rc = __msp_rsp_recv(msp, MSP_STATUS, st, _len);
out:
    return rc;
}

int
msp_servo(struct msp *msp, struct msp_servo *servo, size_t *_len)
{
    int rc;

    rc = msp_req_send(msp, MSP_SERVO, NULL, 0);
    if (rc)
        goto out;

    rc = __msp_rsp_recv(msp, MSP_SERVO, servo, _len);
out:
    return rc;
}

int
msp_motor(struct msp *msp, struct msp_motor *motor, size_t *_len)
{
    int rc;

    rc = msp_req_send(msp, MSP_MOTOR, NULL, 0);
    if (rc)
        goto out;

    rc = __msp_rsp_recv(msp, MSP_MOTOR, motor, _len);
out:
    return rc;
}

int
msp_motor_pins(struct msp *msp, struct msp_motor_pins *pins, size_t *_len)
{
    int rc;

    rc = msp_req_send(msp, MSP_MOTOR_PINS, NULL, 0);
    if (rc)
        goto out;

    rc = __msp_rsp_recv(msp, MSP_MOTOR_PINS, pins, _len);
out:
    return rc;
}

int
msp_rc(struct msp *msp, struct msp_raw_rc *rrc, size_t *_len)
{
    int rc;

    rc = msp_req_send(msp, MSP_RC, NULL, 0);
    if (rc)
        goto out;

    rc = __msp_rsp_recv(msp, MSP_RC, rrc, _len);
out:
    return rc;
}

int
msp_set_raw_rc(struct msp *msp, struct msp_raw_rc *rrc)
{
    int rc;

    rc = msp_req_send(msp, MSP_SET_RAW_RC, rrc, sizeof(*rrc));
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_SET_RAW_RC, NULL, 0);
out:
    return rc;
}

int
msp_analog(struct msp *msp, struct msp_analog *analog, size_t *len)
{
    int rc;

    rc = msp_req_send(msp, MSP_ANALOG, NULL, 0);
    if (rc)
        goto out;

    rc = __msp_rsp_recv(msp, MSP_ANALOG, analog, len);
out:
    return rc;
}

int
msp_box(struct msp *msp, uint16_t *box, int *_cnt)
{
    int rc;
    size_t len;

    rc = msp_req_send(msp, MSP_BOX, NULL, 0);
    if (rc)
        goto out;

    len = *_cnt * sizeof(*box);

    rc = __msp_rsp_recv(msp, MSP_BOX, box, &len);
    if (rc)
        goto out;

    *_cnt = len / sizeof(*box);
out:
    return rc;
}

int
msp_boxnames(struct msp *msp, char *names, size_t *_len)
{
    int rc;

    rc = msp_req_send(msp, MSP_BOXNAMES, NULL, 0);
    if (rc)
        goto out;

    rc = __msp_rsp_recv(msp, MSP_BOXNAMES, names, _len);
out:
    return rc;
}

int
msp_boxids(struct msp *msp, uint8_t *boxids, size_t *_len)
{
    int rc;

    rc = msp_req_send(msp, MSP_BOXIDS, NULL, 0);
    if (rc)
        goto out;

    rc = __msp_rsp_recv(msp, MSP_BOXIDS, boxids, _len);
out:
    return rc;
}

int
msp_set_box(struct msp *msp, uint16_t *items, int cnt)
{
    int rc;

    rc = msp_req_send(msp, MSP_SET_BOX, items, cnt * sizeof(*items));
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_SET_BOX, NULL, 0);
out:
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
