#include "light_hsl_setup_server.h"

#include <stdint.h>
#include "light_hsl_messages.h"
#include "light_hsl_utils.h"

#include "mesh_config_entry.h"
#include "mesh_config.h"

#include "access.h"
#include "access_config.h"
#include "nrf_mesh_utils.h"
#include "nordic_common.h"

#include "nrf5_sdk_log.h"



typedef enum {
    HSL_SERVER,
    HSL_SETUP_SERVER,
} server_context_type_t;



static uint32_t m_total_hsl_ss_instances = 0;



/* forward declarations */
static void hue_level_state_publish(const light_hsl_setup_server_t *p_s_server,
                                    const light_hsl_hue_status_params_t *p_params);

static void saturation_level_state_publish(const light_hsl_setup_server_t *p_s_server,
                                           const light_hsl_saturation_status_params_t *p_params);



static uint32_t status_send(const light_hsl_server_t *p_server,
                            const access_message_rx_t *p_message,
                            const light_hsl_status_params_t *p_params)
{
    light_hsl_status_msg_pkt_t msg_pkt;

    msg_pkt.lightness = p_params->present_lightness;
    msg_pkt.hue = p_params->present_hue;
    msg_pkt.saturation = p_params->present_saturation;
    if (p_params->remaining_time_ms > 0) {
        msg_pkt.remaining_time = model_transition_time_encode(p_params->remaining_time_ms);
    }

    access_message_tx_t reply = {
        .opcode = ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_STATUS),
        .p_buffer = (const uint8_t *)&msg_pkt,
        .length = (p_params->remaining_time_ms > 0 ?
                   LIGHT_HSL_STATUS_MAXLEN :
                   LIGHT_HSL_STATUS_MINLEN),
        .force_segmented = p_server->settings.force_segmented,
        .transmic_size = p_server->settings.transmic_size
    };

    if (p_message == NULL) {
        return access_model_publish(p_server->model_handle, &reply);
    } else {
        return access_model_reply(p_server->model_handle, p_message, &reply);
    }
}


static uint32_t status_target_send(const light_hsl_server_t *p_server,
                                   const access_message_rx_t *p_message,
                                   const light_hsl_status_params_t *p_params)
{
    light_hsl_target_status_msg_pkt_t msg_pkt;

    msg_pkt.lightness = p_params->target_lightness;
    msg_pkt.hue = p_params->target_hue;
    msg_pkt.saturation = p_params->target_saturation;
    if (p_params->remaining_time_ms > 0) {
        msg_pkt.remaining_time = model_transition_time_encode(p_params->remaining_time_ms);
    }

    access_message_tx_t reply = {
        .opcode = ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_TARGET_STATUS),
        .p_buffer = (const uint8_t *)&msg_pkt,
        .length = (p_params->remaining_time_ms > 0 ?
                   LIGHT_HSL_TARGET_STATUS_MAXLEN :
                   LIGHT_HSL_TARGET_STATUS_MINLEN),
        .force_segmented = p_server->settings.force_segmented,
        .transmic_size = p_server->settings.transmic_size
    };

    if (p_message == NULL) {
        return access_model_publish(p_server->model_handle, &reply);
    } else {
        return access_model_reply(p_server->model_handle, p_message, &reply);
    }
}


static uint32_t status_default_send(const void *p_ctx,
                                    server_context_type_t ctx_type,
                                    const access_message_rx_t *p_message,
                                    const light_hsl_default_status_params_t *p_params)
{
    light_hsl_default_status_msg_pkt_t msg_pkt;
    uint16_t model_handle;
    bool force_segmented;
    nrf_mesh_transmic_size_t transmic_size;

    if (ctx_type == HSL_SERVER) {
        light_hsl_server_t *p_server = (light_hsl_server_t *)p_ctx;
        model_handle = p_server->model_handle;
        force_segmented = p_server->settings.force_segmented;
        transmic_size = p_server->settings.transmic_size;
    } else {
        light_hsl_setup_server_t *p_s_server = (light_hsl_setup_server_t *)p_ctx;
        model_handle = p_s_server->model_handle;
        force_segmented = p_s_server->settings.force_segmented;
        transmic_size = p_s_server->settings.transmic_size;
    }

    msg_pkt.lightness = p_params->lightness;
    msg_pkt.hue = p_params->hue;
    msg_pkt.saturation = p_params->saturation;

    access_message_tx_t reply = {
        .opcode = ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_DEFAULT_STATUS),
        .p_buffer = (const uint8_t *)&msg_pkt,
        .length = LIGHT_HSL_DEFAULT_STATUS_LEN,
        .force_segmented = force_segmented,
        .transmic_size = transmic_size
    };

    if (p_message == NULL) {
        return access_model_publish(model_handle, &reply);
    } else {
        return access_model_reply(model_handle, p_message, &reply);
    }
}


static uint32_t status_range_send(const void *p_ctx,
                                  server_context_type_t ctx_type,
                                  const access_message_rx_t *p_message,
                                  const light_hsl_range_status_params_t *p_params)
{
    light_hsl_range_status_msg_pkt_t msg_pkt;
    uint16_t model_handle;
    bool force_segmented;
    nrf_mesh_transmic_size_t transmic_size;

    if (ctx_type == HSL_SERVER) {
        light_hsl_server_t *p_server = (light_hsl_server_t *)p_ctx;
        model_handle = p_server->model_handle;
        force_segmented = p_server->settings.force_segmented;
        transmic_size = p_server->settings.transmic_size;
    } else {
        light_hsl_setup_server_t *p_s_server = (light_hsl_setup_server_t *)p_ctx;
        model_handle = p_s_server->model_handle;
        force_segmented = p_s_server->settings.force_segmented;
        transmic_size = p_s_server->settings.transmic_size;
    }

    msg_pkt.status_code = p_params->status_code;
    msg_pkt.hue_range_min = p_params->hue_range_min;
    msg_pkt.hue_range_max = p_params->hue_range_max;
    msg_pkt.saturation_range_min = p_params->saturation_range_min;
    msg_pkt.saturation_range_max = p_params->saturation_range_max;

    access_message_tx_t reply = {
        .opcode = ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_RANGE_STATUS),
        .p_buffer = (const uint8_t *) &msg_pkt,
        .length = LIGHT_HSL_RANGE_STATUS_LEN,
        .force_segmented = force_segmented,
        .transmic_size = transmic_size
    };

    if (p_message == NULL) {
        return access_model_publish(model_handle, &reply);
    } else {
        return access_model_reply(model_handle, p_message, &reply);
    }
}



static void periodic_publish_cb(access_model_handle_t handle, void *p_args)
{
    light_hsl_server_t *p_server = (light_hsl_server_t *)p_args;
    light_hsl_setup_server_t *p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_srv, p_args);
    light_hsl_status_params_t out_data = {0};

    p_s_server->settings.p_callbacks->light_hsl_cbs.get_cb(p_s_server, NULL, &out_data);
    (void) status_send(p_server, NULL, &out_data);
}



