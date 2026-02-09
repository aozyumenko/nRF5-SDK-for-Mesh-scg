/* Copyright (c) 2025, Alexander Ozumenko
 * All rights reserved.
 */
#include "ad_data_proxy.h"
#include "nrf_mesh_config_core.h"

#include <string.h>

#include "proxy_config_packet.h"
#include "proxy_filter.h"
#include "serial.h"
#include "serial_packet.h"
#include "serial_types.h"
#include "core_tx.h"
#include "bearer_event.h"
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

/* logging */
#include "nrf5_sdk_log.h"



/*********** Definitions ***********/

#define ENABLE_FAST_CACHE               1



/*********** Static globals ***********/

static bool m_initialized = false;
static bool m_enabled = false;

static struct {
    advertiser_t advertiser;
    uint8_t packet_buffer[CORE_TX_QUEUE_BUFFER_SIZE_ORIGINATOR];
} m_advertising;

static uint32_t m_proxy_stop_event_flag;

#ifdef ENABLE_FAST_CACHE
#define FAST_CACHE_SIZE 8
static struct {
    uint8_t length;
    uint8_t hash[8];
} m_fast_cache[FAST_CACHE_SIZE];
static int m_fast_cache_idx = 0;
#endif /* ENABLE_FAST_CACHE */



/*********** Static functions ***********/

#ifdef ENABLE_FAST_CACHE
static inline bool check_fast_cache(const uint8_t *p_data, uint32_t length)
{
    int i;
    size_t hash_length;


    hash_length = (length < sizeof(m_fast_cache[0].hash)) ?
        length : sizeof(m_fast_cache[0].hash);

    for (i = 0; i < ARRAY_SIZE(m_fast_cache); i++) {
        if ((m_fast_cache[i].length == length) && (memcmp(m_fast_cache[i].hash, p_data, hash_length) == 0)) {
            return false;
        }
    }

    m_fast_cache[m_fast_cache_idx].length = length;
    memcpy(m_fast_cache[m_fast_cache_idx].hash, p_data, hash_length);
    m_fast_cache_idx = (m_fast_cache_idx + 1) % ARRAY_SIZE(m_fast_cache);

    return true;
}
#endif /* ENABLE_FAST_CACHE */




static void packet_in(uint8_t ad_type, const uint8_t *p_data, uint32_t data_len)
{
    if (!m_initialized || !m_enabled)
        return;

#ifdef ENABLE_FAST_CACHE
    if (ad_type == AD_TYPE_MESH) {
        if (!check_fast_cache(p_data, data_len)) {
            return;
        }
    }
#endif /* ENABLE_FAST_CACHE */

    UNUSED_VARIABLE(serial_ad_data_send(ad_type, p_data, data_len));

    ctrl_led_blink(LED_1, LED_BLINK_SHORT_INTERVAL_MS);
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


static void advertiser_tx_complete_cb(advertiser_t *p_adv,
                                      nrf_mesh_tx_token_t token,
                                      timestamp_t timestamp)
{
    serial_cmd_rsp_send(SERIAL_OPCODE_CMD_AD_DATA_SEND, token,
                        SERIAL_STATUS_SUCCESS, NULL, 0);
}


static bool proxy_stop_event_handler(void)
{
    advertiser_disable(&m_advertising.advertiser);
    m_enabled = false;
    return true;
}



/* interface functions */

uint32_t ad_data_proxy_init(void)
{
    if (m_initialized) {
        return NRF_ERROR_INVALID_STATE;
    }

#ifdef ENABLE_FAST_CACHE
    memset(m_fast_cache, 0, sizeof(m_fast_cache));
    m_fast_cache_idx = 0;
#endif /* ENABLE_FAST_CACHE */

    advertiser_instance_init(&m_advertising.advertiser,
                             advertiser_tx_complete_cb,
                             m_advertising.packet_buffer,
                             sizeof(m_advertising.packet_buffer));

    m_proxy_stop_event_flag = bearer_event_flag_add(proxy_stop_event_handler);
    m_initialized = true;

    return NRF_SUCCESS;
}


uint32_t proxy_start(void)
{
    if (!m_initialized || m_enabled) {
        return NRF_ERROR_INVALID_STATE;
    }

    m_enabled = true;
    advertiser_enable(&m_advertising.advertiser);

    return NRF_SUCCESS;
}


uint32_t proxy_stop(void)
{
    if (!m_initialized || !m_enabled) {
        return NRF_ERROR_INVALID_STATE;
    }

    bearer_event_flag_set(m_proxy_stop_event_flag);

    return NRF_SUCCESS;
}


void ad_data_proxy_tx(nrf_mesh_tx_token_t token, const uint8_t *data, int length)
{
    adv_packet_t *p_adv_packet;

    if (!m_initialized || !m_enabled) {
        serial_cmd_rsp_send(SERIAL_OPCODE_CMD_AD_DATA_SEND, token,
                            SERIAL_STATUS_ERROR_INVALID_STATE, NULL, 0);
    } else {
        p_adv_packet = advertiser_packet_alloc(&m_advertising.advertiser, length);
        if (p_adv_packet != NULL) {
            p_adv_packet->token = token;
            p_adv_packet->config.repeats = CORE_TX_REPEAT_ORIGINATOR_DEFAULT;
            memcpy(p_adv_packet->packet.payload, data, length);
            advertiser_packet_send(&m_advertising.advertiser, p_adv_packet);
        }

        ctrl_led_blink(LED_1, LED_BLINK_SHORT_INTERVAL_MS);
    }
}
