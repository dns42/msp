#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <msp/msp.h>
#include <msp/defs.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <libgen.h>

#define MSP_TIMEOUT (struct timeval) { 1, 0 }

static int
msp_tty_open(const char *path, speed_t speed)
{
    int rc, fd;
    struct termios tio;

    rc = -1;

    fd = open(path, O_RDWR|O_NOCTTY|O_NDELAY);
    if (fd < 0)
        goto out;

    memset(&tio, 0, sizeof(tio));

    tio.c_iflag = 0;
    tio.c_cflag = CREAD|CLOCAL|CS8;

    tio.c_cc[VMIN] = 1;
    tio.c_cc[VTIME] = 10;

    cfsetospeed(&tio, speed);
    cfsetispeed(&tio, speed);

    rc = tcsetattr(fd, TCSANOW, &tio);
    if (rc)
        goto out;

    rc = fd;
out:
    if (rc < 0) {
        if (fd >= 0)
            close(fd);
    }

    return rc;
}

static int
msp_tty_send(int fd, const void *buf, size_t len)
{
    int rc;
    ssize_t n;

    rc = -1;

    do {
        n = write(fd, buf, len);
        if (n < 0)
            goto out;

        buf = (char *)buf + n;
        len -= n;
    } while (len > 0);

    rc = tcdrain(fd);
out:
    return rc;
}

static int
msp_tty_recv(int fd, void *buf, size_t len, struct timeval timeo)
{
    int rc, nfds;
    fd_set rfds;
    ssize_t n;

    rc = -1;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    do {
        nfds = select(fd + 1, &rfds, NULL, NULL, &timeo);
        if (nfds < 0)
            goto out;
        if (!nfds) {
            errno = ETIMEDOUT;
            goto out;
        }

        n = read(fd, buf, len);
        if (n < 0)
            goto out;

        buf = (char *)buf + n;
        len -= n;
    } while (len > 0);

    rc = 0;
out:
    return rc;
}

static speed_t
msp_tty_speed(int arg)
{
    switch (arg) {
    case 115200:
        return B115200;
    case 57600:
        return B57600;
    case 38400:
        return B38400;
    case 19200:
        return B19200;
    case 9600:
        return B9600;
    }

    return -1;
}

static uint8_t
msp_msg_checksum(uint8_t cks, const void *data, char len)
{
    const uint8_t *pos;

    pos = data;

    while (len--)
        cks ^= *pos++;

    return cks;
}

static int
msp_req_send(int fd, msp_cmd_t cmd, const void *data, msp_len_t len)
{
    struct msp_hdr hdr;
    int rc;

    hdr.tag[0] = '$';
    hdr.tag[1] = 'M';
    hdr.dsc = '<';
    hdr.len = len;
    hdr.cmd = cmd;

    rc = msp_tty_send(fd, &hdr, sizeof(hdr));

    if (!rc && len)
        rc = msp_tty_send(fd, data, len);

    if (!rc) {
        uint8_t cks;

        cks = msp_msg_checksum(0, &cmd, sizeof(cmd));
        if (len)
            cks = msp_msg_checksum(cks, data, len);

        rc = msp_tty_send(fd, &cks, 1);
    }

    return rc;
}

static int
msp_rsp_recv(int fd, msp_cmd_t cmd, void *data, msp_len_t len)
{
    struct msp_hdr hdr;
    uint8_t cks;
    int rc;

    hdr.dsc = 0;

    rc = msp_tty_recv(fd, &hdr, sizeof(hdr), MSP_TIMEOUT);
    if (rc)
        goto out;

    rc = -1;

    if (hdr.tag[0] != '$' || hdr.tag[1] != 'M')
        goto out;

    if (hdr.dsc == '!')
        goto out;

    if (hdr.dsc != '>')
        goto out;

    if (hdr.len != len)
        goto out;

    if (hdr.cmd != cmd)
        goto out;

    rc = len
        ? msp_tty_recv(fd, data, len, MSP_TIMEOUT)
        : 0;

    if (!rc) {
        rc = msp_tty_recv(fd, &cks, 1, MSP_TIMEOUT);

        cks = msp_msg_checksum(cks, &len, 1);
        cks = msp_msg_checksum(cks, &cmd, 1);
        if (len)
            cks = msp_msg_checksum(cks, data, len);

        rc = cks ? -1 : 0;
    }
out:
    if (rc && hdr.dsc)
        fprintf(stderr,
                "rsp %c%c%c len %u/%d cmd %u/%u cks %02x\n",
                hdr.tag[0], hdr.tag[1], hdr.dsc,
                hdr.len, len, hdr.cmd, cmd, cks);

    return rc;
}

