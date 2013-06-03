#ifndef MSP_MSP_H
#define MSP_MSP_H

#include <stdint.h>

#if __GNUC__
#define PACKED __attribute__((packed))
#endif

/*
 * Multiwii Serial Protocol v0
 */
#define MSP_VERSION             0

typedef uint8_t msp_len_t;
typedef uint8_t msp_cmd_t;

struct msp_hdr {
    char tag[2]; /* '$' 'M' */
    char dsc;    /* '>': req, '<': rsp, '!': err */
    msp_len_t len;
    msp_cmd_t cmd;
} PACKED;

/*
 * get
 *   multitype
 *   multiwii version
 *   protocol version
 *   capability variable
 */
#define MSP_IDENT               100

struct msp_ident {
    uint8_t     fwversion;
    uint8_t     multitype;
    uint8_t     mspversion;
    uint32_t    capabilities;
} PACKED;

enum msp_multitype {
    MSP_MULTITYPE_TRI = 1,
    MSP_MULTITYPE_QUADP = 2,
    MSP_MULTITYPE_QUADX = 3,
    MSP_MULTITYPE_BI = 4,
    MSP_MULTITYPE_GIMBAL = 5,
    MSP_MULTITYPE_Y6 = 6,
    MSP_MULTITYPE_HEX6 = 7,
    MSP_MULTITYPE_FLYING_WING = 8,
    MSP_MULTITYPE_Y4 = 9,
    MSP_MULTITYPE_HEX6X = 10,
    MSP_MULTITYPE_OCTOX8 = 11,
    MSP_MULTITYPE_OCTOFLATP = 12,
    MSP_MULTITYPE_OCTOFLATX = 13,
    MSP_MULTITYPE_AIRPLANE = 14,
    MSP_MULTITYPE_HELI_120_CCPM = 15,
    MSP_MULTITYPE_HELI_90_DEG = 16,
    MSP_MULTITYPE_VTAIL4 = 17,
};

#define MSP_IDENT_CAP_BIND    (1<<0)
#define MSP_IDENT_CAP_DYNBAL  (1<<2)
#define MSP_IDENT_CAP_FLAP    (1<<3)

/*
 * get
 *   cycletime
 *   errors_count
 *   sensor present
 *   box activation
 *   current setting number
 */
#define MSP_STATUS              101

struct msp_status {
    uint16_t cycle_time;
    uint16_t i2c_errcnt;
    uint16_t hwcaps;
    uint32_t box;
} PACKED;

#define MSP_STATUS_HWCAP_ACC     (1<<0)
#define MSP_STATUS_HWCAP_BARO    (1<<1)
#define MSP_STATUS_HWCAP_MAG     (1<<2)
#define MSP_STATUS_HWCAP_GPS     (1<<3)
#define MSP_STATUS_HWCAP_SONAR   (1<<4)

#define MSP_STATUS_BOX_ACC       (1<<0)
#define MSP_STATUS_BOX_BARO      (1<<1)
#define MSP_STATUS_BOX_MAG       (1<<2)
#define MSP_STATUS_BOX_CAMSTAB   (1<<3)
#define MSP_STATUS_BOX_CAMTRIG   (1<<4)
#define MSP_STATUS_BOX_ARM       (1<<5)
#define MSP_STATUS_BOX_GPSHOME   (1<<6)
#define MSP_STATUS_BOX_GPSHOLD   (1<<7)
#define MSP_STATUS_BOX_PASSTHRU  (1<<8)
#define MSP_STATUS_BOX_HEADFREE  (1<<9)
#define MSP_STATUS_BOX_BEEPERON  (1<<10)
#define MSP_STATUS_BOX_LEDMAX    (1<<11)
#define MSP_STATUS_BOX_LLIGHTS   (1<<12)
#define MSP_STATUS_BOX_HEADADJ   (1<<13)

/*
 * get
 *   9 DOF
 */
#define MSP_RAW_IMU             102

struct msp_raw_imu {
    int16_t acc[3];
    int16_t gyr[3];
    int16_t adc[3];
};

/*
 * get
 *   8 servos
 */
#define MSP_SERVO               103

