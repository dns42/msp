#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <msp/msp-internal.h>

#include <msp/msp.h>
#include <msp/defs.h>

#include <crt/defs.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <libgen.h>



static int
msp_tty_recv(struct msp *msp,
             void *buf, size_t len, struct timeval timeo)
{
    struct timerevt *timer;
    ssize_t n;
    int rc;

    rc = -1;

    tty_setrxbuf(msp->tty, buf, len);

    timer = evtloop_add_timer(msp->loop, timeo, NULL, NULL);

    do {
        rc = evtloop_iterate(msp->loop);
        if (rc < 0) {
            perror("evtloop_iterate");
            goto out;
        }

        if (!rc) {
            rc = -1;
            errno = ETIMEDOUT;
            goto out;
        }

        n = tty_rxcnt(msp->tty);
        if (n < 0) {
            perror("tty_rxcnt");
            goto out;
        }
    } while (n < len);

    rc = 0;
out:
    if (timer) {
        int err = errno;

        timerevt_destroy(timer);

        errno = err;
    }

    return rc;
}

static uint8_t
msp_msg_checksum(const struct msp_hdr *hdr, const void *data)
{
    uint8_t cks;

    cks  = hdr->cmd;
    cks ^= hdr->len;

    if (hdr->len) {
        const uint8_t *pos;
        msp_len_t len;


        len = hdr->len;
        pos = data;

        while (len--)
            cks ^= *pos++;
    }

    return cks;
}

static int
msp_req_send(struct msp *msp,
             msp_cmd_t cmd, const void *data, size_t len)
{
    struct msp_hdr hdr;
    struct iovec iov[3];
    uint8_t cks;
    int cnt;

    cnt = 0;
    hdr = MSP_REQ_HDR(cmd, len);

    iov[cnt++] = (struct iovec) { &hdr, sizeof(hdr) };

    if (len)
        iov[cnt++] = (struct iovec) { (void*)data, len };

    cks = msp_msg_checksum(&hdr, data);

    iov[cnt++] = (struct iovec) { &cks, sizeof(cks) };

    return tty_sendv(msp->tty, iov, cnt);
}

static int
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

    len = min(hdr.len, len);

    if (buf != data)
        memcpy(data, buf, len);

    *_len = len;
