/* Copyright (c) 2025, Alexander Ozumenko
 * All rights reserved.
 */

#include "serial_cmd_handler.h"

#include <stdint.h>

#include "serial.h"
#include "serial_packet.h"
#include "serial_types.h"
#include "nrf_mesh.h"
#include "nrf_mesh_configure.h"
#include "nrf_mesh_dfu.h"
#include "hal.h"
#include "ad_data_proxy.h"



/* local typedefs */

typedef void (*serial_handler_common_cmd_cb_t)(const serial_packet_t * p_cmd);

typedef struct
{
    uint8_t opcode;
    uint8_t payload_minlen;
    uint8_t payload_optional_extra_bytes;
    serial_handler_common_cmd_cb_t callback;
} serial_handler_common_opcode_to_fp_map_t;



/* static globals */

static struct
{
    uint32_t alloc_fail_count;
} m_hk_data;



/* command handlers */

static void handle_cmd_reset(const serial_packet_t *p_cmd)
{
    hal_device_reset(BL_GPREGRET_FORCED_REBOOT);

    /* Will only trigger if something failed in the reset process. */
    serial_cmd_rsp_send(p_cmd->opcode,
                        p_cmd->payload.cmd.token,
                        SERIAL_STATUS_ERROR_REJECTED,
                        NULL, 0);
}


static void handle_cmd_serial_version_get(const serial_packet_t *p_cmd)
{
    serial_evt_cmd_rsp_data_serial_version_t rsp;

    rsp.serial_ver = SERIAL_API_VERSION;

    serial_cmd_rsp_send(p_cmd->opcode,
                        p_cmd->payload.cmd.token,
                        SERIAL_STATUS_SUCCESS,
                        (uint8_t *) &rsp, sizeof(rsp));
}


static void handle_cmd_start(const serial_packet_t *p_cmd)
{
    uint32_t status;

    status = proxy_start();

    serial_cmd_rsp_send(p_cmd->opcode,
                        p_cmd->payload.cmd.token,
                        serial_translate_error(status),
                        NULL, 0);
}


static void handle_cmd_stop(const serial_packet_t *p_cmd)
{
    uint32_t status;

    status = proxy_stop();

    serial_cmd_rsp_send(p_cmd->opcode,
                        p_cmd->payload.cmd.token,
                        serial_translate_error(status),
                        NULL, 0);
}


static void handle_cmd_ad_data_send(const serial_packet_t *p_cmd)
{
    uint8_t ad_length = p_cmd->payload.cmd.payload.ad_data.data[0];
    uint8_t ad_type = p_cmd->payload.cmd.payload.ad_data.data[1];

    if ((SERIAL_PACKET_OVERHEAD + SERIAL_CMD_OVERHEAD +
         BLE_AD_DATA_LENGTH_OVERHEAD + ad_length) != p_cmd->length)
    {
        serial_cmd_rsp_send(p_cmd->opcode, p_cmd->payload.cmd.token,
                            SERIAL_STATUS_ERROR_INVALID_DATA, NULL, 0);
        return;
    }

    switch (ad_type)
    {
        case AD_TYPE_PB_ADV:
        case AD_TYPE_MESH:
        case AD_TYPE_BEACON:
            ad_data_proxy_tx((nrf_mesh_tx_token_t)p_cmd->payload.cmd.token,
                             p_cmd->payload.cmd.payload.ad_data.data,
                             p_cmd->length - SERIAL_PACKET_OVERHEAD - SERIAL_CMD_OVERHEAD);
            break;
        default:
            serial_cmd_rsp_send(p_cmd->opcode, p_cmd->payload.cmd.token,
                                SERIAL_STATUS_ERROR_INVALID_DATA, NULL, 0);
    }
}


