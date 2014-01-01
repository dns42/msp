#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

void
lua_msp_decode_table(struct lua_State *L,
                     void **_data, const struct msp_msg_field *info)
{
    void *data;

    data = *_data;

    lua_newtable(L);

    for (; info->name != NULL; info++) {
        lua_Number num;

        switch (info->type) {
        case FIELD_T_UINT8:
        case FIELD_T_UINT16:
        case FIELD_T_UINT32:

            switch (info->type) {
            case FIELD_T_UINT8:
                num = * (uint8_t) data;
                data += sizeof(uint8_t);
                break;
            case FIELD_T_UINT16:
                num = * (uint16_t) data;
                data += sizeof(uint16_t);
                break;
            case FIELD_T_UINT32:
                num = * (uint32_t) data;
                data += sizeof(uint32_t);
                break;
            }

            lua_pushnumber(L, num);
            break;

        case FIELD_T_STRUCT:
            lua_msp_decode_table(L, &data, field_info->elem);
            break;

        default:
            abort();
        }


        lua_setfield(L, -2, info->name);
    }

    *_data = data;
}

void
lua_msp_decode(struct lua_State *L,
               const struct msp_hdr *hdr, void *data)
{
    struct msp_cmd_info *cmd_info;
    struct msp_msg_info *msg_info;
    struct msp_field_field *field_info;

    msp_info_for_each_cmd_info(cmd_info)
        if (cmd_info->cmd == hdr->cmd)
            break;

    assert(info != NULL);

    msg_info = &cmd_info->rsp;

    switch (msg_info->type) {
    case MSG_T_STRUCT: {

        lua_newtable(L);

        msp_info_for_each_field(field_info, &cmd_info->rsp) {
        }

    }
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
