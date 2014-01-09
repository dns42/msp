#ifndef MSP_META_H
#define MSP_META_H

#include <msp/msg.h>

enum msp_data_type {
    MSP_DATA_T_UINT8 = 1,
    MSP_DATA_T_UINT16 = 2,
    MSP_DATA_T_UINT32 = 3,
    MSP_DATA_T_STRUCT = 4,
    MSP_DATA_T_ARRAY = 5,
    MSP_DATA_T_NAMES = 6,
    MSP_DATA_T_STRING = 7,
};

/*
 * decoding integer enum and bit set types.
 */
enum msp_value_type {
    MSP_VALUE_T_ENUM = 1,
    MSP_VALUE_T_BITS = 2,
    MSP_VALUE_T_BXBT = 3,
};

struct msp_int_value {
    const char *name;
    enum msp_value_type type;
    int value;
};

#define MSP_SIZE_EOM -1

/*
 * protocol data. integer, struct, or array.
 */
struct msp_data {
    enum msp_data_type type;
    union {
        const struct msp_int_value *vals;
        const struct msp_struct_field *fields;
        struct msp_array *elem;
    };
};

struct msp_array {
    struct msp_data data;
    int cnt;
};

/*
 * a struct field
 */
struct msp_struct_field {
    const char *name;
    const struct msp_data data;
};

struct msp_cmd_info {
    msp_cmd_t cmd;
    const char *name;
    struct msp_data req;
    struct msp_data rsp;
};

extern const struct msp_cmd_info msp_cmd_infos[];

#define msp_for_each_cmd_info(_info)                               \
    for (_info = msp_cmd_infos;                                    \
         (_info) = ((_info) && (_info)->name) ? (_info) : NULL,    \
             (_info) != NULL;                                      \
         (_info)++)

#define msp_for_each_field_info(_info, _field_info)                \
    for (_info = (_field_info);                                    \
         (_info) = ((_info) && (_info)->name) ? (_info) : NULL,    \
             (_info) != NULL;                                      \
         (_info)++)

#define msp_for_each_value_info(_info, _val_info)                  \
    for (_info = _val_info;                                        \
         (_info) = ((_info) && (_info)->type) ? (_info) : NULL,    \
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