static void handle_cmd_uuid_get(const serial_packet_t *p_cmd)
{
    serial_evt_cmd_rsp_data_device_uuid_t rsp;
    memcpy(rsp.device_uuid, nrf_mesh_configure_device_uuid_get(), NRF_MESH_UUID_SIZE);
    serial_cmd_rsp_send(p_cmd->opcode, p_cmd->payload.cmd.token,
                        SERIAL_STATUS_SUCCESS, (uint8_t *)&rsp, sizeof(rsp));
}


static void handle_cmd_hk_data_get(const serial_packet_t *p_cmd)
{
    serial_evt_cmd_rsp_data_housekeeping_t rsp;

    memset(&rsp, 0, sizeof(rsp));
    rsp.alloc_fail_count = m_hk_data.alloc_fail_count;

    serial_cmd_rsp_send(p_cmd->opcode,
                        p_cmd->payload.cmd.token,
                        SERIAL_STATUS_SUCCESS,
                        (const uint8_t *)&rsp, sizeof(rsp));
}


static void handle_cmd_hk_data_clear(const serial_packet_t *p_cmd)
{
    memset(&m_hk_data, 0, sizeof(m_hk_data));

    serial_cmd_rsp_send(p_cmd->opcode,
                        p_cmd->payload.cmd.token,
                        SERIAL_STATUS_SUCCESS,
                        NULL, 0);
}



/* Serial command handler lookup table. */
static const serial_handler_common_opcode_to_fp_map_t m_cmd_handlers[] =
{
    {SERIAL_OPCODE_CMD_RESET,                   0,                              0,                              handle_cmd_reset},
    {SERIAL_OPCODE_CMD_SERIAL_VERSION_GET,      0,                              0,                              handle_cmd_serial_version_get},
    {SERIAL_OPCODE_CMD_START,                   0,                              0,                              handle_cmd_start},
    {SERIAL_OPCODE_CMD_STOP,                    0,                              0,                              handle_cmd_stop},
    {SERIAL_OPCODE_CMD_AD_DATA_SEND,            BLE_AD_DATA_HEADER_LENGTH,      BLE_AD_DATA_PAYLOAD_MAXLEN,     handle_cmd_ad_data_send},
    {SERIAL_OPCODE_CMD_UUID_GET,                0,                              0,                              handle_cmd_uuid_get},
    {SERIAL_OPCODE_CMD_HOUSEKEEPING_DATA_GET,   0,                              0,                              handle_cmd_hk_data_get},
    {SERIAL_OPCODE_CMD_HOUSEKEEPING_DATA_CLEAR, 0,                              0,                              handle_cmd_hk_data_clear}
};



/* interface functions */

void serial_handler_rx(const serial_packet_t* p_cmd)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(m_cmd_handlers); i++)
    {
        if (p_cmd->opcode == m_cmd_handlers[i].opcode)
        {
            if ((p_cmd->length < (SERIAL_PACKET_LENGTH_OVERHEAD +
                                  SERIAL_CMD_OVERHEAD +
                                  m_cmd_handlers[i].payload_minlen)) ||
                (p_cmd->length > (SERIAL_PACKET_LENGTH_OVERHEAD +
                                  SERIAL_CMD_OVERHEAD +
                                  m_cmd_handlers[i].payload_minlen +
                                  m_cmd_handlers[i].payload_optional_extra_bytes)))
            {
                UNUSED_VARIABLE(serial_cmd_rsp_send(p_cmd->opcode,
                                p_cmd->payload.cmd.token,
                                SERIAL_STATUS_ERROR_INVALID_LENGTH,
                                NULL, 0));
            }
            else
            {
                m_cmd_handlers[i].callback(p_cmd);
            }
            return;
        }
    }
    UNUSED_VARIABLE(serial_cmd_rsp_send(p_cmd->opcode,
                                        p_cmd->payload.cmd.token,
                                        SERIAL_STATUS_ERROR_CMD_UNKNOWN,
                                        NULL, 0));
}


void serial_handler_alloc_fail_report(void)
{
    if (m_hk_data.alloc_fail_count < UINT32_MAX)
    {
        m_hk_data.alloc_fail_count++;
    }
}
