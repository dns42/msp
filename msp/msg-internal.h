#ifndef MSP_MSG_INTERNAL_H
#define MSP_MSG_INTERNAL_H

#include <msp/msg.h>
#include <stdint.h>

struct msp_msg_info {
    const char *tag; /* name */
    uint8_t sup; /* supported */
    uint8_t min; /* min rsp len */
    uint8_t max; /* max rsp len */
    int (*req)(const struct msp_hdr *hdr, void *data); /* req encoder */
    int (*rsp)(const struct msp_hdr *hdr, void *data); /* rsp decoder */
};

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
