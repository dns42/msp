#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <msp/meta.h>
#include <stdlib.h>

const struct msp_msg_field msp_pid_field[] = {
    {
        .name = "P",
        .type = FIELD_T_UINT8,
    },
    {
        .name = "I",
        .type = FIELD_T_UINT8,
    },
    {
        .name = "D",
        .type = FIELD_T_UINT8,
    },
};

const struct msp_cmd_info msp_cmd_infos[] = {
    {
        .name = "ident",
        .cmd = MSP_IDENT,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "fwversion",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = "multitype",
                    .type = FIELD_T_UINT8,
                    .vals = (struct msp_field_value []) {
                        {
                            .name = "tri",
                            .type = VALUE_T_ENUM,
                            .value = 1,
                        },
                        {
                            .name = "quadp",
                            .type = VALUE_T_ENUM,
                            .value = 2,
                        },
                        {
                            .name = "quadx",
                            .type = VALUE_T_ENUM,
                            .value = 3,
                        },
                        {
                            .name = "bi",
                            .type = VALUE_T_ENUM,
                            .value = 4,
                        },
                        {
                            .name = "gimbal",
                            .type = VALUE_T_ENUM,
                            .value = 5,
                        },
                        {
                            .name = "y6",
                            .type = VALUE_T_ENUM,
                            .value = 6,
                        },
                        {
                            .name = "hex6",
                            .type = VALUE_T_ENUM,
                            .value = 7,
                        },
                        {
                            .name = "wing",
                            .type = VALUE_T_ENUM,
                            .value = 8,
                        },
                        {
                            .name = "y4",
                            .type = VALUE_T_ENUM,
                            .value = 9,
                        },
                        {
                            .name = "hex6x",
                            .type = VALUE_T_ENUM,
                            .value = 10 },
                        {
                            .name = "octox8",
                            .type = VALUE_T_ENUM,
                            .value = 11,
                        },
                        {
                            .name = "octoflatp",
                            .type = VALUE_T_ENUM,
                            .value = 12,
                        },
                        {
                            .name = "octoflatx",
                            .type = VALUE_T_ENUM,
                            .value = 13,
                        },
                        {
                            .name = "airplane",
                            .type = VALUE_T_ENUM,
                            .value = 14,
                        },
                        {
                            .name = "heli_120_ccpm",
                            .type = VALUE_T_ENUM,
                            .value = 15
                        },
                        {
                            .name = "heli_90_deg",
                            .type = VALUE_T_ENUM,
                            .value = 16,
                        },
                        {
                            .name = "vtail4",
                            .type = VALUE_T_ENUM,
                            .value = 17,
                        },
                        {
                            .name = NULL,
                        },
                    },
                },
                {
                    .name = "mspversion",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = "capabilities",
                    .type = FIELD_T_UINT32,
                    .vals = (struct msp_field_value []) {
                        {
                            .name = "bind",
                            .type = VALUE_T_BITS,
                            .value = 0x1,
                        },
                        {
                            .name = "dynbal",
                            .type = VALUE_T_BITS,
                            .value = 0x4,
                        },
                        {
                            .name = "flap",
                            .type = VALUE_T_BITS,
                            .value = 0x8,
                        },
                    },
                },
                {
                    .name = NULL,
                },
            },
        },
    },
    {
        .name = "status",
        .cmd = MSP_STATUS,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "cycletime",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = "i2cerrcnt",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = "hwcaps",
                    .type = FIELD_T_UINT8,
                    .vals = (struct msp_field_value []) {
                        {
                            .name = "acc",
                            .type = VALUE_T_BITS,
                            .value = 0x1,
                        },
                        {
                            .name = "baro",
                            .type = VALUE_T_BITS,
                            .value = 0x2,
                        },
                        {
                            .name = "mag",
                            .type = VALUE_T_BITS,
                            .value = 0x4,
                        },
                        {
                            .name = "gps",
                            .type = VALUE_T_BITS,
                            .value = 0x8,
                        },
                        {
                            .name = "sonar",
                            .type = VALUE_T_BITS,
                            .value = 0x10,
                        },
                    },
                },
                {
                    .name = "box",
                    .type = FIELD_T_UINT32,
                },
                {
                    .name = "conf",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = NULL,
                },
            },
        },
    },
    {
        .name = "raw_imu",
        .cmd = MSP_RAW_IMU,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "acc",
                    .type = FIELD_T_UINT16,
                    .rep = 2,
                },
                {
                    .name = "gyr",
                    .type = FIELD_T_UINT16,
                    .rep = 2,
                },
                {
                    .name = "mag",
                    .type = FIELD_T_UINT16,
                    .rep = 2,
                },
                {
                    .name = NULL,
                },
            },
        },
    },
    {
        .name = "servo",
        .cmd = MSP_SERVO,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "chn",
                    .type = FIELD_T_UINT16,
                    .rep = REP_EOM,
                },
                {
                    .name = NULL,
                },
            },
        },
    },
    {
        .name = "motor",
        .cmd = MSP_MOTOR,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "chn",
                    .type = FIELD_T_UINT16,
                    .rep = REP_EOM,
                },
                {
                    .name = NULL,
                },
            },
        },
    },
    {
        .name = "rc",
        .cmd = MSP_RC,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "chn",
                    .type = FIELD_T_UINT16,
                    .rep = REP_EOM,
                },
            },
        },
    },
    {
        .name = "raw_gps",
        .cmd = MSP_RAW_GPS,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "fix",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = "numSat",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = "coord_lat",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = "coord_lon",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = "altitude",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = "speed",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = "ground_course",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = NULL,
                },
            },
        },
    },
    {
        .name = "comp_gps",
        .cmd = MSP_COMP_GPS,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "distanceToHome",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = "directionToHome",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = "update",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = NULL,
                },
            },
        },
    },
    {
        .name = "attitude",
        .cmd = MSP_ATTITUDE,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "roll",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = "pitch",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = "heading",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = "headFreeModeHold",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = NULL,
                },
            },
        },
    },
    {
        .name = "analog",
        .cmd = MSP_ANALOG,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "vbat",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = "intPowerMeterSum",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = "rssi",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = NULL,
                },
            },
        },
    },
    {
        .name = "rc_tuning",
        .cmd = MSP_RC_TUNING,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "rcRate",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = "rcExpo",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = "rollPitchRate",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = "yawRate",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = "dynThrPID",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = "thrMid8",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = "thrExpo8",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = NULL,
                },
            },
        },
    },
    {
        .name = "pid",
        .cmd = MSP_PID,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "roll",
                    .type = FIELD_T_STRUCT,
                    .elem = msp_pid_field,
                },
                {
                    .name = "pitch",
                    .type = FIELD_T_STRUCT,
                    .elem = msp_pid_field,
                },
                {
                    .name = "yaw",
                    .type = FIELD_T_STRUCT,
                    .elem = msp_pid_field,
                },
                {
                    .name = "alt",
                    .type = FIELD_T_STRUCT,
                    .elem = msp_pid_field,
                },
                {
                    .name = "pos",
                    .type = FIELD_T_STRUCT,
                    .elem = msp_pid_field,
                },
                {
                    .name = "posr",
                    .type = FIELD_T_STRUCT,
                    .elem = msp_pid_field,
                },
                {
                    .name = "navr",
                    .type = FIELD_T_STRUCT,
                    .elem = msp_pid_field,
                },
                {
                    .name = "level",
                    .type = FIELD_T_STRUCT,
                    .elem = msp_pid_field,
                },
                {
                    .name = "mag",
                    .type = FIELD_T_STRUCT,
                    .elem = msp_pid_field,
                },
                {
                    .name = "vel",
                    .type = FIELD_T_STRUCT,
                    .elem = msp_pid_field,
                },
            },
        },
    },
    {
        .name = "pidnames",
        .cmd = MSP_PIDNAMES,
        .rsp = {
            .type = MSG_T_NAMES,
        },
    },
    {
        .name = "boxnames",
        .cmd = MSP_BOXNAMES,
        .rsp = {
            .type = MSG_T_NAMES,
        },
    },
    {
        .name = "boxids",
        .cmd = MSP_BOXIDS,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "item",
                    .type = FIELD_T_UINT8,
                    .rep = REP_EOM,
                },
                {
                    .name = NULL,
                },
            },
        },
    },
    {
        .name = "misc",
        .cmd = MSP_MISC,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "intPowerTrigger1",
                    .type = FIELD_T_UINT16,
                    .rep = -1,
                },
                {
                    .name = NULL,
                },
            },
        }
    },
    {
        .name = "motor_pins",
        .cmd = MSP_MOTOR_PINS,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "pins",
                    .type = FIELD_T_UINT8,
                    .rep = -1,
                },
            },
        },
    },
    {
        .name = "wp",
        .cmd = MSP_WP,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "wp_no",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = "lat",
                    .type = FIELD_T_UINT32,
                },
                {
                    .name = "lon",
                    .type = FIELD_T_UINT32,
                },
                {
                    .name = "alt",
                    .type = FIELD_T_UINT32,
                },
                {
                    .name = "heading",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = "tts",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = "nav",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = NULL,
                }
            },
        },
    },
    {
        .name = "set_wp",
        .cmd = MSP_SET_WP,
        .req = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "wp_no",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = "lat",
                    .type = FIELD_T_UINT32,
                },
                {
                    .name = "lon",
                    .type = FIELD_T_UINT32,
                },
                {
                    .name = "alt",
                    .type = FIELD_T_UINT32,
                },
                {
                    .name = "heading",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = "tts",
                    .type = FIELD_T_UINT16,
                },
                {
                    .name = "nav",
                    .type = FIELD_T_UINT8,
                },
                {
                    .name = NULL,
                },
            },
        },
    },
    {
        .name = "reset_conf",
        .cmd = MSP_RESET_CONF,
    },
    {
        .name = "acc_calibration",
        .cmd = MSP_ACC_CALIBRATION,
    },
    {
        .name = "mag_calibration",
        .cmd = MSP_MAG_CALIBRATION,
    },
    {
        .name = "spek_bind",
        .cmd = MSP_BIND,
    },
    {
        .name = "eeprom_write",
        .cmd = MSP_EEPROM_WRITE,
    },
    {
        .name = "debug",
        .cmd = MSP_DEBUG,
        .rsp = {
            .type = MSG_T_STRUCT,
            .data = (struct msp_msg_field []) {
                {
                    .name = "debug",
                    .type = FIELD_T_UINT16,
                    .rep = -1,
                },
                {
                    .name = NULL,
                },
            },
        },
    },
    {
        .name = "debugmsg",
        .cmd = MSP_DEBUGMSG,
        .rsp = {
            .type = MSG_T_STRING,
        },
    },
    {
        .name = NULL,
    },
};

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