static int
msp_ident(int fd, struct msp_ident *ident)
{
    int rc;

    rc = msp_req_send(fd, MSP_IDENT, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_IDENT, ident, sizeof(*ident));
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

    return "?";
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

    return "?";
}

static int
msp_cmd_ident(int fd)
{
    struct msp_ident ident;
    int rc, bit;

    rc = msp_ident(fd, &ident);
    if (rc) {
        perror("msp_ident");
        goto out;
    }

    printf("ident.fwversion: %u (%u.%u)\n",
           ident.fwversion,
           ident.fwversion / 100, ident.fwversion % 100);

    printf("ident.multitype: %u (%s)\n",
           ident.multitype,
           msp_multitype_name(ident.multitype));

    printf("ident.mspversion: %u\n",
           ident.mspversion);

    printf("ident.capabilities: %#x%s",
           ident.capabilities, ident.capabilities ? " (" : "\n");
    for_each_bit(bit, &ident.capabilities)
        printf("%s%s",
               msp_ident_capability_name(bit),
               bit == ident.capabilities ? ")\n" : ", ");

out:
    return rc;
}

static int
msp_raw_imu(int fd, struct msp_raw_imu *imu)
{
    int rc, i;

    rc = msp_req_send(fd, MSP_RAW_IMU, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_RAW_IMU, imu, sizeof(*imu));
    if (rc)
        goto out;

    for (i = 0; i < array_size(imu->acc); i++)
        imu->acc[i] = avrtoh(imu->acc[i]);

    for (i = 0; i < array_size(imu->gyr); i++)
        imu->gyr[i] = avrtoh(imu->gyr[i]);

    for (i = 0; i < array_size(imu->adc); i++)
        imu->adc[i] = avrtoh(imu->adc[i]);
out:
    return rc;
}

static int
msp_cmd_raw_imu(int fd)
{
    struct msp_raw_imu imu;
    int rc, i;

    rc = msp_raw_imu(fd, &imu);
    if (rc) {
        perror("msp_raw_imu");
        goto out;
    }

    for (i = 0; i < array_size(imu.acc); i++)
        printf("raw-imu.acc[%d]: %d\n", i, imu.acc[i]);

    for (i = 0; i < array_size(imu.gyr); i++)
        printf("raw-imu.gyr[%d]: %d\n", i, imu.gyr[i]);

    for (i = 0; i < array_size(imu.adc); i++)
        printf("raw-imu.adc[%d]: %d\n", i, imu.adc[i]);

out:
    return rc;
}

static int
msp_altitude(int fd, int32_t *_alt)
{
    int32_t alt;
    int rc;

    rc = msp_req_send(fd, MSP_ALTITUDE, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_ALTITUDE, &alt, sizeof(alt));
    if (rc)
        goto out;

    *_alt = avrtoh(alt);
out:
    return rc;
}

static int
msp_cmd_altitude(int fd)
{
    int32_t alt;
    int rc;

    rc = msp_altitude(fd, &alt);
    if (rc) {
        perror("msp_altitude");
        goto out;
    }

    printf("altitude: %d\n", alt);
out:
    return rc;
}