out:
    if (buf && buf != data)
        free(buf);

    if (rc && hdr.dsc) {
        fprintf(stderr,
                "rsp %c%c%c len %u/%zu cmd %u/%u cks %02x\n",
                hdr.tag[0], hdr.tag[1], hdr.dsc,
                hdr.len, len, hdr.cmd, cmd, cks);
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

static int
msp_ident(struct msp *msp, struct msp_ident *ident)
{
    int rc;

    rc = msp_req_send(msp, MSP_IDENT, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_IDENT, ident, sizeof(*ident));
out:
    return rc;
}

static const char *
msp_multitype_name(enum msp_multitype type)
{
    switch (type) {
    case MSP_MULTITYPE_TRI:
        return "tri";

    case MSP_MULTITYPE_QUADP:
        return "quadp";

    case MSP_MULTITYPE_QUADX:
        return "quadx";

    case MSP_MULTITYPE_BI:
        return "bi";

    case MSP_MULTITYPE_GIMBAL:
        return "gimbal";

    case MSP_MULTITYPE_Y6:
        return "y6";

    case MSP_MULTITYPE_HEX6:
        return "hex6";

    case MSP_MULTITYPE_FLYING_WING:
        return "flying-wing";

    case MSP_MULTITYPE_Y4:
        return "y4";

    case MSP_MULTITYPE_HEX6X:
        return "hex6x";

    case MSP_MULTITYPE_OCTOX8:
        return "octox8";

    case MSP_MULTITYPE_OCTOFLATP:
        return "octoflatp";

    case MSP_MULTITYPE_OCTOFLATX:
        return "octoflatx";

    case MSP_MULTITYPE_AIRPLANE:
        return "airplane";

    case MSP_MULTITYPE_HELI_120_CCPM:
        return "heli-120-ccpm";

    case MSP_MULTITYPE_HELI_90_DEG:
        return "heli-90-deg";

    case MSP_MULTITYPE_VTAIL4:
        return "vtail4";
    }

    return NULL;
}

static const char *
msp_ident_capability_name(int val)
{
    switch (val) {
    case MSP_IDENT_CAP_BIND:
        return "bind";
    case MSP_IDENT_CAP_DYNBAL:
        return "dynbal";
    case MSP_IDENT_CAP_FLAP:
        return "flap";
    }

    return NULL;
}

static int
msp_cmd_ident(struct msp *msp)
{
    struct msp_ident ident;
    int rc, bit;

    rc = msp_ident(msp, &ident);
    if (rc) {
        perror("msp_ident");
        goto out;
    }

    printf("ident.fwversion: %u (%u.%u)\n",
           ident.fwversion,
           ident.fwversion / 100, ident.fwversion % 100);

    printf("ident.multitype: %u (%s)\n",
           ident.multitype,
           msp_multitype_name(ident.multitype) ? : "?");

    printf("ident.mspversion: %u\n",
           ident.mspversion);

    printf("ident.capabilities: %#x%s",
           ident.capabilities, ident.capabilities ? " (" : "\n");
    for_each_bit(bit, &ident.capabilities)
        printf("%s%s",
               msp_ident_capability_name(bit) ? : "?",
               bit == ident.capabilities ? ")\n" : ", ");

out:
    return rc;
}

static int
msp_raw_imu(struct msp *msp, struct msp_raw_imu *imu)
{
    int rc, i;

    rc = msp_req_send(msp, MSP_RAW_IMU, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_RAW_IMU, imu, sizeof(*imu));
    if (rc)
        goto out;

    for (i = 0; i < array_size(imu->acc); i++)
        imu->acc[i] = avrtoh(imu->acc[i]);

    for (i = 0; i < array_size(imu->gyr); i++)
        imu->gyr[i] = avrtoh(imu->gyr[i]);

    for (i = 0; i < array_size(imu->mag); i++)
        imu->mag[i] = avrtoh(imu->mag[i]);
out:
    return rc;
}

static int
msp_cmd_raw_imu(struct msp *msp)
{
    struct msp_raw_imu imu;
    int rc, i;

    rc = msp_raw_imu(msp, &imu);
    if (rc) {
        perror("msp_raw_imu");
        goto out;
    }

    for (i = 0; i < array_size(imu.acc); i++)
        printf("raw-imu.acc[%d]: %d\n", i, imu.acc[i]);

    for (i = 0; i < array_size(imu.gyr); i++)
        printf("raw-imu.gyr[%d]: %d\n", i, imu.gyr[i]);

    for (i = 0; i < array_size(imu.mag); i++)
        printf("raw-imu.mag[%d]: %d\n", i, imu.mag[i]);

out:
    return rc;
}

static int
msp_altitude(struct msp *msp, int32_t *_alt)
{
    int32_t alt;
    int rc;

    rc = msp_req_send(msp, MSP_ALTITUDE, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_ALTITUDE, &alt, sizeof(alt));
    if (rc)
        goto out;

    *_alt = avrtoh(alt);
out:
    return rc;
}

static int
msp_cmd_altitude(struct msp *msp)
{
    int32_t alt;
    int rc;

    rc = msp_altitude(msp, &alt);
    if (rc) {
        perror("msp_altitude");
        goto out;
    }

    printf("altitude: %d\n", alt);
out:
    return rc;
}

static int
msp_attitude(struct msp *msp, struct msp_attitude *att)
{
    int rc;

    rc = msp_req_send(msp, MSP_ATTITUDE, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_ATTITUDE, att, sizeof(*att));
    if (rc)
        goto out;

    att->roll = avrtoh(att->roll);
    att->pitch = avrtoh(att->pitch);
    att->yaw = avrtoh(att->yaw);
    att->yawtf = avrtoh(att->yawtf);
out:
    return rc;
}

static int
msp_cmd_attitude(struct msp *msp)
{
    struct msp_attitude att;
    int rc;

    rc = msp_attitude(msp, &att);
    if (rc) {
        perror("msp_attitude");
        goto out;
    }

    printf("attitude.roll: %d\n", att.roll);

    printf("attitude.pitch: %d\n", att.pitch);

    printf("attitude.yaw: %d\n", att.yaw);

    printf("attitude.yawtf: %d\n", att.yawtf);
out:
    return rc;
}

static int
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

static int
msp_cmd_mag_calibration(struct msp *msp)
{
    int rc;

    rc = msp_mag_calibration(msp);
    if (rc) {
        perror("msp_mag_calibration");
        goto out;
    }

    printf("mag-calibration: ok\n");
out:
    return rc;
}

static int
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

static int
msp_cmd_acc_calibration(struct msp *msp)
{
    int rc;

    rc = msp_acc_calibration(msp);
    if (rc) {
        perror("msp_acc_calibration");
        goto out;
    }

    printf("acc-calibration: ok\n");
out:
    return rc;
}

static int
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

static int
msp_cmd_eeprom_write(struct msp *msp)
{
    int rc;

    rc = msp_eeprom_write(msp);
    if (rc) {
        perror("msp_eeprom_write");
        goto out;
    }

    printf("eeprom-write: ok\n");
out:
    return rc;
}

static int
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

static int
msp_cmd_reset_conf(struct msp *msp)
{
    int rc;

    rc = msp_reset_conf(msp);
    if (rc) {
        perror("msp_reset_conf");
        goto out;
    }

    printf("reset-conf: ok\n");
out:
    return rc;
}

static int
msp_status(struct msp *msp, struct msp_status *st, size_t *_len)
{
    size_t len;
    int rc;

    len = *_len;

    rc = msp_req_send(msp, MSP_STATUS, NULL, 0);
    if (rc)
        goto out;

    rc = __msp_rsp_recv(msp, MSP_STATUS, st, &len);
    if (rc)
        goto out;

    if (len >= msg_data_end(st, box)) {
        st->cycle_time = avrtoh(st->cycle_time);
        st->i2c_errcnt = avrtoh(st->i2c_errcnt);
        st->hwcaps = avrtoh(st->hwcaps);;
        st->box = avrtoh(st->box);
    }

    if (len >= msg_data_end(st, conf))
        st->conf = avrtoh(st->conf);

    *_len = len;
out:
    return rc;
}

static const char *
msp_status_hwcap_name(struct msp *msp, int val)
{
    switch (val) {
    case MSP_STATUS_HWCAP_ACC:
        return "acc";
    case MSP_STATUS_HWCAP_BARO:
        return "baro";
    case MSP_STATUS_HWCAP_MAG:
        return "mag";
    case MSP_STATUS_HWCAP_GPS:
        return "gps";
    case MSP_STATUS_HWCAP_SONAR:
        return "sonar";
    }

    return NULL;
}

static const char *
msp_status_box_name(struct msp *msp, int val)
{
    switch (val) {
    case MSP_STATUS_BOX_ACC:
        return "acc";
    case MSP_STATUS_BOX_BARO:
        return "baro";
    case MSP_STATUS_BOX_MAG:
        return "mag";
    case MSP_STATUS_BOX_CAMSTAB:
        return "camstab";
    case MSP_STATUS_BOX_CAMTRIG:
        return "camtrig";
    case MSP_STATUS_BOX_ARM:
        return "arm";
    case MSP_STATUS_BOX_GPSHOME:
        return "gpshome";
    case MSP_STATUS_BOX_GPSHOLD:
        return "gpshold";
    case MSP_STATUS_BOX_PASSTHRU:
        return "passthru";
    case MSP_STATUS_BOX_HEADFREE:
        return "headfree";
    case MSP_STATUS_BOX_BEEPERON:
        return "beeperon";
    case MSP_STATUS_BOX_LEDMAX:
        return "ledmax";
    case MSP_STATUS_BOX_LLIGHTS:
        return "llights";
    case MSP_STATUS_BOX_HEADADJ:
        return "headadj";
    }

    return NULL;
}

static int
msp_cmd_status(struct msp *msp)
{
    struct msp_status st;
    size_t len;
    int rc, bit;

    len = sizeof(st);

    rc = msp_status(msp, &st, &len);
    if (rc) {
        perror("msp_status");
        goto out;
    }

    if (len >= msg_data_end(&st, box)) {
        printf("status.cycle_time: %u\n", st.cycle_time);

        printf("status.i2c_errcnt: %u\n", st.i2c_errcnt);

        printf("status.hwcaps: %#x%s",
               st.hwcaps, st.hwcaps ? " (" : "\n");
        for_each_bit(bit, &st.hwcaps)
        printf("%s%s",
               msp_status_hwcap_name(msp, bit) ? : "?",
               bit == st.hwcaps ? ")\n" : ", ");

        printf("status.box: %#x%s",
               st.box, st.box ? " (" : "\n");
        for_each_bit(bit, &st.box)
            printf("%s%s",
                   msp_status_box_name(msp, bit) ? : "?",
                   bit == st.box ? ")\n" : ", ");
    }

    if (len >= msg_data_end(&st, conf))
        printf("status.conf: %#x\n", st.conf);

out:
    return rc;
}

static int
msp_servo(struct msp *msp, struct msp_servo *servo)
{
    int rc, i;

    rc = msp_req_send(msp, MSP_SERVO, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_SERVO, servo, sizeof(*servo));
    if (rc)
        goto out;

    for (i = 0; i < array_size(servo->ctl); i++)
        servo->ctl[i] = avrtoh(servo->ctl[i]);
out:
    return rc;
}

static int
msp_cmd_servo(struct msp *msp)
{
    struct msp_servo servo;
    int rc, i;

    rc = msp_servo(msp, &servo);
    if (rc) {
        perror("msp_servo");
        goto out;
    }

    for (i = 0; i < array_size(servo.ctl); i++)
        printf("servo.ctl[%d]: %d\n", i, servo.ctl[i]);
out:
    return rc;
}

static int
msp_motor(struct msp *msp, struct msp_motor *motor)
{
    int rc, i;

    rc = msp_req_send(msp, MSP_MOTOR, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_MOTOR, motor, sizeof(*motor));
    if (rc)
        goto out;

    for (i = 0; i < array_size(motor->ctl); i++)
        motor->ctl[i] = avrtoh(motor->ctl[i]);
out:
    return rc;
}

static int
msp_cmd_motor(struct msp *msp)
{
    struct msp_motor motor;
    int rc, i;

    rc = msp_motor(msp, &motor);
    if (rc) {
        perror("msp_motor");
        goto out;
    }

    for (i = 0; i < array_size(motor.ctl); i++)
        printf("motor.ctl[%d]: %d\n", i, motor.ctl[i]);
out:
    return rc;
}

static int
msp_motor_pins(struct msp *msp, struct msp_motor_pins *pins)
{
    int rc;

    rc = msp_req_send(msp, MSP_MOTOR_PINS, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_MOTOR_PINS, pins, sizeof(*pins));
out:
    return rc;
}

static int
msp_cmd_motor_pins(struct msp *msp)
{
    struct msp_motor_pins pins;
    int rc, i;

    rc = msp_motor_pins(msp, &pins);
    if (rc) {
        perror("msp_motor_pins");
        goto out;
    }

    for (i = 0; i < array_size(pins.pin); i++)
        printf("motor-pins.pin[%d]: %u\n", i, pins.pin[i]);
out:
    return rc;
}

static int
msp_rc(struct msp *msp, struct msp_raw_rc *rrc)
{
    int rc, i;

    rc = msp_req_send(msp, MSP_RC, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_RC, rrc, sizeof(*rrc));
    if (rc)
        goto out;

    for (i = 0; i < array_size(rrc->chn); i++)
        rrc->chn[i] = avrtoh(rrc->chn[i]);
out:
    return rc;
}

static const char *
msp_rc_chan_name(enum msp_rc_chn n)
{
    switch (n) {
    case MSP_CHN_ROLL:
        return "roll";
    case MSP_CHN_PITCH:
        return "pitch";
    case MSP_CHN_YAW:
        return "yaw";
    case MSP_CHN_THROTTLE:
        return "throttle";
    case MSP_CHN_AUX1:
        return "aux1";
    case MSP_CHN_AUX2:
        return "aux2";
    case MSP_CHN_AUX3:
        return "aux3";
    case MSP_CHN_AUX4:
        return "aux4";
    }

    return NULL;
}

static int
msp_cmd_rc(struct msp *msp)
{
    struct msp_raw_rc rrc;
    int rc, i;

    rc = msp_rc(msp, &rrc);
    if (rc) {
        perror("msp_rrc");
        goto out;
    }

    for (i = 0; i < array_size(rrc.chn); i++)
        printf("rc.chn[%s]: %d\n",
               msp_rc_chan_name(i) ? : "?",
               rrc.chn[i]);
out:
    return rc;
}

static int
msp_set_raw_rc(struct msp *msp, const struct msp_raw_rc *rrc)
{
    int rc;

    rc = msp_req_send(msp, MSP_SET_RAW_RC, rrc, sizeof(*rrc));
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_SET_RAW_RC, NULL, 0);
out:
    return rc;
}