/* Utility functions */
static uint16_t clip_hsl_hue_to_range(const light_hsl_setup_server_t *p_s_server,
                                      const access_message_rx_meta_t *p_meta,
                                      uint16_t hue)
{
    light_hsl_range_status_params_t hsl_range;

    p_s_server->settings.p_callbacks->light_hsl_cbs.range_get_cb(p_s_server, p_meta, &hsl_range);
    hue = light_hsl_utils_param_range_restrict(hue,
                                               hsl_range.hue_range_min,
                                               hsl_range.hue_range_max);
    return hue;
}


static uint16_t clip_hsl_saturation_to_range(const light_hsl_setup_server_t *p_s_server,
                                             const access_message_rx_meta_t *p_meta,
                                             uint16_t saturation)
{
    light_hsl_range_status_params_t hsl_range;

    p_s_server->settings.p_callbacks->light_hsl_cbs.range_get_cb(p_s_server, p_meta, &hsl_range);

    saturation = light_hsl_utils_param_range_restrict(saturation,
                                                      hsl_range.saturation_range_min,
                                                      hsl_range.saturation_range_max);
    return saturation;
}


static void clip_hsl_to_range(const light_hsl_setup_server_t *p_s_server,
                              const access_message_rx_meta_t *p_meta,
                              light_hsl_set_params_t *p_params)
{
    light_hsl_range_status_params_t hsl_range;

    p_s_server->settings.p_callbacks->light_hsl_cbs.range_get_cb(p_s_server, p_meta, &hsl_range);

    p_params->hue = light_hsl_utils_param_range_restrict(p_params->hue,
                                                         hsl_range.hue_range_min,
                                                         hsl_range.hue_range_max);
    p_params->saturation = light_hsl_utils_param_range_restrict(p_params->saturation,
                                                                hsl_range.saturation_range_min,
                                                                hsl_range.saturation_range_max);
}


static bool handle_set_message_validate(uint16_t short_message_bytes,
                                        uint16_t long_message_bytes,
                                        uint16_t message_bytes,
                                        uint8_t transition_time,
                                        uint8_t delay,
                                        model_transition_t **pp_transition)
{
    model_transition_t *p_transition = *pp_transition;

    if (short_message_bytes == message_bytes) {
        /* message does not specify a transition */
        *pp_transition = NULL;
        return true;
    }

    if (long_message_bytes == message_bytes) {
        /* the message specifies a transition */
        if (!model_transition_time_is_valid(transition_time)) {
            NRF_LOG_ERROR("%s(): invalid parameter (%d) = (transition_time)",
                          __func__, transition_time);
            return false;
        }
        p_transition->transition_time_ms = model_transition_time_decode(transition_time);
        p_transition->delay_ms = model_delay_decode(delay);
        return true;
    }

    /* message length is invalid */
    NRF_LOG_ERROR("%s(): invalid parameter (%d) = (message_bytes)",
                  __func__, message_bytes);

    return false;
}



/*** HSL Server */

/** opcode handlers */

static void handle_get(access_model_handle_t model_handle,
                       const access_message_rx_t *p_rx_msg, void *p_args)
{
    light_hsl_server_t *p_server = (light_hsl_server_t *)p_args;
    light_hsl_setup_server_t *p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_srv, p_args);
    light_hsl_status_params_t out_data = {0};

    if (p_rx_msg->length == 0) {
        p_s_server->settings.p_callbacks->light_hsl_cbs.get_cb(p_s_server,
                                                               &p_rx_msg->meta_data,
                                                               &out_data);
        (void) status_send(p_server, p_rx_msg, &out_data);
    }
}


static void handle_set(access_model_handle_t model_handle,
                       const access_message_rx_t *p_rx_msg,
                       void *p_args)
{
    light_hsl_server_t *p_server = (light_hsl_server_t *)p_args;
    light_hsl_setup_server_t *p_s_server;
    light_hsl_set_msg_pkt_t *p_msg_pkt = (light_hsl_set_msg_pkt_t *)p_rx_msg->p_data;
    light_hsl_status_params_t out_data = {0};
    light_hsl_status_params_t *p_out_data = NULL;
    light_hsl_set_params_t in_data = {0};
    model_transition_t transition = {0};
    model_transition_t *p_transition = &transition;

    if (!model_tid_validate(&p_server->tid_tracker, &p_rx_msg->meta_data,
                            LIGHT_HSL_OPCODE_SET, p_msg_pkt->tid)) {
        return;
    }

    if (!handle_set_message_validate(LIGHT_HSL_SET_MINLEN,
                                     LIGHT_HSL_SET_MAXLEN,
                                     p_rx_msg->length,
                                     p_msg_pkt->transition_time,
                                     p_msg_pkt->delay,
                                     &p_transition)) {
        return;
    }

    in_data.lightness = p_msg_pkt->lightness;
    in_data.hue = p_msg_pkt->hue;
    in_data.saturation = p_msg_pkt->saturation;
    in_data.tid = p_msg_pkt->tid;

    /* clip hue and saturation to range */
    p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_srv, p_server);
    clip_hsl_to_range(p_s_server, &p_rx_msg->meta_data, &in_data);

    if (p_rx_msg->opcode.opcode == LIGHT_HSL_OPCODE_SET) {
        p_out_data = &out_data;
    }

    p_s_server->settings.p_callbacks->light_hsl_cbs.set_cb(p_s_server, &p_rx_msg->meta_data, &in_data,
                                                           p_transition, p_out_data);

    if (p_out_data != NULL) {
        (void) status_send(p_server, p_rx_msg, p_out_data);
    }
}


static void handle_target_get(access_model_handle_t model_handle,
                              const access_message_rx_t *p_rx_msg,
                              void *p_args)
{
    light_hsl_server_t *p_server = (light_hsl_server_t *)p_args;
    light_hsl_setup_server_t *p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_srv, p_args);
    light_hsl_status_params_t out_data = {0};

    if (p_rx_msg->length == 0) {
        p_s_server->settings.p_callbacks->light_hsl_cbs.get_cb(p_s_server,
                                                               &p_rx_msg->meta_data,
                                                               &out_data);
        (void) status_target_send(p_server, p_rx_msg, &out_data);
    }
}


static void handle_default_get(access_model_handle_t model_handle,
                               const access_message_rx_t *p_rx_msg,
                               void *p_args)
{
    light_hsl_server_t *p_server = (light_hsl_server_t *)p_args;
    light_hsl_setup_server_t *p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_srv, p_args);
    light_hsl_default_status_params_t out_data = {0};

    if (p_rx_msg->length == 0) {
        p_s_server->settings.p_callbacks->light_hsl_cbs.default_get_cb(p_s_server,
                                                                       &p_rx_msg->meta_data,
                                                                       &out_data);
        (void) status_default_send(p_server, HSL_SERVER, p_rx_msg, &out_data);
    }
}


