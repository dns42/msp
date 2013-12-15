#ifndef MSP_STR_H
#define MSP_STR_H

#include <msp/msg.h>

const char *msp_ident_multitype_name(enum msp_multitype type);

const char *msp_ident_capability_name(int val);

const char *msp_rc_chan_name(enum msp_rc_chn n);

const char *msp_status_box_name(int val);

const char *msp_status_hwcap_name(int val);

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