static int
msp_cmd_set_raw_rc(struct msp *msp, int argc, char **argv)
{
    struct msp_raw_rc rrc;
    int rc;

    memset(&rrc, 0, sizeof(rrc));

    rc = -1;

    while (++optind < argc) {
        const char *arg;
        char name[9];
        int16_t val;
        int chn, n;

        arg = argv[optind];
        chn = -1;

        if (!strcmp(arg, "--"))
            break;

        n = sscanf(arg, "%8[^:]:%hu", name, &val);
        if (n != 2)
            goto err;

        if (isdigit(name[0])) {
            char *end;

            chn = strtol(name, &end, 0);
            if (*end != 0)
                chn = -1;

        } else {
            switch (tolower(name[0])) {
            case 'r':
                if (!strncasecmp(name, "rol", 3)) {
                    chn = MSP_CHN_ROLL;
                    break;
                }
                break;
            case 'p':
                if (!strncasecmp(name, "pit", 3)) {
                    chn = MSP_CHN_PITCH;
                    break;
                }
                break;
            case 'y':
                if (!strncasecmp(name, "yaw", 3)) {
                    chn = MSP_CHN_YAW;
                    break;
                }
                break;
            case 't':
                if (!strncasecmp(name, "thr", 3)) {
                    chn = MSP_CHN_THROTTLE;
                    break;
                }
                break;
            case 'a':
                if (!strncasecmp(name, "aux", 3)) {
                    char *end;
                    chn = strtoul(&name[3], &end, 10);
                    if (*end != 0)
                        goto err;
                    chn += MSP_CHN_AUX1 - 1;
                    break;
                }
                break;
            }
        }

    err:
        if (chn < 0 || chn >= array_size(rrc.chn)) {
            fprintf(stderr,
                    "invalid rc item '%s', "
                    "must be '<0-8>:<1000-2000>'\n", arg);
            errno = EINVAL;
            goto out;
        }

        rrc.chn[chn] = htoavr(val);
    }

    rc = msp_set_raw_rc(msp, &rrc);
    if (rc) {
        perror("msp_set_raw_rc");
        goto out;
    }

    printf("set-raw-rc: ok\n");
out:
    return rc;
}

