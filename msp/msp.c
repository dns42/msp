#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <msp/msp-internal.h>

#include <msp/msg.h>
#include <msp/str.h>
#include <msp/defs.h>

#include <crt/defs.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <assert.h>

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
    if (!msp) {
        perror("calloc");
        goto out;
    }

    msp->tty = tty;
    msp->loop = loop;

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

static void
msp_tty_timeo(const struct timeval *timeo, void *priv)
{
    int *err = priv;
    *err = ETIMEDOUT;
}

static void
msp_tty_rxfn(struct tty *tty, int _err, void *priv)
{
    int *err = priv;
    assert(_err != EAGAIN);
    *err = _err;
}

static int
msp_tty_recv(struct msp *msp,
             void *buf, size_t len, struct timeval timeo)
{
    struct timer *timer;
    struct timeval now;
    struct iovec iov;
    int rc, err;

    rc = -1;
    err = 0;

    iov = (struct iovec) {
        .iov_base = buf,
        .iov_len = len
    };

    tty_setrxbuf(msp->tty, &iov, 1, msp_tty_rxfn, &err);
    gettimeofday(&now, NULL);
    timeradd(&now, &timeo, &timeo);

    timer = evtloop_create_timer(msp->loop, &timeo, msp_tty_timeo, &err);

    err = EAGAIN;
    do {
        rc = evtloop_iterate(msp->loop);
        if (rc < 0) {
            perror("evtloop_iterate");
            goto out;
        }
    } while (err == EAGAIN);

    rc = err ? -1 : 0;
    if (rc)
        errno = err;
out:
    if (timer) {
        err = errno;

        timer_destroy(timer);

        errno = err;
    }

    return rc;
}

int
msp_req_send(struct msp *msp,
             msp_cmd_t cmd, void *data, size_t len)
{
    struct msp_hdr hdr;
    struct iovec iov[3];
    uint8_t cks;
    int rc, cnt;

    cnt = 0;
    hdr = MSP_REQ_HDR(cmd, len);

    iov[cnt++] = (struct iovec) { &hdr, sizeof(hdr) };

    if (len) {
        rc = msp_msg_encode_req(&hdr, data);
        if (unexpected(rc))
            goto out;

        iov[cnt++] = (struct iovec) { data, len };
    }

    cks = msp_msg_checksum(&hdr, data);

    iov[cnt++] = (struct iovec) { &cks, sizeof(cks) };

    rc = tty_sendv(msp->tty, iov, cnt);
out:
    return rc;
}

int
__msp_rsp_recv(struct msp *msp,
               msp_cmd_t cmd, void *data, size_t *_len)
{
    struct msp_hdr hdr;
    size_t len;
    void *buf;
    uint8_t cks;
    int rc;

    len = *_len;
    buf = NULL;
    hdr.dsc = 0;

    rc = msp_tty_recv(msp, &hdr, sizeof(hdr), MSP_TIMEOUT);
    if (rc)
        goto out;

    rc = -1;

    if (hdr.tag[0] != '$' || hdr.tag[1] != 'M')
        goto out;

    if (hdr.dsc != '!' && hdr.dsc != '>')
        goto out;

    if (hdr.cmd != cmd)
        goto out;

    if (hdr.len) {
        rc = -1;

        buf = unlikely(hdr.len > len)
            ? malloc(hdr.len)
            : data;

        if (!expected(buf))
            goto out;

        rc = msp_tty_recv(msp, buf, hdr.len, MSP_TIMEOUT);
        if (rc)
            goto out;
    }

    rc = msp_tty_recv(msp, &cks, 1, MSP_TIMEOUT);
    if (rc)
        goto out;

    cks ^= msp_msg_checksum(&hdr, buf);

    rc = cks ? -1 : 0;
    if (rc) {
        errno = EBADMSG;
        goto out;
    }

    if (hdr.dsc == '!') {
        rc = -1;
        errno = ENOSYS;
        goto out;
    }

    rc = msp_msg_decode_rsp(&hdr, buf);
    if (rc)
        goto out;

    len = min(hdr.len, len);

    if (buf != data)
        memcpy(data, buf, len);

    *_len = len;
out:
    if (buf && buf != data)
        free(buf);

    if (rc && errno != ENOSYS && hdr.dsc) {
        fprintf(stderr,
                "%s rsp %c%c%c len %u/%zu cmd %u/%u cks %02x\n",
                msp_cmd_name(cmd),
                hdr.tag[0], hdr.tag[1], hdr.dsc,
                hdr.len, len, hdr.cmd, cmd, cks);
    }

    return rc;
}

int
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
