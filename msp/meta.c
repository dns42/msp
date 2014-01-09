#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <msp/meta.h>
#include <stdlib.h>

const struct msp_struct_field msp_pid_fields[] = {
    {
        .name = "P",
        .data = {
            .type = MSP_DATA_T_UINT8,
        },
    },
    {
        .name = "I",
        .data = {
            .type = MSP_DATA_T_UINT8,
        },
    },
    {
        .name = "D",
        .data = {
            .type = MSP_DATA_T_UINT8,
        },
    },
    {
        .name = NULL,
    },
};

const struct msp_cmd_info msp_cmd_infos[] = {
    {
        .name = "ident",
        .cmd = MSP_IDENT,
        .rsp = {
            .type = MSP_DATA_T_STRUCT,
            .fields = (struct msp_struct_field []) {
                {
                    .name = "fwversion",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    }
                },
                {
                    .name = "multitype",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                        .vals = (struct msp_int_value []) {
                            {
                                .name = "tri",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 1,
                            },
                            {
                                .name = "quadp",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 2,
                            },
                            {
                                .name = "quadx",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 3,
                            },
                            {
                                .name = "bi",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 4,
                            },
                            {
                                .name = "gimbal",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 5,
                            },
                            {
                                .name = "y6",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 6,
                            },
                            {
                                .name = "hex6",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 7,
                            },
                            {
                                .name = "wing",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 8,
                            },
                            {
                                .name = "y4",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 9,
                            },
                            {
                                .name = "hex6x",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 10 },
                            {
                                .name = "octox8",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 11,
                            },
                            {
                                .name = "octoflatp",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 12,
                            },
                            {
                                .name = "octoflatx",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 13,
                            },
                            {
                                .name = "airplane",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 14,
                            },
                            {
                                .name = "heli_120_ccpm",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 15
                            },
                            {
                                .name = "heli_90_deg",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 16,
                            },
                            {
                                .name = "vtail4",
                                .type = MSP_VALUE_T_ENUM,
                                .value = 17,
                            },
                            {
                                .type = 0,
                            },
                        },
                    },
                },
                {
                    .name = "mspversion",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
                },
                {
                    .name = "capabilities",
                    .data = {
                        .type = MSP_DATA_T_UINT32,
                        .vals = (struct msp_int_value []) {
                            {
                                .name = "bind",
                                .type = MSP_VALUE_T_BITS,
                                .value = 0x1,
                            },
                            {
                                .name = "dynbal",
                                .type = MSP_VALUE_T_BITS,
                                .value = 0x4,
                            },
                            {
                                .name = "flap",
                                .type = MSP_VALUE_T_BITS,
                                .value = 0x8,
                            },
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
            .type = MSP_DATA_T_STRUCT,
            .fields = (struct msp_struct_field []) {
                {
                    .name = "cycletime",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
                },
                {
                    .name = "i2cerrcnt",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
                },
                {
                    .name = "hwcaps",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                        .vals = (struct msp_int_value []) {
                            {
                                .name = "acc",
                                .type = MSP_VALUE_T_BITS,
                                .value = 0x1,
                            },
                            {
                                .name = "baro",
                                .type = MSP_VALUE_T_BITS,
                                .value = 0x2,
                            },
                            {
                                .name = "mag",
                                .type = MSP_VALUE_T_BITS,
                                .value = 0x4,
                            },
                            {
                                .name = "gps",
                                .type = MSP_VALUE_T_BITS,
                                .value = 0x8,
                            },
                            {
                                .name = "sonar",
                                .type = MSP_VALUE_T_BITS,
                                .value = 0x10,
                            },
                        },
                    },
                },
                {
                    .name = "box",
                    .data = {
                        .type = MSP_DATA_T_UINT32,
                    },
                },
                {
                    .name = "conf",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
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
            .type = MSP_DATA_T_STRUCT,
            .fields = (struct msp_struct_field []) {
                {
                    .name = "acc",
                    .data = {
                        .type = MSP_DATA_T_ARRAY,
                        .elem = &(struct msp_array) {
                            .data = {
                                .type = MSP_DATA_T_UINT16,
                            },
                            .cnt = 3,
                        },
                    },
                },
                {
                    .name = "gyr",
                    .data = {
                        .type = MSP_DATA_T_ARRAY,
                        .elem = &(struct msp_array) {
                            .data = {
                                .type = MSP_DATA_T_UINT16,
                            },
                            .cnt = 3,
                        },
                    },
                },
                {
                    .name = "mag",
                    .data = {
                        .type = MSP_DATA_T_ARRAY,
                        .elem = &(struct msp_array) {
                            .data = {
                                .type = MSP_DATA_T_UINT16,
                            },
                            .cnt = 3,
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
        .name = "servo",
        .cmd = MSP_SERVO,
        .rsp = {
            .type = MSP_DATA_T_ARRAY,
            .elem = &(struct msp_array) {
                .data = {
                    .type = MSP_DATA_T_UINT16,
                },
                .cnt = MSP_SIZE_EOM,
            },
        },
    },
    {
        .name = "motor",
        .cmd = MSP_MOTOR,
        .rsp = {
            .type = MSP_DATA_T_ARRAY,
            .elem = &(struct msp_array) {
                .data = {
                    .type = MSP_DATA_T_UINT16,
                },
                .cnt = MSP_SIZE_EOM,
            },
        },
    },
    {
        .name = "rc",
        .cmd = MSP_RC,
        .rsp = {
            .type = MSP_DATA_T_ARRAY,
            .elem = &(struct msp_array) {
                .data = {
                    .type = MSP_DATA_T_UINT16,
                },
                .cnt = MSP_SIZE_EOM,
            },
        },
    },
    {
        .name = "raw_gps",
        .cmd = MSP_RAW_GPS,
        .rsp = {
            .type = MSP_DATA_T_STRUCT,
            .fields = (struct msp_struct_field []) {
                {
                    .name = "fix",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    }
                },
                {
                    .name = "numSat",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    }
                },
                {
                    .name = "coord_lat",
                    .data = {
                        .type = MSP_DATA_T_UINT32,
                    }
                },
                {
                    .name = "coord_lon",
                    .data = {
                        .type = MSP_DATA_T_UINT32,
                    }
                },
                {
                    .name = "altitude",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    }
                },
                {
                    .name = "speed",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    }
                },
                {
                    .name = "ground_course",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    }
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
            .type = MSP_DATA_T_STRUCT,
            .fields = (struct msp_struct_field []) {
                {
                    .name = "distanceToHome",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    },
                },
                {
                    .name = "directionToHome",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    },
                },
                {
                    .name = "update",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
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
            .type = MSP_DATA_T_STRUCT,
            .fields = (struct msp_struct_field []) {
                {
                    .name = "roll",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    }
                },
                {
                    .name = "pitch",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    }
                },
                {
                    .name = "heading",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    }
                },
                {
                    .name = "headFreeModeHold",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    }
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
            .type = MSP_DATA_T_STRUCT,
            .fields = (struct msp_struct_field []) {
                {
                    .name = "vbat",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
                },
                {
                    .name = "intPowerMeterSum",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    },
                },
                {
                    .name = "rssi",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    },
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
            .type = MSP_DATA_T_STRUCT,
            .fields = (struct msp_struct_field []) {
                {
                    .name = "rcRate",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
                },
                {
                    .name = "rcExpo",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
                },
                {
                    .name = "rollPitchRate",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
                },
                {
                    .name = "yawRate",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
                },
                {
                    .name = "dynThrPID",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
                },
                {
                    .name = "thrMid8",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
                },
                {
                    .name = "thrExpo8",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
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
            .type = MSP_DATA_T_STRUCT,
            .fields = (struct msp_struct_field []) {
                {
                    .name = "roll",
                    .data = {
                        .type = MSP_DATA_T_STRUCT,
                        .fields = msp_pid_fields,
                    },
                },
                {
                    .name = "pitch",
                    .data = {
                        .type = MSP_DATA_T_STRUCT,
                        .fields = msp_pid_fields,
                    },
                },
                {
                    .name = "yaw",
                    .data = {
                        .type = MSP_DATA_T_STRUCT,
                        .fields = msp_pid_fields,
                    },
                },
                {
                    .name = "alt",
                    .data = {
                        .type = MSP_DATA_T_STRUCT,
                        .fields = msp_pid_fields,
                    },
                },
                {
                    .name = "pos",
                    .data = {
                        .type = MSP_DATA_T_STRUCT,
                        .fields = msp_pid_fields,
                    },
                },
                {
                    .name = "posr",
                    .data = {
                        .type = MSP_DATA_T_STRUCT,
                        .fields = msp_pid_fields,
                    },
                },
                {
                    .name = "navr",
                    .data = {
                        .type = MSP_DATA_T_STRUCT,
                        .fields = msp_pid_fields,
                    },
                },
                {
                    .name = "level",
                    .data = {
                        .type = MSP_DATA_T_STRUCT,
                        .fields = msp_pid_fields,
                    },
                },
                {
                    .name = "mag",
                    .data = {
                        .type = MSP_DATA_T_STRUCT,
                        .fields = msp_pid_fields,
                    },
                },
                {
                    .name = "vel",
                    .data = {
                        .type = MSP_DATA_T_STRUCT,
                        .fields = msp_pid_fields,
                    },
                },
                {
                    .name = NULL,
                },
            },
        },
    },
    {
        .name = "pidnames",
        .cmd = MSP_PIDNAMES,
        .rsp = {
            .type = MSP_DATA_T_NAMES,
        },
    },
    {
        .name = "boxnames",
        .cmd = MSP_BOXNAMES,
        .rsp = {
            .type = MSP_DATA_T_NAMES,
        },
    },
    {
        .name = "boxids",
        .cmd = MSP_BOXIDS,
        .rsp = {
            .type = MSP_DATA_T_ARRAY,
            .elem = &(struct msp_array) {
                .data = {
                    .type = MSP_DATA_T_UINT8,
                },
                .cnt = MSP_SIZE_EOM,
            },
        },
    },
    {
        .name = "box",
        .cmd = MSP_BOX,
        .rsp = {
            .type = MSP_DATA_T_ARRAY,
            .elem = &(struct msp_array) {
                .data = {
                    .type = MSP_DATA_T_UINT16,
                },
                .cnt = MSP_SIZE_EOM,
            },
        },
    },
    {
        .name = "misc",
        .cmd = MSP_MISC,
        .rsp = {
            .type = MSP_DATA_T_STRUCT,
            .fields = (struct msp_struct_field []) {
                {
                    .name = "intPowerTrigger1",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    },
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
            .type = MSP_DATA_T_ARRAY,
            .elem = &(struct msp_array) {
                .data = {
                    .type = MSP_DATA_T_UINT8,
                },
                .cnt = MSP_SIZE_EOM,
            },
        },
    },
    {
        .name = "wp",
        .cmd = MSP_WP,
        .rsp = {
            .type = MSP_DATA_T_STRUCT,
            .fields = (struct msp_struct_field []) {
                {
                    .name = "wp_no",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
                },
                {
                    .name = "lat",
                    .data = {
                        .type = MSP_DATA_T_UINT32,
                    },
                },
                {
                    .name = "lon",
                    .data = {
                        .type = MSP_DATA_T_UINT32,
                    },
                },
                {
                    .name = "alt",
                    .data = {
                        .type = MSP_DATA_T_UINT32,
                    },
                },
                {
                    .name = "heading",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    },
                },
                {
                    .name = "tts",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    },
                },
                {
                    .name = "nav",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
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
            .type = MSP_DATA_T_STRUCT,
            .fields = (struct msp_struct_field []) {
                {
                    .name = "wp_no",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
                },
                {
                    .name = "lat",
                    .data = {
                        .type = MSP_DATA_T_UINT32,
                    },
                },
                {
                    .name = "lon",
                    .data = {
                        .type = MSP_DATA_T_UINT32,
                    },
                },
                {
                    .name = "alt",
                    .data = {
                        .type = MSP_DATA_T_UINT32,
                    },
                },
                {
                    .name = "heading",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    },
                },
                {
                    .name = "tts",
                    .data = {
                        .type = MSP_DATA_T_UINT16,
                    },
                },
                {
                    .name = "nav",
                    .data = {
                        .type = MSP_DATA_T_UINT8,
                    },
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
            .type = MSP_DATA_T_ARRAY,
            .elem = &(struct msp_array) {
                .data = {
                    .type = MSP_DATA_T_UINT16,
                },
                .cnt = MSP_SIZE_EOM,
            },
        },
    },
    {
        .name = "debugmsg",
        .cmd = MSP_DEBUGMSG,
        .rsp = {
            .type = MSP_DATA_T_STRING,
        },
    },
    {
        .name = "set_raw_rc",
        .cmd = MSP_SET_RAW_RC,
        .req = {
            .type = MSP_DATA_T_ARRAY,
            .elem = &(struct msp_array) {
                .data = {
                    .type = MSP_DATA_T_UINT16,
                },
                .cnt = 8,
            },
        },
    },
    {
        .name = "set_box",
        .cmd = MSP_SET_BOX,
        .req = {
            .type = MSP_DATA_T_ARRAY,
            .elem = &(struct msp_array) {
                .data = {
                    .type = MSP_DATA_T_UINT16,
                },
                .cnt = MSP_SIZE_EOM,
            },
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
