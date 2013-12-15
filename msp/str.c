#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <msp/str.h>

#include <stdlib.h>

const char *
msp_ident_multitype_name(enum msp_multitype type)
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

const char *
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

const char *
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

const char *
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

    return NULL;
}

const char *
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

    return NULL;
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
