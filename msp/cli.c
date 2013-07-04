#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <msp/msp.h>
#include <msp/defs.h>

#include <crt/defs.h>
#include <crt/tty.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <libgen.h>

static int
msp_cli_acc_calibration(struct msp *msp)
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
msp_cli_altitude(struct msp *msp)
{
    struct msp_altitude alt;
    size_t len;
    int rc;

    len = sizeof(alt);

    rc = msp_altitude(msp, &alt, &len);
    if (rc) {
        perror("msp_altitude");
        goto out;
    }

    printf("altitude.altitude: %d\n", alt.altitude);

    if (len > msg_data_end(&alt, variometer))
        printf("altitude.variometer: %d\n", alt.variometer);
out:
    return rc;
}

static int
msp_cli_attitude(struct msp *msp)
{
    struct msp_attitude att;
    size_t len;
    int rc;

    len = sizeof(att);

    rc = msp_attitude(msp, &att, &len);
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
msp_cli_analog(struct msp *msp)
{
    struct msp_analog analog;
    size_t len;
    int rc;

    len = sizeof(analog);

    rc = msp_analog(msp, &analog, &len);
    if (rc) {
        perror("msp_analog");
        goto out;
    }

    printf("analog.vbat: %u\n", analog.vbat);
    printf("analog.powermetersum: %u\n", analog.powermetersum);

    if (len >= msg_data_end(&analog, rssi))
        printf("analog.rssi: %u\n", analog.rssi);
out:
    return rc;
}

static int
msp_cli_eeprom_write(struct msp *msp)
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
msp_cli_ident(struct msp *msp)
{
    struct msp_ident ident;
    size_t len;
    int rc, bit;

    len = sizeof(ident);

    rc = msp_ident(msp, &ident, &len);
    if (rc) {
        perror("msp_ident");
        goto out;
    }

    printf("ident.fwversion: %u (%u.%u)\n",
           ident.fwversion,
           ident.fwversion / 100, ident.fwversion % 100);

    printf("ident.multitype: %u (%s)\n",
           ident.multitype,
           msp_ident_multitype_name(ident.multitype) ? : "?");

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
msp_cli_mag_calibration(struct msp *msp)
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
msp_cli_motor(struct msp *msp)
{
    struct msp_motor motor;
    size_t len;
    int rc, i;

    len = sizeof(motor);

    rc = msp_motor(msp, &motor, &len);
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
msp_cli_motor_pins(struct msp *msp)
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
msp_cli_raw_imu(struct msp *msp)
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
msp_cli_rc(struct msp *msp)
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
msp_cli_reset_conf(struct msp *msp)
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
msp_cli_servo(struct msp *msp)
{
    struct msp_servo servo;
    size_t len;
    int rc, i;

    rc = msp_servo(msp, &servo, &len);
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
msp_cli_set_raw_rc(struct msp *msp, int argc, char **argv)
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

        rrc.chn[chn] = val;
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
msp_cli_status(struct msp *msp)
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
msp_cli_box(struct msp *msp)
{
    size_t len;
    char *names, *pos;
    uint16_t *items;
    uint8_t *boxids;
    int rc, boxcnt, i;

    rc = -1;

    boxids = NULL;
    items = NULL;

    len = MSP_LEN_MAX;
    names = malloc(len);
    if (!expected(names))
        goto out;

    rc = msp_boxnames(msp, names, &len);
    if (rc) {
        perror("msp_boxnames");
        goto out;
    }

    boxcnt = 0;
    for (i = 0; i < len; i++)
        if (names[i] == ';')
            boxcnt++;

    len = MSP_LEN_MAX;
    boxids = malloc(len);
    if (!expected(boxids))
        goto out;

    rc = msp_boxids(msp, boxids, &len);
    if (rc && errno == ENOSYS) {
        len = boxcnt;
        for (i = 0; i < len; i++)
            boxids[i] = i;
        rc = 0;
    }
    if (rc) {
        perror("msp_boxids");
        goto out;
    }

    if (unexpected(len != boxcnt)) {
        errno = EPROTO;
        goto out;
    }

    rc = -1;
    items = malloc(boxcnt * sizeof(*items));
    if (!expected(items))
        goto out;

    rc = msp_box(msp, items, &boxcnt);
    if (rc) {
        perror("msp_box");
        goto out;
    }

    pos = names;
    for (i = 0; i < boxcnt; i++)
        printf("box.%s(%d): %x\n",
               strsep(&pos, ";"), boxids[i], items[i]);

out:
    if (items)
        free(items);
    if (boxids)
        free(boxids);
    if (names)
        free(names);

    return rc;
}

static int
msp_cli_set_box(struct msp *msp, int argc, char **argv)
{
    size_t len;
    char *names, **namev, *pos;
    uint16_t *items;
    uint8_t *boxids;
    int rc, boxcnt, i;

    rc = -1;

    boxids = NULL;
    items = NULL;
    namev = NULL;

    len = MSP_LEN_MAX;
    names = malloc(len);
    if (!expected(names))
        goto out;

    rc = msp_boxnames(msp, names, &len);
    if (rc) {
        perror("msp_boxnames");
        goto out;
    }

    boxcnt = 0;
    for (i = 0; i < len; i++)
        if (names[i] == ';')
            boxcnt++;

    len = MSP_LEN_MAX;
    boxids = malloc(len);
    if (!expected(boxids))
        goto out;

    rc = msp_boxids(msp, boxids, &len);
    if (rc && errno == ENOSYS) {
        len = boxcnt;
        for (i = 0; i < len; i++)
            boxids[i] = i;
        rc = 0;
    }
    if (rc) {
        perror("msp_boxids");
        goto out;
    }

    if (unexpected(len != boxcnt)) {
        errno = EPROTO;
        goto out;
    }

    rc = -1;
    items = malloc(boxcnt * sizeof(*items));
    if (!expected(items))
        goto out;

    rc = msp_box(msp, items, &boxcnt);
    if (rc) {
        perror("msp_box");
        goto out;
    }

    namev = malloc(boxcnt * sizeof(*namev));
    if (!expected(namev))
        goto out;

    pos = names;
    for (i = 0; i < boxcnt; i++)
        namev[i] = strsep(&pos, ";");

    while (++optind < argc) {
        const char *arg;
        char name[9];
        int16_t val;
        int n;

        arg = argv[optind];
        i = -1;

        n = sscanf(arg, "%8[^:]:%hu", name, &val);
        if (n != 2)
            goto err;

        if (isdigit(name[0])) {
            char *end;
            int id;

            id = strtol(name, &end, 0);
            if (*end != 0)
                goto err;

            for (i = 0; i < boxcnt; i++)
                if (boxids[i] == id)
                    break;

        } else {

            for (i = 0; i < boxcnt; i++)
                if (!strcasecmp(name, namev[i]))
                    break;
        }

    err:
        if (i < 0 || i >= boxcnt) {
            fprintf(stderr,
                    "Invalid box item '%s', "
                    "must be '<id>:<int16>'\n", arg);
            errno = EINVAL;
            goto out;
        }

        items[i] = val;
    }

    pos = names;
    for (i = 0; i < boxcnt; i++)
        printf("box.%s(%d): %x\n",
               namev[i], boxids[i], items[i]);

    rc = msp_set_box(msp, items, boxcnt);
out:
    if (namev)
        free(namev);
    if (items)
        free(items);
    if (boxids)
        free(boxids);
    if (names)
        free(names);

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
            "  analog -- read analog port status\n"
            "  box -- read checkbox items\n"
            "  eeprom-write -- write current params to eeprom\n"
            "  ident -- identify controller firmware\n"
            "  mag-calibration -- calibrate magnetometer\n"
            "  motor -- read motor control\n"
            "  motor-pins -- read motor pin numbers\n"
            "  raw-imu -- read raw IMU data\n"
            "  rc -- read RC channels\n"
            "  reset-conf -- reset params to firmware defaults\n"
            "  servo -- read servo control\n"
            "  set-box -- set box items\n"
            "  set-raw-rc -- set RC channels\n"
            "  status -- read controller status\n"
            "\n");
}

int
main(int argc, char **argv)
{
    const char *ttypath;
    struct msp *msp;
    struct tty *tty;
    struct evtloop *loop;
    speed_t speed;
    int rc, fd;

    fd = -1;
    rc = -1;
    ttypath = "/dev/ttyUSB0";
    speed = B115200;
    msp = NULL;
    tty = NULL;
    loop = NULL;

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

    tty = tty_open(ttypath, speed);
    if (!tty) {
        perror(ttypath);
        goto out;
    }

    loop = evtloop_create();
    if (!loop) {
        perror("evtloop_create");
        goto out;
    }

    rc = tty_plug(tty, loop);
    if (rc) {
        perror("tty_register_events");
        goto out;
    }

    msp = msp_open(tty, loop);
    if (!msp)
        goto out;

    for (; optind < argc; optind++) {
        const char *cmd;

        cmd = argv[optind];

        switch (cmd[0]) {
        case 'a':
            if (!strcmp(cmd, "acc-calibration")) {
                rc = msp_cli_acc_calibration(msp);
                break;
            }
            if (!strcmp(cmd, "altitude")) {
                rc = msp_cli_altitude(msp);
                break;
            }
            if (!strcmp(cmd, "analog")) {
                rc = msp_cli_analog(msp);
                break;
            }
            if (!strcmp(cmd, "attitude")) {
                rc = msp_cli_attitude(msp);
                break;
            }
            goto invalid;
        case 'b':
            if (!strcmp(cmd, "box")) {
                rc = msp_cli_box(msp);
                break;
            }
            goto invalid;
        case 'e':
            if (!strcmp(cmd, "eeprom-write")) {
                rc = msp_cli_eeprom_write(msp);
                break;
            }
            goto invalid;
        case 'i':
            if (!strcmp(cmd, "ident")) {
                rc = msp_cli_ident(msp);
                break;
            }
            goto invalid;
        case 'm':
            if (!strcmp(cmd, "mag-calibration")) {
                rc = msp_cli_mag_calibration(msp);
                break;
            }
            if (!strcmp(cmd, "motor")) {
                rc = msp_cli_motor(msp);
                break;
            }
            if (!strcmp(cmd, "motor-pins")) {
                rc = msp_cli_motor_pins(msp);
                break;
            }
            goto invalid;
        case 'r':
            if (!strcmp(cmd, "raw-imu")) {
                rc = msp_cli_raw_imu(msp);
                break;
            }
            if (!strcmp(cmd, "rc")) {
                rc = msp_cli_rc(msp);
                break;
            }
            if (!strcmp(cmd, "reset-conf")) {
                rc = msp_cli_reset_conf(msp);
                break;
            }
            goto invalid;
        case 's':
            if (!strcmp(cmd, "servo")) {
                rc = msp_cli_servo(msp);
                break;
            }
            if (!strcmp(cmd, "set-box")) {
                rc = msp_cli_set_box(msp, argc, argv);
                break;
            }
            if (!strcmp(cmd, "set-raw-rc")) {
                rc = msp_cli_set_raw_rc(msp, argc, argv);
                break;
            }
            if (!strcmp(cmd, "status")) {
                rc = msp_cli_status(msp);
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
    if (msp)
        msp_close(msp);

    if (tty)
        tty_close(tty);

    if (loop)
        evtloop_destroy(loop);

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