static void handle_range_get(access_model_handle_t model_handle,
                             const access_message_rx_t *p_rx_msg, void *p_args)
{
    light_hsl_server_t *p_server = (light_hsl_server_t *)p_args;
    light_hsl_setup_server_t *p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_srv, p_args);
    light_hsl_range_status_params_t out_data = {0};

    if (p_rx_msg->length == 0) {
        p_s_server->settings.p_callbacks->light_hsl_cbs.range_get_cb(p_s_server,
                                                                     &p_rx_msg->meta_data,
                                                                     &out_data);
        (void)status_range_send(p_server, HSL_SERVER, p_rx_msg, &out_data);
    }
}


static const access_opcode_handler_t m_opcode_handlers[] =
{
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_GET), handle_get},
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_SET), handle_set},
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_SET_UNACKNOWLEDGED), handle_set},
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_TARGET_GET), handle_target_get},
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_RANGE_GET), handle_range_get},
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_DEFAULT_GET), handle_default_get}
};



static uint32_t hsl_server_init(light_hsl_server_t *p_server, uint8_t element_index)
{
    uint32_t status;

    if (p_server == NULL) {
        return NRF_ERROR_NULL;
    }

    /* add this HSL Server model to the mesh */
    access_model_add_params_t init_params = {
        .model_id = ACCESS_MODEL_SIG(LIGHT_HSL_SERVER_MODEL_ID),
        .element_index =  element_index,
        .p_opcode_handlers = m_opcode_handlers,
        .opcode_count = ARRAY_SIZE(m_opcode_handlers),
        .p_args = p_server,
        .publish_timeout_cb = periodic_publish_cb
    };

    status = access_model_add(&init_params, &p_server->model_handle);

    return status;
}



/*** HSL hue Server */

/** Hue Status sender */
static uint32_t status_hue_send(const light_hsl_hue_server_t *p_server,
                                const access_message_rx_t *p_message,
                                const light_hsl_hue_status_params_t *p_params)
{
    light_hsl_hue_status_msg_pkt_t msg_pkt;

    msg_pkt.present_hue = p_params->present_hue;
    if (p_params->remaining_time_ms > 0) {
        msg_pkt.target_hue = p_params->target_hue;
        msg_pkt.remaining_time = model_transition_time_encode(p_params->remaining_time_ms);
    }

    access_message_tx_t reply = {
        .opcode = ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_HUE_STATUS),
        .p_buffer = (const uint8_t *)&msg_pkt,
        .length = (p_params->remaining_time_ms > 0 ?
                   LIGHT_HSL_HUE_STATUS_MAXLEN :
                   LIGHT_HSL_HUE_STATUS_MINLEN),
        .force_segmented = p_server->settings.force_segmented,
        .transmic_size = p_server->settings.transmic_size
    };

    if (p_message == NULL) {
        return access_model_publish(p_server->model_handle, &reply);
    } else {
        return access_model_reply(p_server->model_handle, p_message, &reply);
    }
}


static void periodic_hue_publish_cb(access_model_handle_t handle, void *p_args)
{
    light_hsl_hue_server_t *p_server = (light_hsl_hue_server_t *)p_args;
    light_hsl_setup_server_t *p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_hue_srv, p_args);
    light_hsl_hue_status_params_t out_data;

    p_s_server->settings.p_callbacks->light_hsl_cbs.hue_get_cb(p_s_server, NULL, &out_data);
    (void) status_hue_send(p_server, NULL, &out_data);
}



/** Opcode Handlers */

static void handle_hue_get(access_model_handle_t model_handle,
                           const access_message_rx_t *p_rx_msg,
                           void *p_args)
{
    light_hsl_hue_server_t *p_server = (light_hsl_hue_server_t *)p_args;
    light_hsl_setup_server_t *p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t,
                                                               hsl_hue_srv,
                                                               p_args);
    light_hsl_hue_status_params_t out_data = {0};

    if (p_rx_msg->length == 0) {
        p_s_server->settings.p_callbacks->light_hsl_cbs.hue_get_cb(p_s_server,
                                                                   &p_rx_msg->meta_data,
                                                                   &out_data);
        (void) status_hue_send(p_server, p_rx_msg, &out_data);
    }
}


static void handle_hue_set(access_model_handle_t model_handle,
                           const access_message_rx_t *p_rx_msg, void *p_args)
{
    light_hsl_hue_server_t *p_server = (light_hsl_hue_server_t *)p_args;
    light_hsl_setup_server_t *p_s_server;
    light_hsl_hue_set_msg_pkt_t *p_msg_pkt = (light_hsl_hue_set_msg_pkt_t *)p_rx_msg->p_data;
    light_hsl_hue_status_params_t out_data = {0};
    light_hsl_hue_status_params_t *p_out_data = NULL;
    light_hsl_hue_set_params_t in_data = {0};
    model_transition_t transition = {0};
    model_transition_t *p_transition = &transition;

    if (!model_tid_validate(&p_server->tid_tracker, &p_rx_msg->meta_data,
                            LIGHT_HSL_OPCODE_HUE_SET, p_msg_pkt->tid)) {
        return;
    }

    if (!handle_set_message_validate(LIGHT_HSL_HUE_SET_MINLEN,
                                     LIGHT_HSL_HUE_SET_MAXLEN,
                                     p_rx_msg->length,
                                     p_msg_pkt->transition_time,
                                     p_msg_pkt->delay,
                                     &p_transition)) {
        return;
    }

    in_data.hue = p_msg_pkt->hue;
    in_data.tid = p_msg_pkt->tid;

    /* clip hue to range */
    p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_hue_srv, p_server);
    in_data.hue = clip_hsl_hue_to_range(p_s_server, &p_rx_msg->meta_data, in_data.hue);

    if (p_rx_msg->opcode.opcode == LIGHT_HSL_OPCODE_HUE_SET) {
        p_out_data = &out_data;
    }

    p_s_server->settings.p_callbacks->light_hsl_cbs.hue_set_cb(p_s_server, &p_rx_msg->meta_data,
                                                               &in_data, p_transition, p_out_data);

    if (p_out_data != NULL) {
        (void) status_hue_send(p_server, p_rx_msg, p_out_data);
    }
}



static const access_opcode_handler_t m_opcode_handlers_hue[] =
{
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_HUE_GET), handle_hue_get},
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_HUE_SET), handle_hue_set},
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_HUE_SET_UNACKNOWLEDGED), handle_hue_set}
};



/** callbacks for the model that HSL Hue instantiated (extended) **/

static void hue_level_state_get_cb(const generic_level_server_t *p_self,
                                   const access_message_rx_meta_t *p_meta,
                                   generic_level_status_params_t *p_out)
{
    light_hsl_hue_server_t *p_server = PARENT_BY_FIELD_GET(light_hsl_hue_server_t,
                                                           generic_level_srv, p_self);
    light_hsl_setup_server_t *p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t,
                                                               hsl_hue_srv, p_server);
    light_hsl_hue_status_params_t out_data = {0};

    p_s_server->settings.p_callbacks->light_hsl_cbs.hue_get_cb(p_s_server, p_meta, &out_data);

    p_out->present_level = light_hsl_utils_param_to_generic_level(out_data.present_hue);
    p_out->target_level = light_hsl_utils_param_to_generic_level(out_data.target_hue);
    p_out->remaining_time_ms = out_data.remaining_time_ms;
}


