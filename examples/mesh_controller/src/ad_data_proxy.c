/* Copyright (c) 2025, Alexander Ozumenko
 * All rights reserved.
 *
 */
#include "ad_data_proxy.h"

#include "nrf_mesh_config_core.h"

#include "proxy_config_packet.h"
#include "proxy_filter.h"
#include "core_tx.h"
#include "serial.h"
#include "serial_packet.h"
#include "serial_types.h"
#include "net_beacon.h"
#include "nordic_common.h"
#include "nrf_mesh.h"
#include "nrf_mesh_utils.h"
#include "ad_listener.h"
#include "advertiser.h"
#include "mesh_config_entry.h"
#include "boards.h"
#include "ctrl_led.h"
#include "example_common.h"




/*****************************************************************************
* Static globals
*****************************************************************************/
static bool m_initialized = false;
static bool m_enabled = false;


static struct
{
    advertiser_t advertiser;
    uint8_t packet_buffer[CORE_TX_QUEUE_BUFFER_SIZE_ORIGINATOR];
} m_advertising;




/*****************************************************************************
* Static functions
*****************************************************************************/



/////////////////////////////////////////////////
static void packet_in(uint8_t ad_type, const uint8_t *p_data, uint32_t data_len)
{
    serial_packet_t *p_packet;
    uint32_t status;

    if (!m_initialized || !m_enabled)
        return;

    status = serial_packet_buffer_get(SERIAL_PACKET_OVERHEAD + BLE_AD_DATA_HEADER_LENGTH + data_len, &p_packet);
    if (status == NRF_SUCCESS) {
        p_packet->opcode = SERIAL_OPCODE_EVT_AD_DATA_RECEIVED;
        p_packet->payload.evt.ad_data.data[0] = data_len + BLE_AD_DATA_OVERHEAD;
        p_packet->payload.evt.ad_data.data[1] = ad_type;
        memcpy(p_packet->payload.evt.ad_data.data + 2, p_data, data_len);
        serial_tx(p_packet);
    }

    ctrl_led_blink_count(LED_1, 1, LED_BLINK_SHORT_INTERVAL_MS, 0);
}


static void pb_adv_packet_in(const uint8_t *p_data, uint32_t data_len, const nrf_mesh_rx_metadata_t *p_metadata)
{
    packet_in(AD_TYPE_PB_ADV, p_data, data_len);
}

static void mesh_packet_in(const uint8_t *p_data, uint32_t data_len, const nrf_mesh_rx_metadata_t *p_metadata)
{
    packet_in(AD_TYPE_MESH, p_data, data_len);
}

static void beacon_packet_in(const uint8_t *p_data, uint32_t data_len, const nrf_mesh_rx_metadata_t *p_metadata)
{
    packet_in(AD_TYPE_BEACON, p_data, data_len);
}

AD_LISTENER(m_pb_adv_ad_listener) = {
    .ad_type = AD_TYPE_PB_ADV,
    .adv_packet_type = BLE_PACKET_TYPE_ADV_NONCONN_IND,
    .handler = pb_adv_packet_in,
};

AD_LISTENER(m_mesh_ad_listener) = {
    .ad_type = AD_TYPE_MESH,
    .adv_packet_type = BLE_PACKET_TYPE_ADV_NONCONN_IND,
    .handler = mesh_packet_in,
};

AD_LISTENER(m_beacon_ad_listener) = {
    .ad_type = AD_TYPE_BEACON,
    .adv_packet_type = BLE_PACKET_TYPE_ADV_NONCONN_IND,
    .handler = beacon_packet_in,
};



/* ...  */
static void advertiser_tx_complete_cb(advertiser_t *p_adv,
                                      nrf_mesh_tx_token_t token,
                                      timestamp_t timestamp)
{
    serial_cmd_rsp_send(SERIAL_OPCODE_CMD_AD_DATA_SEND, token,
                        SERIAL_STATUS_SUCCESS, NULL, 0);
}



/* interface functions */

uint32_t ad_data_proxy_init(void)
{
    if (m_initialized)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    advertiser_instance_init(&m_advertising.advertiser,
                             advertiser_tx_complete_cb,
                             m_advertising.packet_buffer,
                             sizeof(m_advertising.packet_buffer));
    m_initialized = true;

    return NRF_SUCCESS;
}


uint32_t proxy_start(void)
{
    if (m_initialized && !m_enabled)
    {
        m_enabled = true;
        advertiser_enable(&m_advertising.advertiser);

        ctrl_led_set(LED_1, false);

        return NRF_SUCCESS;
    }
    else
    {
        return NRF_ERROR_INVALID_STATE;
    }
}


uint32_t proxy_stop(void)
{
    if (m_initialized && m_enabled)
    {
        advertiser_disable(&m_advertising.advertiser);
        m_enabled = false;

        ctrl_led_set(LED_1, true);

        return NRF_SUCCESS;
    }
    else
    {
        return NRF_ERROR_INVALID_STATE;
    }
}


void ad_data_proxy_tx(nrf_mesh_tx_token_t token, const uint8_t *data, int length)
{
    adv_packet_t *p_adv_packet;

    if (!m_initialized || !m_enabled) {
        serial_cmd_rsp_send(SERIAL_OPCODE_CMD_AD_DATA_SEND, token,
                            SERIAL_STATUS_ERROR_INVALID_STATE, NULL, 0);
        return;
    }

    p_adv_packet = advertiser_packet_alloc(&m_advertising.advertiser, length);
    if (p_adv_packet != NULL) {
        p_adv_packet->token = token;
        p_adv_packet->config.repeats = CORE_TX_REPEAT_ORIGINATOR_DEFAULT;
        memcpy(p_adv_packet->packet.payload, data, length);
        advertiser_packet_send(&m_advertising.advertiser, p_adv_packet);
    }

    ctrl_led_blink_count(LED_1, 1, LED_BLINK_SHORT_INTERVAL_MS, 0);
}
