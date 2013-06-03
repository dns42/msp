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

    return "ufo";
}

static int
msp_cmd_ident(int fd)
{
    struct msp_ident ident;
    int rc;

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

    printf("ident.capabilities: %x\n",
           ident.capabilities);
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
        printf("raw_imu.acc[%d]: %d\n", i, imu.acc[i]);

    for (i = 0; i < array_size(imu.gyr); i++)
        printf("raw_imu.gyr[%d]: %d\n", i, imu.gyr[i]);

    for (i = 0; i < array_size(imu.adc); i++)
        printf("raw_imu.adc[%d]: %d\n", i, imu.adc[i]);

out:
    return rc;
}

static int
msp_altitude(int fd, int32_t *alt)
{
    int rc;

    rc = msp_req_send(fd, MSP_ALTITUDE, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_ALTITUDE, alt, sizeof(*alt));
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
    int rc, i;

    rc = msp_req_send(fd, MSP_ATTITUDE, NULL, 0);
    if (rc)
        goto out;

    rc = msp_rsp_recv(fd, MSP_ATTITUDE, att, sizeof(*att));
    if (rc)
        goto out;

    for (i = 0; i < array_size(att->angle); i++)
        att->angle[i] = avrtoh(att->angle[i]);

    att->heading = avrtoh(att->heading);
    att->headwtf = avrtoh(att->headwtf);
out:
    return rc;
}

static int
msp_cmd_attitude(int fd)
{
    struct msp_attitude att;
    int rc, i;

    rc = msp_attitude(fd, &att);
    if (rc) {
        perror("msp_attitude");
        goto out;
    }

    for (i = 0; i < array_size(att.angle); i++)
        printf("attitude.angle[%d]: %d\n", i, att.angle[i]);

    printf("attitude.heading: %d\n", att.heading);

    printf("attitude.headwtf: %d\n", att.headwtf);
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

    printf("mag_calibration: %s\n", rc ? "err" : "ok" );

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

    printf("acc_calibration: %s\n", rc ? "err" : "ok" );

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

    printf("eeprom_write: %s\n", rc ? "err" : "ok");

    return rc;
}

static void
msp_usage(FILE *s, const char *prog)
{
    fprintf(s,
            "Usage:\n"
            "  %s [ -T <tty> ] [ -b <baud> ] [ -V ] [ -h ]"
            " command [ args .. ] -- ...\n\n", prog);
    fprintf(s,
            "Commands:\n");
    fprintf(s,
            "  acc-calibration -- calibrate accelerometer\n"
            "  altitude -- read altitude\n"
            "  attitude -- read attitude\n"
            "  eeprom-write -- write current params to eeprom\n"
            "  ident -- identify firmware / revision\n"
            "  mag-calibration -- calibrate magnetometer\n"
            "  raw-imu -- read raw IMU data\n");
    fprintf(s,
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

        c = getopt(argc, argv, "T:b:Vh");
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

    while (optind != argc) {
        const char *cmd;

        cmd = argv[optind];
        switch (cmd[0]) {
        case 'a':
            if (!strcmp(cmd, "acc-calibration")) {
                rc = msp_cmd_acc_calibration(fd);
                optind++;
                break;
            }
            if (!strcmp(cmd, "altitude")) {
                rc = msp_cmd_altitude(fd);
                optind++;
                break;
            }
            if (!strcmp(cmd, "attitude")) {
                rc = msp_cmd_attitude(fd);
                optind++;
                break;
            }
            break;
        case 'e':
            if (!strcmp(cmd, "eeprom-write")) {
                rc = msp_cmd_eeprom_write(fd);
                optind++;
                break;
            }
            break;
        case 'i':
            if (!strcmp(cmd, "ident")) {
                rc = msp_cmd_ident(fd);
                optind++;
                break;
            }
            break;
        case 'm':
            if (!strcmp(cmd, "mag-calibration")) {
                rc = msp_cmd_mag_calibration(fd);
                optind++;
                break;
            }
            break;
        case 'r':
            if (!strcmp(cmd, "raw-imu")) {
                rc = msp_cmd_raw_imu(fd);
                optind++;
                break;
            }
            break;
        default:
            rc = -1;
            goto usage;
        }
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