static void hue_level_state_set_cb(const generic_level_server_t *p_self,
                                   const access_message_rx_meta_t *p_meta,
                                   const generic_level_set_params_t *p_in,
                                   const model_transition_t *p_in_transition,
                                   generic_level_status_params_t *p_out)
{
    generic_level_server_t *p_level_server  = (generic_level_server_t *)p_self;
    light_hsl_hue_server_t *p_server =
        PARENT_BY_FIELD_GET(light_hsl_hue_server_t, generic_level_srv, p_level_server);
    light_hsl_setup_server_t *p_s_server =
        PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_hue_srv, p_server);
    light_hsl_hue_set_params_t in_data = {0};
    light_hsl_hue_status_params_t out_data = {0};
    light_hsl_hue_status_params_t *p_out_hue_data;



    /* TID already validated by level model, so copy results into our tid_tracker */
    in_data.tid = p_in->tid;
    p_server->tid_tracker = p_level_server->tid_tracker;

    /* convert the Generic Level data to HSL hue state */
    in_data.hue = light_hsl_utils_generic_level_to_param(p_in->level);

    /* clip hue value to range (binding - 6.1.4.1.3) */
    in_data.hue = clip_hsl_hue_to_range(p_s_server, p_meta, in_data.hue);

    p_out_hue_data = p_out != NULL ? &out_data : NULL;

    /* set hue value */
    p_s_server->settings.p_callbacks->light_hsl_cbs.hue_set_cb(p_s_server, p_meta, &in_data,
                                                               p_in_transition, p_out_hue_data);

    if (p_out != NULL && p_out_hue_data != NULL) {
        p_out->present_level =
            light_hsl_utils_param_to_generic_level(p_out_hue_data->present_hue);
        p_out->target_level =
            light_hsl_utils_param_to_generic_level(p_out_hue_data->target_hue);
        p_out->remaining_time_ms = p_out_hue_data->remaining_time_ms;
    }
}


static void hue_level_state_delta_set_cb(const generic_level_server_t *p_self,
                                         const access_message_rx_meta_t *p_meta,
                                         const generic_level_delta_set_params_t *p_in,
                                         const model_transition_t *p_in_transition,
                                         generic_level_status_params_t *p_out)
{
    generic_level_server_t *p_level_server  = (generic_level_server_t *)p_self;
    light_hsl_hue_server_t *p_server =
        PARENT_BY_FIELD_GET(light_hsl_hue_server_t, generic_level_srv, p_level_server);
    light_hsl_setup_server_t *p_s_server =
        PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_hue_srv, p_server);
    light_hsl_delta_set_params_t in_data = {0};
    light_hsl_hue_status_params_t out_data = {0};
    light_hsl_hue_status_params_t *p_out_hue_data;


    in_data.delta = p_in->delta_level;

    /* TID already validated by level model, so copy results into our tid_tracker */
    in_data.tid = p_in->tid;
    p_server->tid_tracker = p_level_server->tid_tracker;

    p_out_hue_data = p_out ? &out_data : NULL;
    p_s_server->settings.p_callbacks->light_hsl_cbs.hue_delta_set_cb(p_s_server,
                                                                     p_meta,
                                                                     &in_data,
                                                                     p_in_transition,
                                                                     p_out_hue_data);
    if (p_out != NULL && p_out_hue_data != NULL) {
        p_out->present_level =
            light_hsl_utils_param_to_generic_level(p_out_hue_data->present_hue);
        p_out->target_level =
            light_hsl_utils_param_to_generic_level(p_out_hue_data->target_hue);
        p_out->remaining_time_ms = p_out_hue_data->remaining_time_ms;
    }
}


static void hue_level_state_move_set_cb(const generic_level_server_t *p_self,
                                        const access_message_rx_meta_t *p_meta,
                                        const generic_level_move_set_params_t *p_in,
                                        const model_transition_t *p_in_transition,
                                        generic_level_status_params_t *p_out)
{
    generic_level_server_t *p_level_server  = (generic_level_server_t *)p_self;
    light_hsl_hue_server_t *p_server = PARENT_BY_FIELD_GET(light_hsl_hue_server_t,
                                                           generic_level_srv, p_level_server);
    light_hsl_setup_server_t *p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_hue_srv,
                                                               p_server);
    light_hsl_move_set_params_t in_data = {0};
    light_hsl_hue_status_params_t out_data = {0};
    light_hsl_hue_status_params_t *p_out_data;

    in_data.delta = p_in->move_level;

    /* TID already validated by level model, so copy results into our tid_tracker */
    in_data.tid = p_in->tid;
    p_server->tid_tracker = p_level_server->tid_tracker;

    p_out_data = p_out ? &out_data : NULL;
    p_s_server->settings.p_callbacks->light_hsl_cbs.hue_move_set_cb(p_s_server, p_meta, &in_data,
                                                                    p_in_transition, p_out_data);
    if ((p_out != NULL) && (p_out_data != NULL)) {
        p_out->present_level =
            light_hsl_utils_param_to_generic_level(p_out_data->present_hue);
        /* set target level as expected in Move Set status, see @tagMeshMdlSp section 3.3.2.2.4 */
        p_out->target_level = p_out_data->target_hue == UINT16_MAX ? INT16_MAX : INT16_MIN;
        p_out->remaining_time_ms = p_out_data->remaining_time_ms;
    }
}


/** Interface functions */
static const generic_level_server_callbacks_t m_hue_level_srv_cbs =
{
    .level_cbs.get_cb = hue_level_state_get_cb,
    .level_cbs.set_cb = hue_level_state_set_cb,
    .level_cbs.delta_set_cb = hue_level_state_delta_set_cb,
    .level_cbs.move_set_cb = hue_level_state_move_set_cb
};


/* when hue has changed in the Light HSL state, when we also need to publish level */
static void hue_level_state_publish(const light_hsl_setup_server_t *p_s_server,
                                    const light_hsl_hue_status_params_t *p_params)

{
    generic_level_server_t *p_level_server;
    generic_level_status_params_t pub_params;

    p_level_server = (generic_level_server_t *)&p_s_server->hsl_hue_srv.generic_level_srv;

    pub_params.present_level = light_hsl_utils_param_to_generic_level(p_params->present_hue);
    pub_params.target_level = light_hsl_utils_param_to_generic_level(p_params->target_hue);
    pub_params.remaining_time_ms = p_params->remaining_time_ms;

    (void) generic_level_server_status_publish(p_level_server, &pub_params);
}



