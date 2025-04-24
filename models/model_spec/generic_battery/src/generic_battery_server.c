/* (C) 2025 Alexander Ozumenko
 * St(u)dio of Computer Games (https://stdio.ru/)
 */

#include "generic_battery_server.h"
#include "generic_battery_common.h"
#include "generic_battery_messages.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "access.h"
#include "access_config.h"
#include "nrf_mesh_assert.h"
#include "nrf_mesh_utils.h"
#include "nordic_common.h"

#include "log.h"



static uint32_t status_send(generic_battery_server_t *p_server,
                            const access_message_rx_t *p_message,
                            const generic_battery_status_params_t *p_params)
{
    generic_battery_status_msg_pkt_t msg_pkt;

    if ((p_params->battery_level > GENERIC_BATTERY_LEVEL_MAX
                && p_params->battery_level != GENERIC_BATTERY_LEVEL_UNKNOWN)
            || p_params->discharge_time > GENERIC_BATTERY_TIME_MAX
            || p_params->charge_time > GENERIC_BATTERY_TIME_MAX
            || p_params->flags_presence > GENERIC_BATTERY_FLAGS_PRESENCE_MAX
            || p_params->flags_indicator > GENERIC_BATTERY_FLAGS_INDICATOR_MAX
            || p_params->flags_charging > GENERIC_BATTERY_FLAGS_CHARGING_MAX
            || p_params->flags_service > GENERIC_BATTERY_FLAGS_SERVICE_MAX) {
        return NRF_ERROR_INVALID_PARAM;
    }

    msg_pkt.battery_level = p_params->battery_level;
    generic_battery_time_pack(msg_pkt.discharge_time, p_params->discharge_time);
    generic_battery_time_pack(msg_pkt.charge_time, p_params->charge_time);
    msg_pkt.flags_presence = p_params->flags_presence;
    msg_pkt.flags_indicator = p_params->flags_indicator;
    msg_pkt.flags_charging = p_params->flags_charging;
    msg_pkt.flags_service = p_params->flags_service;

    access_message_tx_t reply =
    {
        .opcode = ACCESS_OPCODE_SIG(GENERIC_BATTERY_OPCODE_STATUS),
        .p_buffer = (const uint8_t *) &msg_pkt,
        .length = GENERIC_BATTERY_STATUS_LEN,
        .force_segmented = p_server->settings.force_segmented,
        .transmic_size = p_server->settings.transmic_size
    };

    if (p_message == NULL) {
        return access_model_publish(p_server->model_handle, &reply);
    } else {
        return access_model_reply(p_server->model_handle, p_message, &reply);
    }
}




static inline bool get_params_validate(const access_message_rx_t *p_rx_msg)
{
    return (p_rx_msg->length == 0);
}

static void handle_get(access_model_handle_t model_handle, const access_message_rx_t *p_rx_msg, void *p_args)
{
    generic_battery_server_t *p_server = (generic_battery_server_t *)p_args;
    generic_battery_status_params_t out_data = { 0 };

    if (get_params_validate(p_rx_msg)) {
        p_server->settings.p_callbacks->battery_cbs.get_cb(p_server, &p_rx_msg->meta_data, &out_data);
        (void)status_send(p_server, p_rx_msg, &out_data);
    }
}

static void periodic_publish_cb(access_model_handle_t handle, void *p_args)
{
    generic_battery_server_t *p_server = (generic_battery_server_t *)p_args;
    generic_battery_status_params_t out_data = { 0 };

    p_server->settings.p_callbacks->battery_cbs.get_cb(p_server, NULL, &out_data);
    (void)status_send(p_server, NULL, &out_data);
}


/** Opcode Handlers */

static const access_opcode_handler_t m_opcode_handlers[] =
{
    {ACCESS_OPCODE_SIG(GENERIC_BATTERY_OPCODE_GET), handle_get},
};


/** Interface functions */
uint32_t generic_battery_server_init(generic_battery_server_t *p_server, uint8_t element_index)
{
    if (p_server == NULL
            || p_server->settings.p_callbacks == NULL
            || p_server->settings.p_callbacks->battery_cbs.get_cb == NULL ) {
        return NRF_ERROR_NULL;
    }

    access_model_add_params_t init_params =
    {
        .model_id = ACCESS_MODEL_SIG(GENERIC_BATTERY_SERVER_MODEL_ID),
        .element_index =  element_index,
        .p_opcode_handlers = m_opcode_handlers,
        .opcode_count = ARRAY_SIZE(m_opcode_handlers),
        .p_args = p_server,
        .publish_timeout_cb = periodic_publish_cb
    };

    uint32_t status = access_model_add(&init_params, &p_server->model_handle);

    if (status == NRF_SUCCESS) {
        status = access_model_subscription_list_alloc(p_server->model_handle);
    }

    return status;
}


uint32_t generic_battery_server_status_publish(generic_battery_server_t *p_server, const generic_battery_status_params_t *p_params)
{
    if (p_server == NULL || p_params == NULL) {
        return NRF_ERROR_NULL;
    }

    return status_send(p_server, NULL, p_params);
}
