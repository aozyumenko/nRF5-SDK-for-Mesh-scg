/* (C) 2024, Alexander Ozumenko
 * St(u)dio of Computer Games (https://stdio.ru/)
 */

#include "time_client.h"
#include "time_common.h"
#include "time_messages.h"

#include "access.h"
#include "access_config.h"
 
#include "nrf5_sdk_log.h"




/* opcode Handlers */

static void time_status_handle(access_model_handle_t handle, const access_message_rx_t *p_rx_msg, void *p_args)
{
    time_client_t *p_client = (time_client_t *)p_args;
    time_status_param_t in_data = {0};

    if (p_rx_msg->length == sizeof(time_status_msg_pkt_t)) {
        const time_status_msg_pkt_t *p_msg_params_packed = (const time_status_msg_pkt_t *)p_rx_msg->p_data;

        in_data.TAI_seconds =
            (uint64_t)p_msg_params_packed->TAI_seconds[4] << 32 |
            (uint64_t)p_msg_params_packed->TAI_seconds[3] << 24 |
            (uint64_t)p_msg_params_packed->TAI_seconds[2] << 16 |
            (uint64_t)p_msg_params_packed->TAI_seconds[1] << 8 |
            (uint64_t)p_msg_params_packed->TAI_seconds[0];
        in_data.subsecond = p_msg_params_packed->subsecond;
        in_data.uncertainty = p_msg_params_packed->uncertainty;
        in_data.time_authority = p_msg_params_packed->time_authority != 0;
        in_data.tai_utc_delta = p_msg_params_packed->tai_utc_delta;
        in_data.time_zone_offset = p_msg_params_packed->time_zone_offset;

        p_client->settings.p_callbacks->time_status_cb(p_client, &p_rx_msg->meta_data, &in_data);
    }
}


static void time_role_status_handle(access_model_handle_t handle, const access_message_rx_t *p_rx_msg, void *p_args)
{
    time_client_t *p_client = (time_client_t *)p_args;
    time_role_status_param_t in_data = {0};

    if (p_rx_msg->length == sizeof(time_role_status_msg_pkt_t)) {
        const time_role_status_msg_pkt_t *p_msg_params_packed = (const time_role_status_msg_pkt_t *)p_rx_msg->p_data;
        in_data.time_role = p_msg_params_packed->time_role;

        p_client->settings.p_callbacks->time_role_status_cb(p_client, &p_rx_msg->meta_data, &in_data);
    }
}


static void time_zone_status_handle(access_model_handle_t handle, const access_message_rx_t *p_rx_msg, void *p_args)
{
    /* do nothing */
}


static void tai_utc_delta_status_handle(access_model_handle_t handle, const access_message_rx_t *p_rx_msg, void *p_args)
{
    /* do nothing */
}


static const access_opcode_handler_t m_opcode_handlers[] =
{
    {ACCESS_OPCODE_SIG(TIME_OPCODE_TIME_STATUS), time_status_handle},
    {ACCESS_OPCODE_SIG(TIME_OPCODE_TIME_ROLE_STATUS), time_role_status_handle},
    {ACCESS_OPCODE_SIG(TIME_OPCODE_TIME_ZONE_STATUS), time_zone_status_handle},
    {ACCESS_OPCODE_SIG(TIME_OPCODE_TAI_UTC_DELTA_STATUS), tai_utc_delta_status_handle},
};


static void message_create(time_client_t *p_client, uint16_t tx_opcode,
                           const uint8_t *p_buffer, uint16_t length,
                           access_message_tx_t *p_message)
{
    p_message->opcode.opcode = tx_opcode;
    p_message->opcode.company_id = ACCESS_COMPANY_ID_NONE;
    p_message->p_buffer = p_buffer;
    p_message->length = length;
    p_message->force_segmented = p_client->settings.force_segmented;
    p_message->transmic_size = p_client->settings.transmic_size;
    p_message->access_token = nrf_mesh_unique_token_get();
}

static void reliable_context_create(time_client_t *p_client, uint16_t reply_opcode,
                                    access_reliable_t * p_reliable)
{
    p_reliable->model_handle = p_client->model_handle;
    p_reliable->reply_opcode.opcode = reply_opcode;
    p_reliable->reply_opcode.company_id = ACCESS_COMPANY_ID_NONE;
    p_reliable->timeout = p_client->settings.timeout;
    p_reliable->status_cb = p_client->settings.p_callbacks->ack_transaction_status_cb;
}



/* interface functions */

uint32_t time_client_init(time_client_t *p_client, uint8_t element_index)
{
    if (p_client == NULL ||
        p_client->settings.p_callbacks == NULL ||
        p_client->settings.p_callbacks->time_status_cb == NULL ||
        p_client->settings.p_callbacks->time_role_status_cb == NULL ||
        p_client->settings.p_callbacks->time_zone_status_cb == NULL ||
        p_client->settings.p_callbacks->tai_utc_delta_cb == NULL ||
        p_client->settings.p_callbacks->periodic_publish_cb == NULL) {
        return NRF_ERROR_NULL;
    }


    if (p_client->settings.timeout == 0) {
        p_client->settings.timeout = MODEL_ACKNOWLEDGED_TRANSACTION_TIMEOUT;
    }

    access_model_add_params_t add_params =
    {
        .model_id = ACCESS_MODEL_SIG(TIME_CLIENT_MODEL_ID),
        .element_index = element_index,
        .p_opcode_handlers = m_opcode_handlers,
        .opcode_count = ARRAY_SIZE(m_opcode_handlers),
        .p_args = p_client,
        .publish_timeout_cb = p_client->settings.p_callbacks->periodic_publish_cb
    };

    uint32_t status = access_model_add(&add_params, &p_client->model_handle);

    if (status == NRF_SUCCESS) {
        status = access_model_subscription_list_alloc(p_client->model_handle);
    }

    return status;
}


uint32_t time_client_get(time_client_t *p_client)
{
    if (p_client == NULL)     {
        return NRF_ERROR_NULL;
    }

    if (access_reliable_model_is_free(p_client->model_handle)) {
        message_create(p_client, TIME_OPCODE_TIME_GET, NULL, 0, &p_client->access_message.message);
        reliable_context_create(p_client, TIME_OPCODE_TIME_STATUS, &p_client->access_message);

        return access_model_reliable_publish(&p_client->access_message);
    } else {
        return NRF_ERROR_BUSY;
    }
}