static uint32_t hsl_hue_server_init(light_hsl_hue_server_t *p_server, uint8_t element_index)
{
    uint32_t status;

    if (p_server == NULL) {
        return NRF_ERROR_NULL;
    }

    /* initialize parent model instance - Generic Level */
    p_server->generic_level_srv.settings.p_callbacks = &m_hue_level_srv_cbs;

    status = generic_level_server_init(&p_server->generic_level_srv, element_index);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* add this HSL Hue server model to the mesh */
    access_model_add_params_t init_params = {
        .model_id = ACCESS_MODEL_SIG(LIGHT_HSL_HUE_SERVER_MODEL_ID),
        .element_index =  element_index,
        .p_opcode_handlers = m_opcode_handlers_hue,
        .opcode_count = ARRAY_SIZE(m_opcode_handlers_hue),
        .p_args = p_server,
        .publish_timeout_cb = periodic_hue_publish_cb
    };

    status = access_model_add(&init_params, &p_server->model_handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* HSL Hue Server sets up its subscription list, then shares it with its extended level model */
    status = access_model_subscription_list_alloc(p_server->model_handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* deallocate the level model's subscription list so we can share with it */
    status = access_model_subscription_list_dealloc(p_server->generic_level_srv.model_handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* share the subscription list with the level model */
    status = access_model_subscription_lists_share(p_server->model_handle,
                                                   p_server->generic_level_srv.model_handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

    return status;
}



/*** HSL saturation Server */

/** Saturation Status Sender */
static uint32_t status_saturation_send(const light_hsl_saturation_server_t *p_server,
                                       const access_message_rx_t *p_message,
                                       const light_hsl_saturation_status_params_t *p_params)
{
    light_hsl_saturation_status_msg_pkt_t msg_pkt;

    msg_pkt.present_saturation = p_params->present_saturation;
    if (p_params->remaining_time_ms > 0) {
        msg_pkt.target_saturation = p_params->target_saturation;
        msg_pkt.remaining_time = model_transition_time_encode(p_params->remaining_time_ms);
    }

    access_message_tx_t reply = {
        .opcode = ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_SATURATION_STATUS),
        .p_buffer = (const uint8_t *)&msg_pkt,
        .length = (p_params->remaining_time_ms > 0 ?
                   LIGHT_HSL_SATURATION_STATUS_MAXLEN :
                   LIGHT_HSL_SATURATION_STATUS_MINLEN),
        .force_segmented = p_server->settings.force_segmented,
        .transmic_size = p_server->settings.transmic_size
    };

    if (p_message == NULL) {
        return access_model_publish(p_server->model_handle, &reply);
    } else {
        return access_model_reply(p_server->model_handle, p_message, &reply);
    }
}


static void periodic_saturation_publish_cb(access_model_handle_t handle, void *p_args)
{
    light_hsl_saturation_server_t *p_server = (light_hsl_saturation_server_t *)p_args;
    light_hsl_setup_server_t *p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_saturation_srv, p_args);
    light_hsl_saturation_status_params_t out_data;

    p_s_server->settings.p_callbacks->light_hsl_cbs.saturation_get_cb(p_s_server, NULL, &out_data);
    (void) status_saturation_send(p_server, NULL, &out_data);
}


/** Opcode Handlers */

static void handle_saturation_get(access_model_handle_t model_handle,
                                  const access_message_rx_t *p_rx_msg,
                                  void *p_args)
{
    light_hsl_saturation_server_t *p_server = (light_hsl_saturation_server_t *)p_args;
    light_hsl_setup_server_t *p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t,
                                                               hsl_saturation_srv,
                                                               p_args);
    light_hsl_saturation_status_params_t out_data = {0};

    if (p_rx_msg->length == 0) {
        p_s_server->settings.p_callbacks->light_hsl_cbs.saturation_get_cb(p_s_server,
                                                                          &p_rx_msg->meta_data,
                                                                          &out_data);
        (void) status_saturation_send(p_server, p_rx_msg, &out_data);
    }
}


static void handle_saturation_set(access_model_handle_t model_handle,
                                  const access_message_rx_t *p_rx_msg, void *p_args)
{
    light_hsl_saturation_server_t *p_server = (light_hsl_saturation_server_t *)p_args;
    light_hsl_setup_server_t *p_s_server;
    light_hsl_saturation_set_msg_pkt_t *p_msg_pkt = (light_hsl_saturation_set_msg_pkt_t *)p_rx_msg->p_data;
    light_hsl_saturation_status_params_t out_data = {0};
    light_hsl_saturation_status_params_t *p_out_data = NULL;
    light_hsl_saturation_set_params_t in_data = {0};
    model_transition_t transition = {0};
    model_transition_t *p_transition = &transition;

    if (!model_tid_validate(&p_server->tid_tracker, &p_rx_msg->meta_data,
                            LIGHT_HSL_OPCODE_SATURATION_SET, p_msg_pkt->tid)) {
        return;
    }

    if (!handle_set_message_validate(LIGHT_HSL_SATURATION_SET_MINLEN,
                                     LIGHT_HSL_SATURATION_SET_MAXLEN,
                                     p_rx_msg->length,
                                     p_msg_pkt->transition_time,
                                     p_msg_pkt->delay,
                                     &p_transition)) {
        return;
    }

    in_data.saturation = p_msg_pkt->saturation;
    in_data.tid = p_msg_pkt->tid;

    /* clip saturation to range */
    p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_saturation_srv, p_server);
    in_data.saturation = clip_hsl_saturation_to_range(p_s_server, &p_rx_msg->meta_data, in_data.saturation);

    if (p_rx_msg->opcode.opcode == LIGHT_HSL_OPCODE_SATURATION_SET) {
        p_out_data = &out_data;
    }

    p_s_server->settings.p_callbacks->light_hsl_cbs.saturation_set_cb(p_s_server, &p_rx_msg->meta_data,
                                                                      &in_data, p_transition, p_out_data);

    if (p_out_data != NULL) {
        (void) status_saturation_send(p_server, p_rx_msg, p_out_data);
    }
}


static const access_opcode_handler_t m_opcode_handlers_saturation[] =
{
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_SATURATION_GET), handle_saturation_get},
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_SATURATION_SET), handle_saturation_set},
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_SATURATION_SET_UNACKNOWLEDGED), handle_saturation_set}
};



/** Callbacks for the model that HSL Saturation instantiated (extended) **/

static void saturation_level_state_get_cb(const generic_level_server_t *p_self,
                                          const access_message_rx_meta_t *p_meta,
                                          generic_level_status_params_t *p_out)
{
    light_hsl_saturation_server_t *p_server = PARENT_BY_FIELD_GET(light_hsl_saturation_server_t,
                                                                  generic_level_srv, p_self);
    light_hsl_setup_server_t *p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t,
                                                               hsl_saturation_srv, p_server);
    light_hsl_saturation_status_params_t out_data = {0};

    p_s_server->settings.p_callbacks->light_hsl_cbs.saturation_get_cb(p_s_server, p_meta, &out_data);

    p_out->present_level = light_hsl_utils_param_to_generic_level(out_data.present_saturation);
    p_out->target_level = light_hsl_utils_param_to_generic_level(out_data.target_saturation);
    p_out->remaining_time_ms = out_data.remaining_time_ms;
}


