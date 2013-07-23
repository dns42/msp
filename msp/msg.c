#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <msp/msg-internal.h>
#include <msp/defs.h>
#include <crt/defs.h>

static int
msp_ident_rsp_dec(const struct msp_hdr *hdr, void *data)
{
    struct msp_ident *ident = data;

    ident->capabilities = avrtoh(ident->capabilities);

    return 0;
}

static int
msp_status_rsp_dec(const struct msp_hdr *hdr, void *data)
{
    struct msp_status *st = data;

    if (hdr->len >= msg_data_end(st, box)) {
        st->cycle_time = avrtoh(st->cycle_time);
        st->i2c_errcnt = avrtoh(st->i2c_errcnt);
        st->hwcaps = avrtoh(st->hwcaps);;
        st->box = avrtoh(st->box);
    }

    if (hdr->len >= msg_data_end(st, conf))
        st->conf = avrtoh(st->conf);

    return 0;
}

static int
msp_raw_imu_rsp_dec(const struct msp_hdr *hdr, void *data)
{
    struct msp_raw_imu *imu = data;
    int i;

    for (i = 0; i < array_size(imu->acc); i++)
        imu->acc[i] = avrtoh(imu->acc[i]);

    for (i = 0; i < array_size(imu->gyr); i++)
        imu->gyr[i] = avrtoh(imu->gyr[i]);

    for (i = 0; i < array_size(imu->mag); i++)
        imu->mag[i] = avrtoh(imu->mag[i]);

    return 0;
}

static int
msp_servo_rsp_dec(const struct msp_hdr *hdr, void *data)
{
    struct msp_servo *servo = data;
    int i;

    for (i = 0; i < array_size(servo->ctl); i++)
        servo->ctl[i] = avrtoh(servo->ctl[i]);

    return 0;
}

static int
msp_motor_rsp_dec(const struct msp_hdr *hdr, void *data)
{
    struct msp_motor *motor = data;
    int i;

    for (i = 0; i < array_size(motor->ctl); i++)
        motor->ctl[i] = avrtoh(motor->ctl[i]);

    return 0;
}

static int
msp_rc_rsp_dec(const struct msp_hdr *hdr, void *data)
{
    struct msp_raw_rc *rrc = data;
    int i;

    for (i = 0; i < array_size(rrc->chn); i++)
        rrc->chn[i] = avrtoh(rrc->chn[i]);

    return 0;
}

static int
msp_attitude_rsp_dec(const struct msp_hdr *hdr, void *data)
{
    struct msp_attitude *att = data;

    att->roll = avrtoh(att->roll);
    att->pitch = avrtoh(att->pitch);
    att->yaw = avrtoh(att->yaw);
    att->yawtf = avrtoh(att->yawtf);

    return 0;
}

static int
msp_altitude_rsp_dec(const struct msp_hdr *hdr, void *data)
{
    struct msp_altitude *alt = data;

    alt->altitude = avrtoh(alt->altitude);
    alt->variometer = avrtoh(alt->variometer);

    return 0;
}

static int
msp_analog_rsp_dec(const struct msp_hdr *hdr, void *data)
{
    struct msp_analog *analog = data;

    analog->powermetersum = avrtoh(analog->powermetersum);

    if (hdr->len >= msg_data_end(analog, rssi))
        analog->rssi = avrtoh(analog->rssi);

    return 0;
}

static int
msp_box_rsp_dec(const struct msp_hdr *hdr, void *data)
{
    uint16_t *box = data;
    int rc, cnt, i;

    rc = -1;

    if (unexpected(hdr->len % sizeof(*box))) {
        errno = EPROTO;
        goto out;
    }

    cnt = hdr->len / sizeof(*box);

    for (i = 0; i < cnt; i++)
        box[i] = avrtoh(box[i]);

    rc = 0;
out:
    return rc;
}

static int
msp_set_raw_rc_req_enc(const struct msp_hdr *hdr, void *data)
{
    struct msp_raw_rc *rrc = data;
    int i;

    for (i = 0; i < array_size(rrc->chn); i++)
        rrc->chn[i] = htoavr(rrc->chn[i]);

    return 0;
}

static int
msp_set_box_req_enc(const struct msp_hdr *hdr, void *data)
{
    uint16_t *items = data;
    int rc, cnt, i;

    rc = -1;

    if (unexpected(hdr->len % sizeof(*items))) {
        errno = EPROTO;
        goto out;
    }

    cnt = hdr->len / sizeof(*items);

    for (i = 0; i < cnt; i++)
        items[i] = htoavr(items[i]);

    rc = 0;
out:
    return rc;
}