static int
msp_attitude(int fd, struct msp_attitude *att)
{
    int rc;

    rc = msp_req_send(fd, MSP_ATTITUDE, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_ATTITUDE, att, sizeof(*att));
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
msp_cmd_attitude(int fd)
{
    struct msp_attitude att;
    int rc;

    rc = msp_attitude(fd, &att);
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
msp_mag_calibration(int fd)
{
    int rc;

    rc = msp_req_send(fd, MSP_MAG_CALIBRATION, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_MAG_CALIBRATION, NULL, 0);
out:
    return rc;
}

static int
msp_cmd_mag_calibration(int fd)
{
    int rc;

    rc = msp_mag_calibration(fd);
    if (rc) {
        perror("msp_mag_calibration");
        goto out;
    }

    printf("mag-calibration: ok\n");
out:
    return rc;
}

static int
msp_acc_calibration(int fd)
{
    int rc;

    rc = msp_req_send(fd, MSP_ACC_CALIBRATION, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_ACC_CALIBRATION, NULL, 0);
out:
    return rc;
}

static int
msp_cmd_acc_calibration(int fd)
{
    int rc;

    rc = msp_acc_calibration(fd);
    if (rc) {
        perror("msp_acc_calibration");
        goto out;
    }

    printf("acc-calibration: ok\n");
out:
    return rc;
}

static int
msp_eeprom_write(int fd)
{
    int rc;

    rc = msp_req_send(fd, MSP_EEPROM_WRITE, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_EEPROM_WRITE, NULL, 0);
out:
    return rc;
}

static int
msp_cmd_eeprom_write(int fd)
{
    int rc;

    rc = msp_eeprom_write(fd);
    if (rc) {
        perror("msp_eeprom_write");
        goto out;
    }

    printf("eeprom-write: ok\n");
out:
    return rc;
}

static int
msp_reset_conf(int fd)
{
    int rc;

    rc = msp_req_send(fd, MSP_RESET_CONF, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_RESET_CONF, NULL, 0);
out:
    return rc;
}

static int
msp_cmd_reset_conf(int fd)
{
    int rc;

    rc = msp_reset_conf(fd);
    if (rc) {
        perror("msp_reset_conf");
        goto out;
    }

    printf("reset-conf: ok\n");
out:
    return rc;
}

static int
msp_status(int fd, struct msp_status *st)
{
    int rc;

    rc = msp_req_send(fd, MSP_STATUS, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_STATUS, st, sizeof(*st));
    if (rc)
        goto out;

    st->cycle_time = avrtoh(st->cycle_time);
    st->i2c_errcnt = avrtoh(st->i2c_errcnt);
    st->hwcaps = avrtoh(st->hwcaps);;
    st->box = avrtoh(st->box);
out:
    return rc;
}

static const char *
msp_status_hwcap_name(int val)
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

    return "?";
}

static const char *
msp_status_box_name(int val)
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

    return "?";
}

static int
msp_cmd_status(int fd)
{
    struct msp_status st;
    int rc, bit;

    rc = msp_status(fd, &st);
    if (rc) {
        perror("msp_status");
        goto out;
    }

    printf("status.cycle_time: %u\n", st.cycle_time);

    printf("status.i2c_errcnt: %u\n", st.i2c_errcnt);

    printf("status.hwcaps: %#x%s",
           st.hwcaps, st.hwcaps ? " (" : "\n");
    for_each_bit(bit, &st.hwcaps)
        printf("%s%s",
               msp_status_hwcap_name(bit),
               bit == st.hwcaps ? ")\n" : ", ");

    printf("status.box: %#x%s",
           st.box, st.box ? " (" : "\n");
    for_each_bit(bit, &st.box)
        printf("%s%s",
               msp_status_box_name(bit),
               bit == st.box ? ")\n" : ", ");
out:
    return rc;
}

static int
msp_servo(int fd, struct msp_servo *servo)
{
    int rc, i;

    rc = msp_req_send(fd, MSP_SERVO, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_SERVO, servo, sizeof(*servo));
    if (rc)
        goto out;

    for (i = 0; i < array_size(servo->ctl); i++)
        servo->ctl[i] = avrtoh(servo->ctl[i]);
out:
    return rc;
}