static void saturation_level_state_set_cb(const generic_level_server_t *p_self,
                                          const access_message_rx_meta_t *p_meta,
                                          const generic_level_set_params_t *p_in,
                                          const model_transition_t *p_in_transition,
                                          generic_level_status_params_t *p_out)
{
    generic_level_server_t *p_level_server  = (generic_level_server_t *)p_self;
    light_hsl_saturation_server_t *p_server =
        PARENT_BY_FIELD_GET(light_hsl_saturation_server_t, generic_level_srv, p_level_server);
    light_hsl_setup_server_t *p_s_server =
        PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_saturation_srv, p_server);
    light_hsl_saturation_set_params_t in_data = {0};
    light_hsl_saturation_status_params_t out_data = {0};
    light_hsl_saturation_status_params_t *p_out_saturation_data;


    /* TID already validated by level model, so copy results into our tid_tracker */
    in_data.tid = p_in->tid;
    p_server->tid_tracker = p_level_server->tid_tracker;

    /* convert the Generic Level data to HSL saturation state */
    in_data.saturation = light_hsl_utils_generic_level_to_param(p_in->level);

    /* clip saturation value to range (binding - 6.1.4.1.3) */
    in_data.saturation = clip_hsl_saturation_to_range(p_s_server, p_meta, in_data.saturation);

    p_out_saturation_data = p_out != NULL ? &out_data : NULL;

    /* set saturation value */
    p_s_server->settings.p_callbacks->light_hsl_cbs.saturation_set_cb(p_s_server, p_meta, &in_data,
                                                                      p_in_transition,
                                                                      p_out_saturation_data);

    if (p_out != NULL && p_out_saturation_data != NULL) {
        p_out->present_level =
            light_hsl_utils_param_to_generic_level(p_out_saturation_data->present_saturation);
        p_out->target_level =
            light_hsl_utils_param_to_generic_level(p_out_saturation_data->target_saturation);
        p_out->remaining_time_ms = p_out_saturation_data->remaining_time_ms;
    }
}


static void saturation_level_state_delta_set_cb(const generic_level_server_t *p_self,
                                                const access_message_rx_meta_t *p_meta,
                                                const generic_level_delta_set_params_t *p_in,
                                                const model_transition_t *p_in_transition,
                                                generic_level_status_params_t *p_out)
{
    generic_level_server_t *p_level_server  = (generic_level_server_t *)p_self;
    light_hsl_saturation_server_t *p_server =
        PARENT_BY_FIELD_GET(light_hsl_saturation_server_t, generic_level_srv, p_level_server);
    light_hsl_setup_server_t *p_s_server =
        PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_saturation_srv, p_server);
    light_hsl_delta_set_params_t in_data = {0};
    light_hsl_saturation_status_params_t out_data = {0};
    light_hsl_saturation_status_params_t *p_out_saturation_data;


    in_data.delta = p_in->delta_level;

    /* TID already validated by level model, so copy results into our tid_tracker */
    in_data.tid = p_in->tid;
    p_server->tid_tracker = p_level_server->tid_tracker;

    p_out_saturation_data = p_out ? &out_data : NULL;
    p_s_server->settings.p_callbacks->light_hsl_cbs.saturation_delta_set_cb(p_s_server,
                                                                            p_meta,
                                                                            &in_data,
                                                                            p_in_transition,
                                                                            p_out_saturation_data);
    if (p_out != NULL && p_out_saturation_data != NULL) {
        p_out->present_level =
            light_hsl_utils_param_to_generic_level(p_out_saturation_data->present_saturation);
        p_out->target_level =
            light_hsl_utils_param_to_generic_level(p_out_saturation_data->target_saturation);
        p_out->remaining_time_ms = p_out_saturation_data->remaining_time_ms;
    }
}


static void saturation_level_state_move_set_cb(const generic_level_server_t *p_self,
                                               const access_message_rx_meta_t *p_meta,
                                               const generic_level_move_set_params_t *p_in,
                                               const model_transition_t *p_in_transition,
                                               generic_level_status_params_t *p_out)
{
    generic_level_server_t *p_level_server  = (generic_level_server_t *)p_self;
    light_hsl_saturation_server_t *p_server = PARENT_BY_FIELD_GET(light_hsl_saturation_server_t,
                                                           generic_level_srv, p_level_server);
    light_hsl_setup_server_t *p_s_server = PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_saturation_srv,
                                                               p_server);
    light_hsl_move_set_params_t in_data = {0};
    light_hsl_saturation_status_params_t out_data = {0};
    light_hsl_saturation_status_params_t *p_out_data;

    in_data.delta = p_in->move_level;

    /* TID already validated by level model, so copy results into our tid_tracker */
    in_data.tid = p_in->tid;
    p_server->tid_tracker = p_level_server->tid_tracker;

    p_out_data = p_out ? &out_data : NULL;
    p_s_server->settings.p_callbacks->light_hsl_cbs.saturation_move_set_cb(p_s_server, p_meta, &in_data,
                                                                           p_in_transition, p_out_data);
    if ((p_out != NULL) && (p_out_data != NULL)) {
        p_out->present_level =
            light_hsl_utils_param_to_generic_level(p_out_data->present_saturation);
        /* set target level as expected in Move Set status, see @tagMeshMdlSp section 3.3.2.2.4 */
        p_out->target_level = p_out_data->target_saturation == UINT16_MAX ? INT16_MAX : INT16_MIN;
        p_out->remaining_time_ms = p_out_data->remaining_time_ms;
    }
}


/** Interface functions */
static const generic_level_server_callbacks_t m_saturation_level_srv_cbs =
{
    .level_cbs.get_cb = saturation_level_state_get_cb,
    .level_cbs.set_cb = saturation_level_state_set_cb,
    .level_cbs.delta_set_cb = saturation_level_state_delta_set_cb,
    .level_cbs.move_set_cb = saturation_level_state_move_set_cb
};


static void saturation_level_state_publish(const light_hsl_setup_server_t *p_s_server,
                                           const light_hsl_saturation_status_params_t *p_params)
{
    generic_level_server_t *p_level_server;
    generic_level_status_params_t pub_params;

    p_level_server = (generic_level_server_t *)&p_s_server->hsl_saturation_srv.generic_level_srv;

    pub_params.present_level = light_hsl_utils_param_to_generic_level(p_params->present_saturation);
    pub_params.target_level = light_hsl_utils_param_to_generic_level(p_params->target_saturation);
    pub_params.remaining_time_ms = p_params->remaining_time_ms;

    (void) generic_level_server_status_publish(p_level_server, &pub_params);
}