const struct msp_msg_info msp_msg_infos[MSP_CMD_MAX] = {
    [MSP_IDENT] = {
        .tag = "MSP_IDENT",
        .sup = 1,
        .min = sizeof(struct msp_ident),
        .max = sizeof(struct msp_ident),
        .req = NULL,
        .rsp = msp_ident_rsp_dec,
    },
    [MSP_STATUS] = {
        .tag = "MSP_STATUS",
        .sup = 1,
        .min = msg_type_end(struct msp_status, box),
        .max = sizeof(struct msp_status),
        .req = NULL,
        .rsp = msp_status_rsp_dec,
    },
    [MSP_RAW_IMU] = {
        .tag = "MSP_RAW_IMU",
        .sup = 1,
        .min = sizeof(struct msp_raw_imu),
        .max = sizeof(struct msp_raw_imu),
        .req = NULL,
        .rsp = msp_raw_imu_rsp_dec,
    },
    [MSP_SERVO] = {
        .tag = "MSP_SERVO",
        .sup = 1,
        .min = sizeof(struct msp_servo),
        .max = sizeof(struct msp_servo),
        .req = NULL,
        .rsp = msp_servo_rsp_dec,
    },
    [MSP_MOTOR] = {
        .tag = "MSP_MOTOR",
        .sup = 1,
        .min = sizeof(struct msp_motor),
        .max = sizeof(struct msp_motor),
        .req = NULL,
        .rsp = msp_motor_rsp_dec,
    },
    [MSP_RC] = {
        .tag = "MSP_RC",
        .sup = 1,
        .min = sizeof(struct msp_raw_rc),
        .max = sizeof(struct msp_raw_rc),
        .req = NULL,
        .rsp = msp_rc_rsp_dec,
    },
    [MSP_ATTITUDE] = {
        .tag = "MSP_ATTITUDE",
        .sup = 1,
        .min = sizeof(struct msp_attitude),
        .max = sizeof(struct msp_attitude),
        .req = NULL,
        .rsp = msp_attitude_rsp_dec,
    },
    [MSP_ALTITUDE] = {
        .tag = "MSP_ALTITUDE",
        .sup = 1,
        .min = msg_type_end(struct msp_altitude, altitude),
        .max = msg_type_end(struct msp_altitude, variometer),
        .req = NULL,
        .rsp = msp_altitude_rsp_dec,
    },
    [MSP_ANALOG] = {
        .tag = "MSP_ANALOG",
        .sup = 1,
        .min = msg_type_end(struct msp_analog, powermetersum),
        .max = msg_type_end(struct msp_analog, rssi),
        .req = NULL,
        .rsp = msp_analog_rsp_dec,
    },
    [MSP_BOX] = {
        .tag = "MSP_BOX",
        .sup = 1,
        .min = 0,
        .max = MSP_LEN_MAX,
        .req = NULL,
        .rsp = msp_box_rsp_dec,
    },
    [MSP_MOTOR_PINS] = {
        .tag = "MSP_PINS",
        .sup = 1,
        .min = sizeof(struct msp_motor_pins),
        .max = sizeof(struct msp_motor_pins),
        .req = NULL,
        .rsp = NULL,
    },
    [MSP_BOXNAMES] = {
        .tag = "MSP_BOXNAMES",
        .sup = 1,
        .min = 0,
        .max = MSP_LEN_MAX,
        .req = NULL,
        .rsp = NULL,
    },
    [MSP_BOXIDS] = {
        .tag = "MSP_BOXIDS",
        .sup = 1,
        .min = 0,
        .max = MSP_LEN_MAX,
        .req = NULL,
        .rsp = NULL,
    },
    [MSP_SET_RAW_RC] = {
        .tag = "MSP_SET_RAW_RC",
        .sup = 1,
        .min = 0,
        .max = 0,
        .req = msp_set_raw_rc_req_enc,
        .rsp = NULL,
    },
    [MSP_SET_BOX] = {
        .tag = "MSP_SET_BOX",
        .sup = 1,
        .min = 0,
        .max = MSP_LEN_MAX,
        .req = msp_set_box_req_enc,
        .rsp = NULL,
    },
    [MSP_ACC_CALIBRATION] = {
        .tag = "MSP_ACC_CALIBRATION",
        .sup = 1,
        .min = 0,
        .max = 0,
        .req = NULL,
        .rsp = NULL,
    },
    [MSP_MAG_CALIBRATION] = {
        .tag = "MSP_MAG_CALIBRATION",
        .sup = 1,
        .min = 0,
        .max = 0,
        .req = NULL,
        .rsp = NULL,
    },
    [MSP_RESET_CONF] = {
        .tag = "MSP_RESET_CONF",
        .sup = 1,
        .min = 0,
        .max = 0,
        .req = NULL,
        .rsp = NULL,
    },
    [MSP_EEPROM_WRITE] = {
        .tag = "MSP_EEPROM_WRITE",
        .sup = 1,
        .min = 0,
        .max = 0,
        .req = NULL,
        .rsp = NULL,
    },
};

int
msp_msg_encode_req(const struct msp_hdr *hdr, void *data)
{
    const struct msp_msg_info *info;
    int rc;

    rc = -1;
    info = &msp_msg_infos[hdr->cmd];

    if (!info->sup) {
        errno = EINVAL;
        goto out;
    }

    if (info->req)
        rc = info->req(hdr, data);
    else
        rc = 0;
out:
    return rc;
}

int
msp_msg_decode_rsp(const struct msp_hdr *hdr, void *data)
{
    const struct msp_msg_info *info;
    int rc;

    rc = -1;
    info = &msp_msg_infos[hdr->cmd];

    if (!info->sup) {
        errno = EPROTO;
        goto out;
    }

    if (unexpected(hdr->len < info->min) ||
        unexpected(hdr->len > info->max)) {
        errno = EILSEQ;
        goto out;
    }

    if (info->rsp)
        rc = info->rsp(hdr, data);
    else
        rc = 0;
out:
    return rc;
}

uint8_t
msp_msg_checksum(const struct msp_hdr *hdr, const void *_data)
{
    const uint8_t *pos;
    msp_len_t len;
    uint8_t cks;

    pos = _data;

    len = hdr->len;
    cks = hdr->cmd ^ len;

    while (len--)
        cks ^= *pos++;

    return cks;
}

const char *
msp_cmd_name(msp_cmd_t cmd)
{
    return msp_msg_infos[cmd].tag;
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
