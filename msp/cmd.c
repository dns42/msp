#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <msp/cmd.h>
#include <msp/msp-internal.h>

#include <crt/defs.h>
#include <crt/log.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

struct msp_cmd_sync {
    int err;
    void *data;
    size_t len;
};

static void
msp_sync_retfn(int err,
               const struct msp_hdr *hdr,
               void *data, void *priv)
{
    struct msp_cmd_sync *sync = priv;

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
    struct msp_cmd_sync *sync;
    int rc;

    sync = calloc(1, sizeof(*sync));

    rc = expected(sync) ? 0 : -1;
    if (rc)
        goto out;

    rc = msp_call(msp, cmd, args, len,
                  msp_sync_retfn, sync,
                  &MSP_CMD_TIMEOUT);

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
    struct msp_cmd_sync *sync;
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