static int
msp_bat(struct msp *msp, struct msp_bat *bat)
{
    int rc;

    rc = msp_req_send(msp, MSP_BAT, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(msp, MSP_BAT, bat, sizeof(*bat));
out:
    return rc;
}

static int
msp_cmd_bat(struct msp *msp)
{
    struct msp_bat bat;
    int rc;

    rc = msp_bat(msp, &bat);
    if (rc) {
        perror("msp_bat");
        goto out;
    }

    printf("bat.vbat: %u\n", bat.vbat);
    printf("bat.powermetersum: %u\n", bat.powermetersum);
out:
    return rc;
}

static void
msp_usage(FILE *s, const char *prog)
{
    fprintf(s,
            "Usage:\n"
            "  %s [ -T <tty> ] [ -b <baud> ] [ -V ] [ -h ]"
            " command [ args .. ] -- ...\n"
            "\n", prog);
    fprintf(s,
            "Commands:\n"
            "  acc-calibration -- calibrate accelerometer\n"
            "  altitude -- read altitude\n"
            "  attitude -- read attitude\n"
            "  bat -- read battery status\n"
            "  eeprom-write -- write current params to eeprom\n"
            "  ident -- identify controller firmware\n"
            "  mag-calibration -- calibrate magnetometer\n"
            "  motor -- read motor control\n"
            "  motor-pins -- read motor pin numbers\n"
            "  raw-imu -- read raw IMU data\n"
            "  rc -- read RC channels\n"
            "  reset-conf -- reset params to firmware defaults\n"
            "  servo -- read servo control\n"
            "  set-raw-rc -- set RC channels\n"
            "  status -- read controller status\n"
            "\n");
}

int
main(int argc, char **argv)
{
    const char *ttypath;
    struct msp *msp;
    speed_t speed;
    int rc, fd;

    fd = -1;
    rc = -1;
    ttypath = "/dev/ttyUSB0";
    speed = B115200;
    msp = NULL;

    do {
        int c;

        c = getopt(argc, argv, "+T:b:Vh");
        if (c < 0)
            break;

        switch (c) {
        case 'T':
            ttypath = optarg;
            break;

        case 'b':
            speed = tty_speed(atoi(optarg));
            if (speed == -1)
                goto usage;
            break;

        case 'V':
            printf("MultiWii Serial Protocol v%s, "
                   "MSPv%d\n", PACKAGE_VERSION, MSP_VERSION);
            rc = 0;
            goto out;

        case 'h':
            rc = 0;
        default:
            goto usage;
        }
    } while (1);

    if (optind == argc)
        goto usage;

    msp = calloc(1, sizeof(*msp));
    if (!msp) {
        perror("calloc");
        goto out;
    }

    msp->tty = tty_open(ttypath, speed);
    if (!msp->tty) {
        perror(ttypath);
        goto out;
    }

    msp->loop = evtloop_create();
    if (!msp->loop) {
        perror("evtloop_create");
        goto out;
    }

    rc = tty_plug(msp->tty, msp->loop);
    if (rc) {
        perror("tty_register_events");
        goto out;
    }

    for (; optind < argc; optind++) {
        const char *cmd;

        cmd = argv[optind];

        switch (cmd[0]) {
        case 'a':
            if (!strcmp(cmd, "acc-calibration")) {
                rc = msp_cmd_acc_calibration(msp);
                break;
            }
            if (!strcmp(cmd, "altitude")) {
                rc = msp_cmd_altitude(msp);
                break;
            }
            if (!strcmp(cmd, "attitude")) {
                rc = msp_cmd_attitude(msp);
                break;
            }
            goto invalid;
        case 'b':
            if (!strcmp(cmd, "bat")) {
                rc = msp_cmd_bat(msp);
                break;
            }
            goto invalid;
        case 'e':
            if (!strcmp(cmd, "eeprom-write")) {
                rc = msp_cmd_eeprom_write(msp);
                break;
            }
            goto invalid;
        case 'i':
            if (!strcmp(cmd, "ident")) {
                rc = msp_cmd_ident(msp);
                break;
            }
            goto invalid;
        case 'm':
            if (!strcmp(cmd, "mag-calibration")) {
                rc = msp_cmd_mag_calibration(msp);
                break;
            }
            if (!strcmp(cmd, "motor")) {
                rc = msp_cmd_motor(msp);
                break;
            }
            if (!strcmp(cmd, "motor-pins")) {
                rc = msp_cmd_motor_pins(msp);
                break;
            }
            goto invalid;
        case 'r':
            if (!strcmp(cmd, "raw-imu")) {
                rc = msp_cmd_raw_imu(msp);
                break;
            }
            if (!strcmp(cmd, "rc")) {
                rc = msp_cmd_rc(msp);
                break;
            }
            if (!strcmp(cmd, "reset-conf")) {
                rc = msp_cmd_reset_conf(msp);
                break;
            }
            goto invalid;
        case 's':
            if (!strcmp(cmd, "servo")) {
                rc = msp_cmd_servo(msp);
                break;
            }
            if (!strcmp(cmd, "set-raw-rc")) {
                rc = msp_cmd_set_raw_rc(msp, argc, argv);
                break;
            }
            if (!strcmp(cmd, "status")) {
                rc = msp_cmd_status(msp);
                break;
            }
            goto invalid;
        case '-':
            if (!strcmp(cmd, "--"))
                break;
        default:
        invalid:
            rc = -1;
            goto usage;
        }

        if (rc < 0)
            break;
    }

out:
    if (msp) {
        if (msp->tty)
            tty_close(msp->tty);

        if (msp->loop)
            evtloop_destroy(msp->loop);

        free(msp);
    }


    if (fd >= 0)
        close(fd);

    return rc ? 1 : 0;

usage:
    msp_usage(rc ? stderr : stdout, basename(argv[0]));
    goto out;
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
