#ifndef MSP_META_H
#define MSP_META_H

#include <msp/msg.h>

enum msp_field_type {
    FIELD_T_UINT8 = 1,
    FIELD_T_UINT16 = 2,
    FIELD_T_UINT32 = 3,
    FIELD_T_STRUCT = 4,
};

enum msp_value_type {
    VALUE_T_ENUM = 0,
    VALUE_T_BITS = 1,
    VALUE_T_BXBT = 2,
};

struct msp_field_value {
    const char *name;
    int type;
    int value;
};

#define REP_EOM -1

struct msp_msg_field {
    const char *name;
    enum msp_field_type type;
    int rep;
    union {
        const struct msp_field_value *vals;
        const struct msp_msg_field *elem;
    };
};

enum msp_msg_type {
    MSG_T_STRUCT = 1,
    MSG_T_NAMES = 2,
    MSG_T_STRING = 3,
};

struct msp_msg_info {
    enum msp_msg_type type;
    union {
        const struct msp_msg_field *data;
    };
};

struct msp_cmd_info {
    msp_cmd_t cmd;
    const char *name;
    struct msp_msg_info req;
    struct msp_msg_info rsp;
};

extern const struct msp_cmd_info msp_cmd_infos[];

#define msp_info_for_each_cmd(_info)                               \
    for (_info = msp_cmd_infos;                                    \
         (_info) = (_info)->name != NULL ? (_info) : NULL,         \
             (_info) != NULL;                                      \
         (_info)++)

#define msp_info_for_each_field(_info, _msg_info)                  \
    for (_info = (_msg_info)->data;                                \
         (_info) = (_info)->name != NULL ? (_info) : NULL,         \
             (_info) != NULL;                                      \
         (_info)++)

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