static uint32_t hsl_saturation_server_init(light_hsl_saturation_server_t *p_server, uint8_t element_index)
{
    uint32_t status;

    if (p_server == NULL) {
        return NRF_ERROR_NULL;
    }

    /* Initialize parent model instance - Generic Level */
    p_server->generic_level_srv.settings.p_callbacks = &m_saturation_level_srv_cbs;

    status = generic_level_server_init(&p_server->generic_level_srv, element_index);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* Add this HSL Saturation server model to the mesh */
    access_model_add_params_t init_params = {
        .model_id = ACCESS_MODEL_SIG(LIGHT_HSL_SATURATION_SERVER_MODEL_ID),
        .element_index =  element_index,
        .p_opcode_handlers = m_opcode_handlers_saturation,
        .opcode_count = ARRAY_SIZE(m_opcode_handlers_saturation),
        .p_args = p_server,
        .publish_timeout_cb = periodic_saturation_publish_cb
    };

    status = access_model_add(&init_params, &p_server->model_handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* HSL Saturation Server sets up its subscription list, then shares it with its extended
     * level model */
    status = access_model_subscription_list_alloc(p_server->model_handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* deallocate the level model's subscription list so we can share with it */
    status = access_model_subscription_list_dealloc(p_server->generic_level_srv.model_handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* share the subscription list with the level model */
    status = access_model_subscription_lists_share(p_server->model_handle,
                                                   p_server->generic_level_srv.model_handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

    return status;
}



/*** HSL setup Server */

/** Opcode Handlers */

static void handle_setup_default_set(access_model_handle_t model_handle,
                                     const access_message_rx_t *p_rx_msg, void *p_args)
{
    light_hsl_setup_server_t *p_s_server = (light_hsl_setup_server_t *)p_args;
    light_hsl_default_set_msg_pkt_t *p_msg_pkt;
    light_hsl_default_set_params_t in_data = {0};
    light_hsl_default_status_params_t out_data = {0};
    light_hsl_default_status_params_t *p_out_data = NULL;

    if (p_rx_msg->length != LIGHT_HSL_DEFAULT_SET_LEN) {
        return;
    }

    p_msg_pkt = (light_hsl_default_set_msg_pkt_t *)p_rx_msg->p_data;

    in_data.lightness = p_msg_pkt->lightness;
    in_data.hue = p_msg_pkt->hue;
    in_data.saturation = p_msg_pkt->saturation;

    if (p_rx_msg->opcode.opcode == LIGHT_HSL_OPCODE_DEFAULT_SET) {
        p_out_data = &out_data;
    }

    p_s_server->settings.p_callbacks->light_hsl_cbs.default_set_cb(p_s_server,
                                                                   &p_rx_msg->meta_data,
                                                                   &in_data, p_out_data);
    if (p_out_data != NULL) {
        (void) status_default_send(p_s_server, HSL_SETUP_SERVER, p_rx_msg, &out_data);
    }
}


static void handle_setup_range_set(access_model_handle_t model_handle,
                                   const access_message_rx_t *p_rx_msg, void *p_args)
{
    light_hsl_setup_server_t *p_s_server = (light_hsl_setup_server_t *)p_args;
    light_hsl_range_set_msg_pkt_t *p_msg_pkt;
    light_hsl_range_set_params_t in_data = {0};
    light_hsl_range_status_params_t out_data = {0};
    light_hsl_range_status_params_t *p_out_data = NULL;

    if (p_rx_msg->length != LIGHT_HSL_RANGE_SET_LEN) {
        return;
    }

    p_msg_pkt = (light_hsl_range_set_msg_pkt_t *)p_rx_msg->p_data;

    in_data.hue_range_min = p_msg_pkt->hue_range_min;
    in_data.hue_range_max = p_msg_pkt->hue_range_max;
    in_data.saturation_range_min = p_msg_pkt->saturation_range_min;
    in_data.saturation_range_max = p_msg_pkt->saturation_range_max;

    if (p_rx_msg->opcode.opcode == LIGHT_HSL_OPCODE_RANGE_SET) {
        p_out_data = &out_data;
    }

    p_s_server->settings.p_callbacks->light_hsl_cbs.range_set_cb(p_s_server,
                                                                 &p_rx_msg->meta_data,
                                                                 &in_data, p_out_data);
    if (p_out_data != NULL) {
        (void)status_range_send(p_s_server, HSL_SETUP_SERVER, p_rx_msg, &out_data);
    }
}


static const access_opcode_handler_t m_opcode_handlers_setup[] =
{
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_DEFAULT_SET), handle_setup_default_set},
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_DEFAULT_SET_UNACKNOWLEDGED), handle_setup_default_set},
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_RANGE_SET), handle_setup_range_set},
    {ACCESS_OPCODE_SIG(LIGHT_HSL_OPCODE_RANGE_SET_UNACKNOWLEDGED), handle_setup_range_set}
};