struct msp_servo {
    int16_t ctl[8];
};

/*
 * get
 *   8 motors
 */
#define MSP_MOTOR               104

struct msp_motor {
    int16_t ctl[8];
};

/*
 * get
 *   8 rc chan and more
 */
#define MSP_RC                  105

/*
 * get
 *      fix
 *      numsat
 *      lat
 *      lon
 *      alt
 *      speed
 *      ground course
 */
#define MSP_RAW_GPS             106

/*
 * get
 *      distance home
 *      direction home
 */
#define MSP_COMP_GPS            107

/*
 * get
 *      2 angles 1 heading
 */
#define MSP_ATTITUDE            108

struct msp_attitude {
    int16_t roll;
    int16_t pitch;
    int16_t yaw;
    int16_t yawtf;
};

/*
 * get
 *      altitude
 *      variometer
 */
#define MSP_ALTITUDE            109
/*
 * get
 *      vbat
 *      powermetersum
 *      rssi, if available on rx
 */
#define MSP_ANALOG              110

/*
 * get
 *      rc rate
 *      rc expo
 *      rollpitch rate
 *      yaw rate
 *      dyn throttle PID
 */
#define MSP_RC_TUNING           111

/*
 * get
 *      P I D coeff (9 are used currently)
 */
#define MSP_PID                 112

/*
 * BOX setup (number is dependant of your setup)
 */
#define MSP_BOX                 113

/*
 * get
 *      powermeter trig
 */
#define MSP_MISC                114

/*
 *
 */
#define MSP_MOTOR_PINS          115		//out message		which pins are in use for motors & servos, for GUI 

/*
 *
 */
#define MSP_BOXNAMES            116		//out message		the aux switch names

/*
 *
 */
#define MSP_PIDNAMES            117		//out message		the PID names

/*
 *
 */
#define MSP_WP                  118		//out message		get a WP, WP# is in the payload, returns (WP#, lat, lon, alt, flags) WP#0-home, WP#16-poshold

/*
 *
 */
#define MSP_BOXIDS              119		//out message		get the permanent IDs associated to BOXes

/*
 *
 */
#define MSP_SERVO_CONF          120		//out message		Servo settings


/*
 * set
 *      8 rc chan
 */
#define MSP_SET_RAW_RC          200

/*
 * set
 *      fix
 *      numsat
 *      lat
 *      lon
 *      alt
 *      speed
 */
#define MSP_SET_RAW_GPS         201

/*
 * set
 *      P I D coeff (9 are used currently)
 */
#define MSP_SET_PID             202

/*
 * set
 *      BOX setup (number is dependant of your setup)
 */
#define MSP_SET_BOX             203

/*
 * set
 *      rc rate
 *      rc expo
 *      rollpitch rate
 *      yaw rate
 *      dyn throttle PID
 */
#define MSP_SET_RC_TUNING       204

/*
 * set
 *      (no param)
 */
#define MSP_ACC_CALIBRATION     205

/*
 * set
 *      (no param)
 */
#define MSP_MAG_CALIBRATION     206

/*
 * set
 *      powermeter trig
 *      8 free for future use
 */
#define MSP_SET_MISC            207

/*
 * set
 *      (no param)
 */
#define MSP_RESET_CONF          208

/*
 * set a given WP
 *      WP#
 *      lat
 *      lon
 *      alt
 *      flags
 */
#define MSP_SET_WP              209

/*
 * set
 *      selected setting number (0-2)
 */
#define MSP_SELECT_SETTING      210

/*
 * set
 *      heading hold direction
 */
#define MSP_SET_HEAD            211

/*
 * set
 *      servo settings
 */
#define MSP_SET_SERVO_CONF      212

/*
 * set
 *      propbalance function
 */
#define MSP_SET_MOTOR           214

/*
 * set
 *      no param
 */
#define MSP_BIND                240

/*
 * set
 */
#define MSP_EEPROM_WRITE        250

/*
 * out
 *      debug string buffer
 */
#define MSP_DEBUGMSG            253

/*
 * out
 *      debug1
 *      debug2
 *      debug3
 *      debug4
 */
#define MSP_DEBUG               254

#endif

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