static int
msp_cmd_servo(int fd)
{
    struct msp_servo servo;
    int rc, i;

    rc = msp_servo(fd, &servo);
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
msp_motor(int fd, struct msp_motor *motor)
{
    int rc, i;

    rc = msp_req_send(fd, MSP_MOTOR, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_MOTOR, motor, sizeof(*motor));
    if (rc)
        goto out;

    for (i = 0; i < array_size(motor->ctl); i++)
        motor->ctl[i] = avrtoh(motor->ctl[i]);
out:
    return rc;
}

static int
msp_cmd_motor(int fd)
{
    struct msp_motor motor;
    int rc, i;

    rc = msp_motor(fd, &motor);
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
msp_motor_pins(int fd, struct msp_motor_pins *pins)
{
    int rc;

    rc = msp_req_send(fd, MSP_MOTOR_PINS, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_MOTOR_PINS, pins, sizeof(*pins));
out:
    return rc;
}

static int
msp_cmd_motor_pins(int fd)
{
    struct msp_motor_pins pins;
    int rc, i;

    rc = msp_motor_pins(fd, &pins);
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
msp_rc(int fd, struct msp_raw_rc *rrc)
{
    int rc, i;

    rc = msp_req_send(fd, MSP_RC, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_RC, rrc, sizeof(*rrc));
    if (rc)
        goto out;

    for (i = 0; i < array_size(rrc->chn); i++)
        rrc->chn[i] = avrtoh(rrc->chn[i]);
out:
    return rc;
}

static int
msp_cmd_rc(int fd)
{
    struct msp_raw_rc rrc;
    int rc, i;

    rc = msp_rc(fd, &rrc);
    if (rc) {
        perror("msp_rrc");
        goto out;
    }

    for (i = 0; i < array_size(rrc.chn); i++)
        printf("rc.chn[%d]: %d\n", i, rrc.chn[i]);
out:
    return rc;
}

static int
msp_bat(int fd, struct msp_bat *bat)
{
    int rc;

    rc = msp_req_send(fd, MSP_BAT, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_BAT, bat, sizeof(*bat));
out:
    return rc;
}

static int
msp_cmd_bat(int fd)
{
    struct msp_bat bat;
    int rc;

    rc = msp_bat(fd, &bat);
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
            "  rc -- read RC controls\n"
            "  reset-conf -- reset params to firmware defaults\n"
            "  servo -- read servo control\n"
            "  status -- read controller status\n"
            "\n");
}

int
main(int argc, char **argv)
{
    const char *ttypath;
    speed_t speed;
    int rc, fd;

    fd = -1;
    rc = -1;
    ttypath = "/dev/ttyUSB0";
    speed = B115200;

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
            speed = msp_tty_speed(atoi(optarg));
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

    fd = msp_tty_open(ttypath, speed);
    if (fd < 0) {
        perror(ttypath);
        goto out;
    }

    for (; optind < argc; optind++) {
        const char *cmd;

        cmd = argv[optind];

        switch (cmd[0]) {
        case 'a':
            if (!strcmp(cmd, "acc-calibration")) {
                rc = msp_cmd_acc_calibration(fd);
                break;
            }
            if (!strcmp(cmd, "altitude")) {
                rc = msp_cmd_altitude(fd);
                break;
            }
            if (!strcmp(cmd, "attitude")) {
                rc = msp_cmd_attitude(fd);
                break;
            }
            goto invalid;
        case 'b':
            if (!strcmp(cmd, "bat")) {
                rc = msp_cmd_bat(fd);
                break;
            }
            goto invalid;
        case 'e':
            if (!strcmp(cmd, "eeprom-write")) {
                rc = msp_cmd_eeprom_write(fd);
                break;
            }
            goto invalid;
        case 'i':
            if (!strcmp(cmd, "ident")) {
                rc = msp_cmd_ident(fd);
                break;
            }
            goto invalid;
        case 'm':
            if (!strcmp(cmd, "mag-calibration")) {
                rc = msp_cmd_mag_calibration(fd);
                break;
            }
            if (!strcmp(cmd, "motor")) {
                rc = msp_cmd_motor(fd);
                break;
            }
            if (!strcmp(cmd, "motor-pins")) {
                rc = msp_cmd_motor_pins(fd);
                break;
            }
            goto invalid;
        case 'r':
            if (!strcmp(cmd, "raw-imu")) {
                rc = msp_cmd_raw_imu(fd);
                break;
            }
            if (!strcmp(cmd, "rc")) {
                rc = msp_cmd_rc(fd);
                break;
            }
            if (!strcmp(cmd, "reset-conf")) {
                rc = msp_cmd_reset_conf(fd);
                break;
            }
            goto invalid;
        case 's':
            if (!strcmp(cmd, "servo")) {
                rc = msp_cmd_servo(fd);
                break;
            }
            if (!strcmp(cmd, "status")) {
                rc = msp_cmd_status(fd);
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