/** HSL Setup Server initialization */
uint32_t light_hsl_setup_server_init(light_hsl_setup_server_t *p_s_server,
                                     light_lightness_setup_server_t *p_ll_s_server,
                                     uint8_t element_index)
{
    uint32_t status;

    if (m_total_hsl_ss_instances >= LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX) {
        return NRF_ERROR_NO_MEM;
    }

    /* check pointers on callbacks */
    if (p_s_server == NULL ||
            p_s_server->settings.p_callbacks == NULL ||
            p_s_server->settings.p_callbacks->light_hsl_cbs.get_cb == NULL ||
            p_s_server->settings.p_callbacks->light_hsl_cbs.set_cb == NULL ||
            p_s_server->settings.p_callbacks->light_hsl_cbs.hue_get_cb == NULL ||
            p_s_server->settings.p_callbacks->light_hsl_cbs.hue_set_cb == NULL ||
            p_s_server->settings.p_callbacks->light_hsl_cbs.saturation_get_cb == NULL ||
            p_s_server->settings.p_callbacks->light_hsl_cbs.saturation_set_cb == NULL ||
            p_s_server->settings.p_callbacks->light_hsl_cbs.default_get_cb == NULL ||
            p_s_server->settings.p_callbacks->light_hsl_cbs.default_set_cb == NULL ||
            p_s_server->settings.p_callbacks->light_hsl_cbs.range_get_cb == NULL ||
            p_s_server->settings.p_callbacks->light_hsl_cbs.range_set_cb == NULL ||
            p_s_server->settings.p_callbacks->light_hsl_cbs.hue_delta_set_cb == NULL ||
            p_s_server->settings.p_callbacks->light_hsl_cbs.hue_move_set_cb == NULL ||
            p_s_server->settings.p_callbacks->light_hsl_cbs.saturation_delta_set_cb == NULL ||
            p_s_server->settings.p_callbacks->light_hsl_cbs.saturation_move_set_cb == NULL) {
        return NRF_ERROR_NULL;
    }

    p_s_server->settings.element_index = element_index;

    /* initialize parent model instance - HSL Server */
    status = hsl_server_init(&p_s_server->hsl_srv, element_index);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* initialize model instance - HSL Hue Server on the next higher element */
    status = hsl_hue_server_init(&p_s_server->hsl_hue_srv, element_index + 1);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* initialize model instance - HSL Saturation Server on the next element after HSL Hue server */
    status = hsl_saturation_server_init(&p_s_server->hsl_saturation_srv, element_index + 2);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* Add this HSL setup server model to the mesh */
    access_model_add_params_t init_params = {
        .model_id = ACCESS_MODEL_SIG(LIGHT_HSL_SETUP_SERVER_MODEL_ID),
        .element_index =  element_index,
        .p_opcode_handlers = m_opcode_handlers_setup,
        .opcode_count = ARRAY_SIZE(m_opcode_handlers_setup),
        .p_args = p_s_server
    };

    status = access_model_add(&init_params, &p_s_server->model_handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* HSL Setup Server sets up its subscription list, then shares it with each of its extended models */
    status = access_model_subscription_list_alloc(p_s_server->model_handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* share the subscription list with the HSL Server (no deallocation needed, since the HSL server
     * doesn't do an alloc). */
    status = access_model_subscription_lists_share(p_s_server->model_handle,
                                                   p_s_server->hsl_srv.model_handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* Note that we don't share the subscription list with hue and saturation models
     * - it's on a different elements */

    /* The Light Lightness instance shares HSL setup server's subscription list */
    status = access_model_subscription_list_dealloc(p_ll_s_server->model_handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

    NRF_MESH_ERROR_CHECK(access_model_subscription_lists_share(p_s_server->model_handle,
                                                               p_ll_s_server->model_handle));
    NRF_MESH_ERROR_CHECK(access_model_subscription_lists_share(p_s_server->model_handle,
                                                               p_ll_s_server->generic_ponoff_setup_srv.model_handle));
    NRF_MESH_ERROR_CHECK(access_model_subscription_lists_share(p_s_server->model_handle,
                                                               p_ll_s_server->generic_ponoff_setup_srv.generic_ponoff_srv.model_handle));
    NRF_MESH_ERROR_CHECK(access_model_subscription_lists_share(p_s_server->model_handle,
                                                               p_ll_s_server->generic_ponoff_setup_srv.generic_ponoff_srv.generic_onoff_srv.model_handle));
    NRF_MESH_ERROR_CHECK(access_model_subscription_lists_share(p_s_server->model_handle,
                                                               p_ll_s_server->generic_ponoff_setup_srv.generic_dtt_srv.model_handle));
    NRF_MESH_ERROR_CHECK(access_model_subscription_lists_share(p_s_server->model_handle,
                                                               p_ll_s_server->light_lightness_srv.model_handle));
    NRF_MESH_ERROR_CHECK(access_model_subscription_lists_share(p_s_server->model_handle,
                                                               p_ll_s_server->light_lightness_srv.generic_level_srv.model_handle));
    m_total_hsl_ss_instances++;

    return NRF_SUCCESS;
}


/* @tagMeshMdlSp sections 6.1.4.1.2, 6.1.4.4.2 - Binding with the Generic OnPowerUp state */
uint32_t light_hsl_ponoff_binding_setup(light_hsl_setup_server_t *p_s_server, light_hsl_saved_values_t *p_saved_states)
{
    if (p_s_server == NULL || p_saved_states == NULL) {
        return NRF_ERROR_NULL;
    }

    light_hsl_hue_set_params_t hsl_hue_params = {0};
    light_hsl_saturation_set_params_t hsl_saturation_params = {0};

    if ((p_saved_states->onpowerup == GENERIC_ON_POWERUP_OFF) ||
            (p_saved_states->onpowerup == GENERIC_ON_POWERUP_DEFAULT)) {
        hsl_hue_params.hue = p_saved_states->default_hue;
        hsl_saturation_params.saturation = p_saved_states->default_saturation;
    } else if (p_saved_states->onpowerup == GENERIC_ON_POWERUP_RESTORE) {
        hsl_hue_params.hue = p_saved_states->hue;
        hsl_saturation_params.saturation = p_saved_states->saturation;
    } else {
        return NRF_ERROR_INVALID_DATA;
    }


    /* before setting the hue and saturation values, make sure it conforms to the range
       (@tagMeshMdlSp sections 6.1.4.1.3, @tagMeshMdlSp sections 6.1.4.4.3) */
    hsl_hue_params.hue = clip_hsl_hue_to_range(p_s_server, NULL, hsl_hue_params.hue);
    hsl_saturation_params.saturation = clip_hsl_saturation_to_range(p_s_server, NULL, 
                                                                    hsl_saturation_params.saturation);

    /* set HSL values (we can't use "set_cb" since we don't control lightness at powerup) */
    p_s_server->settings.p_callbacks->light_hsl_cbs.hue_set_cb(p_s_server, NULL, &hsl_hue_params,
                                                               NULL, NULL);
    p_s_server->settings.p_callbacks->light_hsl_cbs.saturation_set_cb(p_s_server, NULL,
                                                                      &hsl_saturation_params,
                                                                      NULL, NULL);

    p_s_server->state.initialized = true;

    return NRF_SUCCESS;
}


uint32_t light_hsl_server_status_publish(const light_hsl_server_t *p_server,
                                         const light_hsl_status_params_t *p_params)
{
    if ((p_server == NULL) || (p_params == NULL)) {
        return NRF_ERROR_NULL;
    }

    light_hsl_setup_server_t *p_s_server =
        PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_srv, p_server);
    light_hsl_hue_status_params_t status_params_hue;
    light_hsl_saturation_status_params_t status_params_saturation;

    status_params_hue.present_hue = p_params->present_hue;
    status_params_hue.target_hue = p_params->target_hue;
    status_params_hue.remaining_time_ms = p_params->remaining_time_ms;
    hue_level_state_publish(p_s_server, &status_params_hue);

    status_params_saturation.present_saturation = p_params->present_saturation;
    status_params_saturation.target_saturation = p_params->target_saturation;
    status_params_saturation.remaining_time_ms = p_params->remaining_time_ms;
    saturation_level_state_publish(p_s_server, &status_params_saturation);

    return status_send(p_server, NULL, p_params);
}


uint32_t light_hsl_server_hue_status_publish(const light_hsl_hue_server_t *p_server,
                                             const light_hsl_hue_status_params_t *p_params)
{
    if ((p_server == NULL) || (p_params == NULL)) {
        return NRF_ERROR_NULL;
    }

    light_hsl_setup_server_t *p_s_server =
        PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_hue_srv, p_server);
    hue_level_state_publish(p_s_server, p_params);

    return status_hue_send(p_server, NULL, p_params);
}


uint32_t light_hsl_server_saturation_status_publish(const light_hsl_saturation_server_t *p_server,
                                                    const light_hsl_saturation_status_params_t *p_params)
{
    if ((p_server == NULL) || (p_params == NULL)) {
        return NRF_ERROR_NULL;
    }

    light_hsl_setup_server_t *p_s_server =
        PARENT_BY_FIELD_GET(light_hsl_setup_server_t, hsl_hue_srv, p_server);
    saturation_level_state_publish(p_s_server, p_params);

    return status_saturation_send(p_server, NULL, p_params);
}


uint32_t light_hsl_server_default_status_publish(const light_hsl_server_t *p_server,
                                                 const light_hsl_default_status_params_t *p_params)
{
    if ((p_server == NULL) || (p_params == NULL)) {
        return NRF_ERROR_NULL;
    }

    return status_default_send(p_server, HSL_SERVER, NULL, p_params);
}


uint32_t light_hsl_server_range_status_publish(const light_hsl_server_t *p_server,
                                               const light_hsl_range_status_params_t *p_params)
{
    if ((p_server == NULL) || (p_params == NULL)) {
        return NRF_ERROR_NULL;
    }

    return status_range_send(p_server, HSL_SERVER, NULL, p_params);
}
