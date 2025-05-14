/* Copyright (C)2021-2023, St(u)dio of Computer Games
 * All rights reserved.
 */

#include "app_light_hsl.h"

/* application */
#include "light_lightness_mc.h"
#include "light_hsl_common.h"
#include "light_hsl_setup_server.h"
#include "light_hsl_utils.h"
#include "light_hsl_mc.h"

#include "list.h"
#include "bearer_event.h"

#include "mesh_app_utils.h"

/* logging */
#include "nrf5_sdk_log.h"



/* defines */

#define HSL_ADDITIONAL_PUBLISH_START_MARGIN_MS  (1500)
#define HSL_ADDITIONAL_PUBLISH_INTERVAL_MS      (1000)



/* type declarations */

typedef enum {
    PUBLISH_HSL,
    PUBLISH_HUE,
    PUBLISH_SATURATION,
} publish_type_t;



/* global variables */

static list_node_t * mp_app_light_hsl_head;
static bearer_event_flag_t m_transition_abort_flag;



/* forward declarations */

static void transition_parameters_set(app_light_hsl_setup_server_t *p_app,
                                      app_transition_t *transition,
                                      int32_t delta,
                                      const model_transition_t *p_in_transition,
                                      app_transition_type_t transition_type);



/* HSL Setup server model callbacks declaration */

static void hsl_state_get_cb(const light_hsl_setup_server_t *p_self,
                             const access_message_rx_meta_t *p_meta,
                             light_hsl_status_params_t *p_out);

static void hsl_state_set_cb(const light_hsl_setup_server_t *p_self,
                             const access_message_rx_meta_t *p_meta,
                             const light_hsl_set_params_t *p_in,
                             const model_transition_t *p_in_transition,
                             light_hsl_status_params_t *p_out);

static void hsl_state_hue_set_cb(const light_hsl_setup_server_t *p_self,
                                 const access_message_rx_meta_t *p_meta,
                                 const light_hsl_hue_set_params_t *p_in,
                                 const model_transition_t *p_in_transition,
                                 light_hsl_hue_status_params_t *p_out);

static void hsl_state_hue_get_cb(const light_hsl_setup_server_t *p_self,
                                 const access_message_rx_meta_t *p_meta,
                                 light_hsl_hue_status_params_t *p_out);

static void hsl_state_saturation_set_cb(const light_hsl_setup_server_t *p_self,
                                        const access_message_rx_meta_t *p_meta,
                                        const light_hsl_saturation_set_params_t *p_in,
                                        const model_transition_t *p_in_transition,
                                        light_hsl_saturation_status_params_t *p_out);

static void hsl_state_saturation_get_cb(const light_hsl_setup_server_t *p_self,
                                        const access_message_rx_meta_t *p_meta,
                                        light_hsl_saturation_status_params_t *p_out);

static void hsl_state_default_get_cb(const light_hsl_setup_server_t *p_self,
                                     const access_message_rx_meta_t *p_meta,
                                     light_hsl_default_status_params_t *p_out);

static void hsl_state_default_set_cb(const light_hsl_setup_server_t *p_self,
                                     const access_message_rx_meta_t *p_meta,
                                     const light_hsl_default_set_params_t *p_in,
                                     light_hsl_default_status_params_t *p_out);

static void hsl_state_range_get_cb(const light_hsl_setup_server_t *p_self,
                                   const access_message_rx_meta_t *p_meta,
                                   light_hsl_range_status_params_t *p_out);

static void hsl_state_range_set_cb(const light_hsl_setup_server_t *p_self,
                                   const access_message_rx_meta_t *p_meta,
                                   const light_hsl_range_set_params_t *p_in,
                                   light_hsl_range_status_params_t *p_out);

static void hsl_state_hue_delta_set_cb(const light_hsl_setup_server_t *p_self,
                                       const access_message_rx_meta_t *p_meta,
                                       const light_hsl_delta_set_params_t *p_in,
                                       const model_transition_t *p_in_transition,
                                       light_hsl_hue_status_params_t * p_out);

static void hsl_state_hue_move_set_cb(const light_hsl_setup_server_t *p_self,
                                      const access_message_rx_meta_t *p_meta,
                                      const light_hsl_move_set_params_t *p_in,
                                      const model_transition_t *p_in_transition,
                                      light_hsl_hue_status_params_t *p_out);

static void hsl_state_saturation_delta_set_cb(const light_hsl_setup_server_t *p_self,
                                              const access_message_rx_meta_t *p_meta,
                                              const light_hsl_delta_set_params_t *p_in,
                                              const model_transition_t *p_in_transition,
                                              light_hsl_saturation_status_params_t * p_out);

static void hsl_state_saturation_move_set_cb(const light_hsl_setup_server_t *p_self,
                                             const access_message_rx_meta_t *p_meta,
                                             const light_hsl_move_set_params_t *p_in,
                                             const model_transition_t *p_in_transition,
                                             light_hsl_saturation_status_params_t *p_out);


static const light_hsl_setup_server_callbacks_t hsl_setup_srv_cbs = {
    .light_hsl_cbs.get_cb = hsl_state_get_cb,
    .light_hsl_cbs.set_cb = hsl_state_set_cb,
    .light_hsl_cbs.hue_get_cb = hsl_state_hue_get_cb,
    .light_hsl_cbs.hue_set_cb = hsl_state_hue_set_cb,
    .light_hsl_cbs.saturation_get_cb = hsl_state_saturation_get_cb,
    .light_hsl_cbs.saturation_set_cb = hsl_state_saturation_set_cb,
    .light_hsl_cbs.default_get_cb = hsl_state_default_get_cb,
    .light_hsl_cbs.default_set_cb = hsl_state_default_set_cb,
    .light_hsl_cbs.range_get_cb = hsl_state_range_get_cb,
    .light_hsl_cbs.range_set_cb = hsl_state_range_set_cb,
    .light_hsl_cbs.hue_delta_set_cb = hsl_state_hue_delta_set_cb,
    .light_hsl_cbs.hue_move_set_cb = hsl_state_hue_move_set_cb,
    .light_hsl_cbs.saturation_delta_set_cb = hsl_state_saturation_delta_set_cb,
    .light_hsl_cbs.saturation_move_set_cb = hsl_state_saturation_move_set_cb,
};


#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
static void app_light_hsl_scene_store(const app_scene_model_interface_t * p_app_scene_if,
                                      uint8_t scene_index);
static void app_light_hsl_scene_recall(const app_scene_model_interface_t * p_app_scene_if,
                                       uint8_t scene_index,
                                       uint32_t delay_ms,
                                       uint32_t transition_time_ms);
static void app_light_hsl_scene_delete(const app_scene_model_interface_t * p_app_scene_if,
                                       uint8_t scene_index);


const app_scene_callbacks_t m_scene_light_hsl_cbs =
{
    .scene_store_cb = app_light_hsl_scene_store,
    .scene_recall_cb = app_light_hsl_scene_recall,
    .scene_delete_cb = app_light_hsl_scene_delete
};
#endif



/*** HSL Setup server model */

static uint32_t app_hsl_setup_server_init(app_light_hsl_setup_server_t *p_app, uint8_t element_index)
{
    uint32_t status = NRF_ERROR_INTERNAL;
    light_hsl_setup_server_t * p_s_server;

    p_s_server = &p_app->light_hsl_setup_srv;

    /* Initialize the CTL Setup server callback structure. */
    p_s_server->settings.p_callbacks = &hsl_setup_srv_cbs;

    /* set the default state (initialize flash) */
    status = light_hsl_mc_open(&p_s_server->state.handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

    p_s_server->state.initialized = false;

    return light_hsl_setup_server_init(p_s_server,
                                       &p_app->p_app_ll->light_lightness_setup_server,
                                       element_index);
}


static uint32_t hsl_publish(app_light_hsl_setup_server_t *p_app, uint32_t remaining_time_ms,
                            publish_type_t publish_type)
{
    light_hsl_server_t *p_hsl_srv;
    app_light_hsl_hw_state_t present_hsl_state;
    uint32_t status_hsl = NRF_SUCCESS;
    uint32_t status_hue = NRF_SUCCESS;
    uint32_t status_saturation = NRF_SUCCESS;

    p_app->app_light_hsl_get_cb(p_app, &present_hsl_state);

    // publish Light HSL status
    if (p_app->hsl_state_set_active) {
        uint16_t lightness;
        light_hsl_status_params_t publication_data;
        uint32_t ll_remaining_time = 0;

        p_app->p_app_ll->app_light_lightness_get_cb(p_app->p_app_ll, &lightness);

        /* calculate how much time is remaining for light lightness (if any) */
        if (app_transition_ongoing_get(&p_app->p_app_ll->state.transition)->transition_time_ms > 0) {
            ll_remaining_time = app_transition_remaining_time_get(&p_app->p_app_ll->state.transition);
        }

        if (ll_remaining_time == 0) {
            /* lightness is not transitioning, so use the provided time */
            publication_data.remaining_time_ms = remaining_time_ms;
        } else {
            /* lightness isn't done transitioning, so use the longer of the 2 transition times */
            publication_data.remaining_time_ms = MAX(remaining_time_ms, ll_remaining_time);
        }

        publication_data.present_lightness = lightness;
        publication_data.present_hue = present_hsl_state.hue;
        publication_data.present_saturation = present_hsl_state.saturation;
        publication_data.target_lightness = p_app->p_app_ll->state.target_lightness;
        publication_data.target_hue = p_app->state.target_hue;
        publication_data.target_saturation = p_app->state.target_saturation;

        p_hsl_srv = &p_app->light_hsl_setup_srv.hsl_srv;

        status_hsl = light_hsl_server_status_publish(p_hsl_srv, &publication_data);
    }

    // publish Light HSL hue status
    if (publish_type == PUBLISH_HUE) {
        light_hsl_hue_server_t *p_hsl_hue_srv;
        light_hsl_hue_status_params_t hue_publication_data;

        hue_publication_data.present_hue = present_hsl_state.hue;
        hue_publication_data.target_hue = p_app->state.target_hue;
        hue_publication_data.remaining_time_ms = remaining_time_ms;

        p_hsl_hue_srv = &p_app->light_hsl_setup_srv.hsl_hue_srv;
        status_hue = light_hsl_server_hue_status_publish(p_hsl_hue_srv, &hue_publication_data);
    }

    // publish Light HSL saturation status
    if (publish_type == PUBLISH_SATURATION) {
        light_hsl_saturation_server_t *p_hsl_saturation_srv;
        light_hsl_saturation_status_params_t saturation_publication_data;

        saturation_publication_data.present_saturation = present_hsl_state.saturation;
        saturation_publication_data.target_saturation = p_app->state.target_saturation;
        saturation_publication_data.remaining_time_ms = remaining_time_ms;

        p_hsl_saturation_srv = &p_app->light_hsl_setup_srv.hsl_saturation_srv;
        status_saturation = light_hsl_server_saturation_status_publish(p_hsl_saturation_srv,
                                                                       &saturation_publication_data);
    }

    return status_hsl != NRF_SUCCESS ? status_hsl :
            (status_hue != NRF_SUCCESS ? status_hue : status_saturation);
}


/* callback used by light lightness module to inform this module when it has done a lightness
 * publish. This is to cover the case where a Light Lightness message was received meaning
 * that HSL should publish since the lightness component of the composite state changed */
static void ll_add_publish_cb(const void *p_app_v, light_lightness_status_params_t *p_pub_data)
{
    const app_light_hsl_setup_server_t *p_app = (app_light_hsl_setup_server_t *)p_app_v;

    if (p_app->light_hsl_setup_srv.state.initialized == false) {
        /* ignore any publish before we have finished setting up the HSL server */
        return;
    }

    if (p_app->hsl_state_set_active) {
        app_light_hsl_hw_state_t present_hsl_state;
        light_hsl_status_params_t publication_data;

        p_app->app_light_hsl_get_cb(p_app, &present_hsl_state);

        publication_data.present_lightness = p_pub_data->present_lightness;
        publication_data.present_hue = present_hsl_state.hue;
        publication_data.present_saturation = present_hsl_state.saturation;
        publication_data.target_lightness = p_pub_data->target_lightness;
        publication_data.target_hue = p_app->state.target_hue;
        publication_data.target_saturation = p_app->state.target_saturation;
        publication_data.remaining_time_ms = p_pub_data->remaining_time_ms;

        (void) light_hsl_server_status_publish(&p_app->light_hsl_setup_srv.hsl_srv,
                                               &publication_data);
    }
}


/* 6.1.4.1.3 Binding with the HSL Hue Range state */
static void present_hsl_values_set(app_light_hsl_setup_server_t *p_app, light_hsl_range_set_params_t *p_range)
{
    app_light_hsl_hw_state_t present_hsl_state;

    present_hsl_state.hue =
        light_hsl_utils_param_range_restrict(p_app->state.present_hue,
                                             p_range->hue_range_min,
                                             p_range->hue_range_max);

    present_hsl_state.saturation =
        light_hsl_utils_param_range_restrict(p_app->state.present_saturation,
                                             p_range->saturation_range_min,
                                             p_range->saturation_range_max);

    if (p_app->app_light_hsl_set_cb != NULL) {
        p_app->app_light_hsl_set_cb(p_app, &present_hsl_state);
    }

    p_app->state.present_hue = present_hsl_state.hue;
    p_app->state.present_saturation = present_hsl_state.saturation;
}



/** HSL Setup server model interface callbacks */

static void hsl_state_get_cb(const light_hsl_setup_server_t *p_self,
                             const access_message_rx_meta_t *p_meta,
                             light_hsl_status_params_t *p_out)
{
    app_light_hsl_setup_server_t *p_app;
    app_transition_params_t *p_params_hue, *p_params_saturation;
    uint16_t lightness;
    app_light_hsl_hw_state_t present_hsl_state;

    p_app = PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, light_hsl_setup_srv, p_self);
    p_params_hue = app_transition_ongoing_get(&p_app->state.transition_hue);
    p_params_saturation = app_transition_ongoing_get(&p_app->state.transition_saturation);

    p_app->p_app_ll->app_light_lightness_get_cb(p_app->p_app_ll, &lightness);
    p_app->app_light_hsl_get_cb(p_app, &present_hsl_state);

    p_out->present_lightness = lightness;
    p_out->target_lightness = p_app->p_app_ll->state.target_lightness;
    p_out->present_hue = present_hsl_state.hue;
    p_out->target_hue = p_app->state.target_hue;
    p_out->present_saturation = present_hsl_state.saturation;
    p_out->target_saturation = p_app->state.target_saturation;

    /* requirement: report remaining time during processing of SET or DELTA SET,
     *              report zero/unknown transition time during processing of MOVE */
    if (p_params_hue->transition_type == APP_TRANSITION_TYPE_MOVE_SET ||
            p_params_saturation->transition_type == APP_TRANSITION_TYPE_MOVE_SET) {
        p_out->remaining_time_ms =
            (p_params_hue->transition_time_ms == 0 ||
                p_params_saturation->transition_time_ms == 0) ?
                    0 : MODEL_TRANSITION_TIME_UNKNOWN;
    } else {
        p_out->remaining_time_ms = MAX(app_transition_remaining_time_get(&p_app->state.transition_hue),
                                       app_transition_remaining_time_get(&p_app->state.transition_saturation));
    }

    NRF_LOG_INFO("%s(): pr H: %d S: %d L: %d", __func__,
        p_out->present_hue,
        p_out->present_saturation,
        p_out->present_lightness);
    NRF_LOG_INFO("%s(): tgt H: %d S: %d L: %d", __func__,
        p_out->target_hue,
        p_out->target_saturation,
        p_out->target_lightness);
    NRF_LOG_INFO("%s(): rem-tt: %d ms req-delta-hue: %d req-delta-sat: %d", __func__,
        p_out->remaining_time_ms,
        p_params_hue->required_delta,
        p_params_saturation->required_delta);
}


static void hsl_state_set_cb(const light_hsl_setup_server_t *p_self,
                             const access_message_rx_meta_t *p_meta,
                             const light_hsl_set_params_t *p_in,
                             const model_transition_t *p_in_transition,
                             light_hsl_status_params_t *p_out)
{
    app_light_hsl_setup_server_t *p_app;
    app_transition_params_t *p_params;
    uint32_t transition_time_ms;
    light_lightness_set_params_t ll_in;
    light_lightness_status_params_t ll_out;
    light_lightness_setup_server_t *p_ll_ss;
    app_light_hsl_hw_state_t present_hsl_state;
    bool changed = false;


    p_app = PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, light_hsl_setup_srv, p_self);

    /* set a module flag to let the code know there is a HSL state set occurring */
    p_app->hsl_state_set_active = true;

    /* first, call the light lightness mid app to do the lightness portion of the HSL state */
    ll_in.lightness = p_in->lightness;
    ll_in.tid = p_in->tid;

    p_ll_ss = &p_app->p_app_ll->light_lightness_setup_server;
    p_ll_ss->settings.p_callbacks->light_lightness_cbs.set_cb(p_ll_ss, p_meta, &ll_in,
                                                              p_in_transition, &ll_out);
    NRF_LOG_INFO("%s(): tgt L: %d", __func__,
        p_in->lightness);

    /* process hue/saturation states */
    p_app->app_light_hsl_get_cb(p_app, &present_hsl_state);
    p_app->state.snapshot_target_hue = p_in->hue;
    p_app->state.snapshot_present_hue = present_hsl_state.hue;
    p_app->state.snapshot_target_saturation = p_in->saturation;
    p_app->state.snapshot_present_saturation = present_hsl_state.saturation;

    // ToDo: check for current OnPowerOn status
    ERROR_CHECK(light_hsl_mc_hue_state_set(p_self->state.handle, p_app->state.snapshot_target_hue));
    ERROR_CHECK(light_hsl_mc_saturation_state_set(p_self->state.handle, p_app->state.snapshot_target_saturation));

    /* if this is first time since boot, set all of the values, otherwise, check which values to set */
    if (!p_self->state.initialized || (present_hsl_state.hue != p_in->hue)) {
        int32_t delta = (int32_t)p_app->state.snapshot_target_hue - (int32_t)present_hsl_state.hue;
        transition_parameters_set(p_app, &p_app->state.transition_hue,
                                  delta, p_in_transition, APP_TRANSITION_TYPE_SET);

        app_transition_trigger(&p_app->state.transition_hue);

        changed = true;
        p_params = app_transition_requested_get(&p_app->state.transition_hue);
        NRF_LOG_INFO("%s(): tgt H: %d delay: %d tt: %d req-delta: %d", __func__,
            p_in->hue,
            p_app->state.transition_hue.delay_ms,
            p_params->transition_time_ms,
            p_params->required_delta);
    }

    if (!p_self->state.initialized || (present_hsl_state.saturation != p_in->saturation)) {
        int32_t delta = (int32_t)p_app->state.snapshot_target_saturation - (int32_t)present_hsl_state.saturation;
        transition_parameters_set(p_app, &p_app->state.transition_saturation,
                                  delta, p_in_transition, APP_TRANSITION_TYPE_SET);

        app_transition_trigger(&p_app->state.transition_saturation);

        changed = true;
        p_params = app_transition_requested_get(&p_app->state.transition_saturation);
        NRF_LOG_INFO("%s(): tgt S: %d delay: %d tt: %d req-delta: %d", __func__,
            p_in->saturation,
            p_app->state.transition_hue.delay_ms,
            p_params->transition_time_ms,
            p_params->required_delta);
    }

    if (changed) {
        transition_time_ms = p_in_transition->transition_time_ms;
#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
        app_scene_model_scene_changed(p_app->p_app_scene);
#endif

    } else {
        transition_time_ms = 0;
    }

    /* prepare response */
    if (p_out != NULL) {
        p_out->present_lightness = ll_out.present_lightness;
        p_out->present_hue = p_app->state.present_hue;
        p_out->present_saturation = p_app->state.present_saturation;
        p_out->remaining_time_ms = transition_time_ms;
    }
}


static void hsl_state_hue_get_cb(const light_hsl_setup_server_t *p_self,
                                 const access_message_rx_meta_t *p_meta,
                                 light_hsl_hue_status_params_t *p_out)
{
    app_light_hsl_setup_server_t * p_app;
    app_light_hsl_hw_state_t present_hsl_state;
    light_hsl_range_set_params_t range_set;

    p_app = PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, light_hsl_setup_srv, p_self);
    app_transition_params_t *p_params = app_transition_ongoing_get(&p_app->state.transition_hue);

    /* requirement: Provide the current value of some of the CTL composite state */
    p_app->app_light_hsl_get_cb(p_app, &present_hsl_state);
    p_out->present_hue = present_hsl_state.hue;

    /* requirement: report remaining time during processing of SET or DELTA SET
     *              report zero/unknown transition time during processing of MOVE */
    if (p_params->transition_type == APP_TRANSITION_TYPE_MOVE_SET) {
        ERROR_CHECK(light_hsl_mc_range_state_get(p_self->state.handle, &range_set));
        /* insert values as expected in level status message */
        p_out->target_hue =
            p_app->state.target_hue == range_set.hue_range_max ? INT16_MAX : INT16_MIN;
        /* response to Move Set message is sent with unknown transition time value, if given
         * transition time is non-zero. */
        p_out->remaining_time_ms = (p_params->transition_time_ms == 0 || p_params->required_delta == 0) ?
                                    0 : MODEL_TRANSITION_TIME_UNKNOWN;
    } else {
        p_out->target_hue = p_app->state.target_hue;
        p_out->remaining_time_ms = app_transition_remaining_time_get(&p_app->state.transition_hue);
    }

    NRF_LOG_INFO("%s(): pr H: %d tgt H: %d rem-tt: %d ms req-delta: %d", __func__,
        p_out->present_hue,
        p_out->target_hue,
        p_out->remaining_time_ms,
        p_params->required_delta);
}


static void hsl_state_hue_set_cb(const light_hsl_setup_server_t *p_self,
                                 const access_message_rx_meta_t *p_meta,
                                 const light_hsl_hue_set_params_t *p_in,
                                 const model_transition_t *p_in_transition,
                                 light_hsl_hue_status_params_t *p_out)
{
    app_light_hsl_setup_server_t *p_app;
    uint32_t transition_time_ms;
    app_light_hsl_hw_state_t present_hsl_state;

    p_app = PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, light_hsl_setup_srv, p_self);

    /* process hue states */
    p_app->app_light_hsl_get_cb(p_app, &present_hsl_state);
    p_app->state.snapshot_target_hue = p_in->hue;
    p_app->state.snapshot_present_hue = present_hsl_state.hue;

    // ToDo: check for current OnPowerOn status
    ERROR_CHECK(light_hsl_mc_hue_state_set(p_self->state.handle, p_app->state.snapshot_target_hue));

    /* if this is first time since boot, set all of the values, otherwise, check which values to set */
    if (!p_self->state.initialized || (present_hsl_state.hue != p_in->hue)) {
        /* set a module flag to let the code know there is a HSL Hue state set occurring */
        p_app->hsl_state_hue_set_active = true;

        int32_t delta = (int32_t)p_app->state.snapshot_target_hue - (int32_t)present_hsl_state.hue;
        transition_parameters_set(p_app, &p_app->state.transition_hue,
                                  delta, p_in_transition, APP_TRANSITION_TYPE_SET);

        app_transition_trigger(&p_app->state.transition_hue);

#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
        app_scene_model_scene_changed(p_app->p_app_scene);
#endif

        app_transition_params_t *p_params = app_transition_ongoing_get(&p_app->state.transition_hue);
        transition_time_ms = p_params->transition_time_ms;
        NRF_LOG_INFO("%s(): tgt H: %d delay: %d tt: %d req-delta: %d", __func__,
            p_in->hue,
            p_app->state.transition_hue.delay_ms,
            p_params->transition_time_ms,
            p_params->required_delta);

    } else {
        transition_time_ms = 0;
    }

    /* prepare response */
    if (p_out != NULL) {
        p_out->present_hue = p_app->state.present_hue;
        p_out->target_hue = p_app->state.snapshot_target_hue;
        p_out->remaining_time_ms = transition_time_ms;
    }
}


static void hsl_state_saturation_get_cb(const light_hsl_setup_server_t *p_self,
                                        const access_message_rx_meta_t *p_meta,
                                        light_hsl_saturation_status_params_t *p_out)
{
    app_light_hsl_setup_server_t * p_app;
    app_light_hsl_hw_state_t present_hsl_state;
    light_hsl_range_set_params_t range_set;

    p_app = PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, light_hsl_setup_srv, p_self);
    app_transition_params_t *p_params = app_transition_ongoing_get(&p_app->state.transition_saturation);

    /* requirement: Provide the current value of some of the CTL composite state */
    p_app->app_light_hsl_get_cb(p_app, &present_hsl_state);
    p_out->present_saturation = present_hsl_state.saturation;

    /* requirement: report remaining time during processing of SET or DELTA SET
     *              report zero/unknown transition time during processing of MOVE */
    if (p_params->transition_type == APP_TRANSITION_TYPE_MOVE_SET) {
        ERROR_CHECK(light_hsl_mc_range_state_get(p_self->state.handle, &range_set));
        /* insert values as expected in level status message */
        p_out->target_saturation =
            p_app->state.target_saturation == range_set.saturation_range_max ? INT16_MAX : INT16_MIN;
        /* response to Move Set message is sent with unknown transition time value, if given
         * transition time is non-zero. */
        p_out->remaining_time_ms = (p_params->transition_time_ms == 0 || p_params->required_delta == 0) ?
                                    0 : MODEL_TRANSITION_TIME_UNKNOWN;
    } else {
        p_out->target_saturation = p_app->state.target_saturation;
        p_out->remaining_time_ms = app_transition_remaining_time_get(&p_app->state.transition_saturation);
    }

    NRF_LOG_INFO("%s(): pr S: %d tgt S: %d rem-tt: %d ms req-delta: %d", __func__,
        p_out->present_saturation,
        p_out->target_saturation,
        p_out->remaining_time_ms,
        p_params->required_delta);
}


static void hsl_state_saturation_set_cb(const light_hsl_setup_server_t *p_self,
                                        const access_message_rx_meta_t *p_meta,
                                        const light_hsl_saturation_set_params_t *p_in,
                                        const model_transition_t *p_in_transition,
                                        light_hsl_saturation_status_params_t *p_out)
{
    app_light_hsl_setup_server_t *p_app;
    uint32_t transition_time_ms;
    app_light_hsl_hw_state_t present_hsl_state;

    p_app = PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, light_hsl_setup_srv, p_self);

    /* process saturation states */
    p_app->app_light_hsl_get_cb(p_app, &present_hsl_state);
    p_app->state.snapshot_target_saturation = p_in->saturation;
    p_app->state.snapshot_present_saturation = present_hsl_state.saturation;

    // ToDo: check for current OnPowerOn status
    ERROR_CHECK(light_hsl_mc_saturation_state_set(p_self->state.handle, p_app->state.snapshot_target_saturation));

    /* if this is first time since boot, set all of the values, otherwise, check which values to set */

    if (!p_self->state.initialized || (present_hsl_state.saturation != p_in->saturation)) {
        /* set a module flag to let the code know there is a HSL state set occurring */
        p_app->hsl_state_saturation_set_active = true;

        int32_t delta = (int32_t)p_app->state.snapshot_target_saturation - (int32_t)present_hsl_state.saturation;
        transition_parameters_set(p_app, &p_app->state.transition_saturation,
                                  delta, p_in_transition, APP_TRANSITION_TYPE_SET);

        app_transition_trigger(&p_app->state.transition_saturation);

#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
        app_scene_model_scene_changed(p_app->p_app_scene);
#endif

        app_transition_params_t *p_params = app_transition_ongoing_get(&p_app->state.transition_saturation);
        transition_time_ms = p_params->transition_time_ms;
        NRF_LOG_INFO("%s(): tgt S: %d delay: %d tt: %d req-delta: %d", __func__,
            p_in->saturation,
            p_app->state.transition_saturation.delay_ms,
            p_params->transition_time_ms,
            p_params->required_delta);
    } else {
        transition_time_ms = 0;
    }

    /* prepare response */
    if (p_out != NULL) {
        p_out->present_saturation = p_app->state.present_saturation;
        p_out->target_saturation = p_app->state.snapshot_target_saturation;
        p_out->remaining_time_ms = transition_time_ms;
    }
}


static void hsl_state_default_get_cb(const light_hsl_setup_server_t *p_self,
                                     const access_message_rx_meta_t *p_meta,
                                     light_hsl_default_status_params_t *p_out)
{
    app_light_hsl_setup_server_t *p_app;

    p_app = PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, light_hsl_setup_srv, p_self);

    ERROR_CHECK(light_lightness_mc_default_state_get(p_app->p_app_ll->light_lightness_setup_server.state.handle,
                                                     &p_out->lightness));
    ERROR_CHECK(light_hsl_mc_default_hue_state_get(p_self->state.handle, &p_out->hue));
    ERROR_CHECK(light_hsl_mc_default_saturation_state_get(p_self->state.handle,
                                                          &p_out->saturation));
}


static void hsl_state_default_set_cb(const light_hsl_setup_server_t *p_self,
                                     const access_message_rx_meta_t *p_meta,
                                     const light_hsl_default_set_params_t *p_in,
                                     light_hsl_default_status_params_t *p_out)
{
    app_light_hsl_setup_server_t *p_app;
    light_hsl_default_status_params_t default_status;

    p_app = PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, light_hsl_setup_srv, p_self);

    /* Update internal representation of default value. */
    ERROR_CHECK(light_lightness_mc_default_state_set(p_app->p_app_ll->light_lightness_setup_server.state.handle,
                                                     p_in->lightness));
    ERROR_CHECK(light_hsl_mc_default_hue_state_set(p_self->state.handle, p_in->hue));
    ERROR_CHECK(light_hsl_mc_default_saturation_state_set(p_self->state.handle, p_in->saturation));

    ERROR_CHECK(light_lightness_mc_default_state_get(p_app->p_app_ll->light_lightness_setup_server.state.handle,
                                                     &default_status.lightness));
    ERROR_CHECK(light_hsl_mc_default_hue_state_get(p_self->state.handle, &default_status.hue));
    ERROR_CHECK(light_hsl_mc_default_saturation_state_get(p_self->state.handle,
                                                          &default_status.saturation));

    /* ignore status, as publish may fail due to several reasons and it is ok */
    (void) light_hsl_server_default_status_publish(&p_self->hsl_srv, &default_status);
 
    if (p_out != NULL) {
        /* Prepare response */
        *p_out = default_status;
    }
}



static void hsl_state_range_get_cb(const light_hsl_setup_server_t *p_self,
                                   const access_message_rx_meta_t *p_meta,
                                   light_hsl_range_status_params_t *p_out)
{
    light_hsl_range_set_params_t range_set;

    ERROR_CHECK(light_hsl_mc_range_state_get(p_self->state.handle, &range_set));
    p_out->status_code = LIGHT_HSL_RANGE_STATUS_SUCCESS;
    p_out->hue_range_min = range_set.hue_range_min;
    p_out->hue_range_max = range_set.hue_range_max;
    p_out->saturation_range_min = range_set.saturation_range_min;
    p_out->saturation_range_max = range_set.saturation_range_max;
}


static void hsl_state_range_set_cb(const light_hsl_setup_server_t *p_self,
                                   const access_message_rx_meta_t *p_meta,
                                   const light_hsl_range_set_params_t *p_in,
                                   light_hsl_range_status_params_t *p_out)
{
    light_hsl_range_set_params_t range_set;
    light_hsl_range_status_params_t range;

    range_set = *p_in;
    ERROR_CHECK(light_hsl_mc_range_state_set(p_self->state.handle, &range_set));

    ERROR_CHECK(light_hsl_mc_range_state_get(p_self->state.handle, &range_set));

    range.status_code = LIGHT_HSL_RANGE_STATUS_SUCCESS;
    range.hue_range_min = range_set.hue_range_min;
    range.hue_range_max = range_set.hue_range_max;
    range.saturation_range_min = range_set.saturation_range_min;
    range.saturation_range_max = range_set.saturation_range_max;

    /* ignore status, as publish may fail due to several reasons and it is ok */
    (void) light_hsl_server_range_status_publish(&p_self->hsl_srv, &range);

    if (p_out != NULL) {
        *p_out = range;
    }
}



/** Generic Level server model interface callbacks */

static void hsl_state_hue_delta_set_cb(const light_hsl_setup_server_t *p_self,
                                       const access_message_rx_meta_t *p_meta,
                                       const light_hsl_delta_set_params_t *p_in,
                                       const model_transition_t *p_in_transition,
                                       light_hsl_hue_status_params_t *p_out)
{
    app_light_hsl_setup_server_t *p_app;
    app_transition_params_t *p_params;
    int32_t target_hue;
    light_hsl_range_set_params_t range_set;
    app_light_hsl_hw_state_t present_hsl_state;


    p_app = PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t,
                                light_hsl_setup_srv,
                                p_self);
    NRF_MESH_ASSERT(p_app != NULL && p_in != NULL);
    p_params = app_transition_requested_get(&p_app->state.transition_hue);

    p_app->hsl_state_hue_set_active = true;

    p_app->state.new_tid_hue = model_transaction_is_new(&p_app->light_hsl_setup_srv.hsl_hue_srv.tid_tracker);
    NRF_LOG_INFO("%s(): tid: %d %s", __func__, p_in->tid, p_app->state.new_tid_hue ?
          "new" : "same, cumulative delta set");

    p_app->app_light_hsl_get_cb(p_app, &present_hsl_state);
    if (p_app->state.new_tid_hue) {
        p_app->state.snapshot_present_hue = present_hsl_state.hue;
    }

    target_hue = p_app->state.new_tid_hue ?
        (int32_t)present_hsl_state.hue + p_in->delta :
        (int32_t)p_app->state.initial_present_hue + p_in->delta;

    /* if delta is too large, target value should get clipped to range limits */
    ERROR_CHECK(light_hsl_mc_range_state_get(p_self->state.handle, &range_set));
    p_app->state.snapshot_target_hue = target_hue <= 0 ? 0 :
        target_hue > range_set.hue_range_max ? range_set.hue_range_max :
        target_hue < range_set.hue_range_min ? range_set.hue_range_min
            : target_hue;

    transition_parameters_set(p_app, &p_app->state.transition_hue,
                              p_in->delta,
                              p_in_transition,
                              APP_TRANSITION_TYPE_DELTA_SET);

    NRF_LOG_INFO("%s(): delta: %d  delay: %d  tt: %d", __func__,
        p_in->delta,
        p_app->state.transition_hue.delay_ms,
        p_params->transition_time_ms);

    app_transition_trigger(&p_app->state.transition_hue);

#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
    if (p_in->delta != 0) {
        app_scene_model_scene_changed(p_app->p_app_scene);
    }
#endif

    /* prepare response */
    if (p_out != NULL) {
        p_out->present_hue = p_app->state.present_hue;
        p_out->target_hue  = p_app->state.snapshot_target_hue;
        p_out->remaining_time_ms = p_params->transition_time_ms;
    }
}


static void hsl_state_hue_move_set_cb(const light_hsl_setup_server_t *p_self,
                                      const access_message_rx_meta_t *p_meta,
                                      const light_hsl_move_set_params_t *p_in,
                                      const model_transition_t *p_in_transition,
                                      light_hsl_hue_status_params_t *p_out)
{
    app_light_hsl_setup_server_t *p_app;
    app_transition_params_t *p_params;
    light_hsl_range_set_params_t range_set;
    app_light_hsl_hw_state_t present_hsl_state;

    p_app = PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, light_hsl_setup_srv, p_self);
    p_params = app_transition_requested_get(&p_app->state.transition_hue);

    p_app->hsl_state_hue_set_active = true;
    p_app->app_light_hsl_get_cb(p_app, &present_hsl_state);
    p_app->state.present_hue = present_hsl_state.hue;

    transition_parameters_set(p_app, &p_app->state.transition_hue,
                              (int32_t)p_in->delta, p_in_transition,
                              APP_TRANSITION_TYPE_MOVE_SET);

    /* requirement: for the status message: target temperature state is the upper limit of
     * the temperature state when the transition speed is positive, or the lower limit of the
     * temperature state when the transition speed is negative. */
    ERROR_CHECK(light_hsl_mc_range_state_get(p_self->state.handle, &range_set));
    if (p_in->delta > 0 &&
            p_app->state.transition_hue.requested_params.transition_time_ms > 0) {
        p_app->state.snapshot_target_hue = range_set.hue_range_max;
    } else if (p_in->delta < 0 &&
            p_app->state.transition_hue.requested_params.transition_time_ms > 0) {
        p_app->state.snapshot_target_hue = range_set.hue_range_min;
    } else {
        p_app->state.snapshot_target_hue = p_app->state.present_hue;
    }

    NRF_LOG_INFO("%s(): delta: %d delay: %d tt: %d", __func__,
          p_in->delta, p_app->state.transition_hue.delay_ms,
          p_params->transition_time_ms);

    app_transition_trigger(&p_app->state.transition_hue);

#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
    if ((p_in->delta != 0) &&
            (p_app->state.transition_hue.requested_params.transition_time_ms > 0)) {
        app_scene_model_scene_changed(p_app->p_app_scene);
    }
#endif

    /* prepare response */
    if (p_out != NULL) {
        p_out->present_hue = p_app->state.present_hue;
        /* insert values as expected in level status message for Move Set */
        p_out->target_hue =
            p_app->state.snapshot_target_hue == range_set.hue_range_max ? UINT16_MAX : 0;
        /* response to Move Set message is sent with unknown transition time value, if given
         * transition time is non-zero. */
        p_out->remaining_time_ms = (p_params->transition_time_ms == 0 || p_in->delta == 0) ?
            0 : MODEL_TRANSITION_TIME_UNKNOWN;
    }
}



static void hsl_state_saturation_delta_set_cb(const light_hsl_setup_server_t *p_self,
                                              const access_message_rx_meta_t *p_meta,
                                              const light_hsl_delta_set_params_t *p_in,
                                              const model_transition_t *p_in_transition,
                                              light_hsl_saturation_status_params_t * p_out)
{
    app_light_hsl_setup_server_t *p_app;
    app_transition_params_t *p_params;
    int32_t target_saturation;
    light_hsl_range_set_params_t range_set;
    app_light_hsl_hw_state_t present_hsl_state;


    p_app = PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t,
                                light_hsl_setup_srv,
                                p_self);
    NRF_MESH_ASSERT(p_app != NULL && p_in != NULL);
    p_params = app_transition_requested_get(&p_app->state.transition_saturation);

    p_app->hsl_state_saturation_set_active = true;

    p_app->state.new_tid_saturation = model_transaction_is_new(&p_app->light_hsl_setup_srv.hsl_saturation_srv.tid_tracker);
    NRF_LOG_INFO("%s(): tid: %d %s", __func__, p_in->tid, p_app->state.new_tid_saturation ?
          "new" : "same, cumulative delta set");

    p_app->app_light_hsl_get_cb(p_app, &present_hsl_state);
    if (p_app->state.new_tid_saturation) {
        p_app->state.snapshot_present_saturation = present_hsl_state.saturation;
    }

    target_saturation = p_app->state.new_tid_saturation ?
        (int32_t)present_hsl_state.saturation + p_in->delta :
        (int32_t)p_app->state.initial_present_saturation + p_in->delta;

    /* if delta is too large, target value should get clipped to range limits */
    ERROR_CHECK(light_hsl_mc_range_state_get(p_self->state.handle, &range_set));
    p_app->state.snapshot_target_saturation = target_saturation <= 0 ? 0 :
        target_saturation > range_set.saturation_range_max ? range_set.saturation_range_max :
        target_saturation < range_set.saturation_range_min ? range_set.saturation_range_min
            : target_saturation;

    transition_parameters_set(p_app, &p_app->state.transition_saturation,
                              p_in->delta,
                              p_in_transition,
                              APP_TRANSITION_TYPE_DELTA_SET);

    NRF_LOG_INFO("%s(): delta: %d delay: %d tt: %d", __func__,
        p_in->delta,
        p_app->state.transition_saturation.delay_ms,
        p_params->transition_time_ms);

    app_transition_trigger(&p_app->state.transition_saturation);

#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
    if (p_in->delta != 0) {
        app_scene_model_scene_changed(p_app->p_app_scene);
    }
#endif

    /* prepare response */
    if (p_out != NULL) {
        p_out->present_saturation = p_app->state.present_saturation;
        p_out->target_saturation  = p_app->state.snapshot_target_saturation;
        p_out->remaining_time_ms = p_params->transition_time_ms;
    }
}


static void hsl_state_saturation_move_set_cb(const light_hsl_setup_server_t *p_self,
                                             const access_message_rx_meta_t *p_meta,
                                             const light_hsl_move_set_params_t *p_in,
                                             const model_transition_t *p_in_transition,
                                             light_hsl_saturation_status_params_t *p_out)
{
    app_light_hsl_setup_server_t *p_app;
    app_transition_params_t *p_params;
    light_hsl_range_set_params_t range_set;
    app_light_hsl_hw_state_t present_hsl_state;

    p_app = PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, light_hsl_setup_srv, p_self);
    p_params = app_transition_requested_get(&p_app->state.transition_saturation);

    p_app->hsl_state_saturation_set_active = true;
    p_app->app_light_hsl_get_cb(p_app, &present_hsl_state);
    p_app->state.present_saturation = present_hsl_state.saturation;

    transition_parameters_set(p_app, &p_app->state.transition_saturation,
                              (int32_t)p_in->delta, p_in_transition,
                              APP_TRANSITION_TYPE_MOVE_SET);

    /* requirement: for the status message: target temperature state is the upper limit of
     * the temperature state when the transition speed is positive, or the lower limit of the
     * temperature state when the transition speed is negative. */
    ERROR_CHECK(light_hsl_mc_range_state_get(p_self->state.handle, &range_set));
    if (p_in->delta > 0 &&
            p_app->state.transition_saturation.requested_params.transition_time_ms > 0) {
        p_app->state.snapshot_target_saturation = range_set.saturation_range_max;
    } else if (p_in->delta < 0 &&
            p_app->state.transition_saturation.requested_params.transition_time_ms > 0) {
        p_app->state.snapshot_target_saturation = range_set.saturation_range_min;
    } else {
        p_app->state.snapshot_target_saturation = p_app->state.present_saturation;
    }

    NRF_LOG_INFO("%s(): delta: %d delay: %d tt: %d", __func__,
          p_in->delta, p_app->state.transition_saturation.delay_ms,
          p_params->transition_time_ms);

    app_transition_trigger(&p_app->state.transition_saturation);

#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
    if ((p_in->delta != 0) &&
            (p_app->state.transition_saturation.requested_params.transition_time_ms > 0)) {
        app_scene_model_scene_changed(p_app->p_app_scene);
    }
#endif

    /* prepare response */
    if (p_out != NULL) {
        p_out->present_saturation = p_app->state.present_saturation;
        /* insert values as expected in level status message for Move Set */
        p_out->target_saturation =
            p_app->state.snapshot_target_saturation == range_set.saturation_range_max ? UINT16_MAX : 0;
        /* response to Move Set message is sent with unknown transition time value, if given
         * transition time is non-zero. */
        p_out->remaining_time_ms = (p_params->transition_time_ms == 0 || p_in->delta == 0) ?
            0 : MODEL_TRANSITION_TIME_UNKNOWN;
    }
}



/** values transition routines  */

/* set transition options */
static void transition_parameters_set(app_light_hsl_setup_server_t *p_app,
                                      app_transition_t *transition,
                                      int32_t delta,
                                      const model_transition_t *p_in_transition,
                                      app_transition_type_t transition_type)
{
    app_transition_params_t *p_params = app_transition_requested_get(transition);

    p_params->transition_type = transition_type;
    p_params->required_delta  = delta;
    p_params->minimum_step_ms = TRANSITION_STEP_MIN_MS;

    /* requirement: If transition time parameters are unavailable and default transition time state
     * is not available, transition shall be instantaneous */
    if (p_in_transition == NULL) {
        transition->delay_ms = 0;
        ERROR_CHECK(light_lightness_mc_dtt_state_get(p_app->p_app_ll->light_lightness_setup_server.state.handle,
                                                     &p_params->transition_time_ms));
    } else {
        transition->delay_ms = p_in_transition->delay_ms;
        p_params->transition_time_ms = p_in_transition->transition_time_ms;
    }
}


/* stop transition routine on move abort */
static bool m_transition_abort_flag_cb(void)
{
    LIST_FOREACH(p_iter, mp_app_light_hsl_head) {
        app_light_hsl_setup_server_t *p_app =
            PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, node, p_iter);

        if (p_app->abort_move_hue) {
            p_app->abort_move_hue = false;
            app_transition_abort(&p_app->state.transition_hue);
        }

        if (p_app->abort_move_saturation) {
            p_app->abort_move_saturation = false;
            app_transition_abort(&p_app->state.transition_saturation);
        }
    }
    return true;
}



/** hue transition implementation */

static app_light_hsl_setup_server_t *transition_hue_to_app(const app_transition_t *p_transition)
{
    app_light_hsl_state_t *p_state =
        PARENT_BY_FIELD_GET(app_light_hsl_state_t, transition_hue, p_transition);
    return PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, state, p_state);
}


static inline bool hue_limit_check(uint16_t present_hue,
                                   light_hsl_range_set_params_t *p_range,
                                   int32_t required_delta)
{
    /* return true, if the hue present value is at the range limits */
    return ((required_delta > 0 && present_hue == p_range->hue_range_max) ||
            (required_delta < 0 && present_hue == p_range->hue_range_min));
}


static void transition_hue_delay_start_cb(const app_transition_t *p_transition)
{
#if NRF_LOG_ENABLED
    app_light_hsl_setup_server_t *p_app = transition_hue_to_app(p_transition);

    NRF_LOG_INFO("%s(): element %d: starting delay",
        __func__, p_app->light_hsl_setup_srv.settings.element_index);
#endif
}


static void transition_hue_start_cb(const app_transition_t *p_transition)
{
    app_light_hsl_setup_server_t *p_app = transition_hue_to_app(p_transition);
    app_transition_params_t *p_params = app_transition_ongoing_get(&p_app->state.transition_hue);

    if ((p_app->state.transition_hue.requested_params.transition_type == APP_TRANSITION_TYPE_DELTA_SET &&
                p_app->state.new_tid_hue) ||
            (p_app->state.transition_hue.requested_params.transition_type == APP_TRANSITION_TYPE_SET) ||
            (p_app->state.transition_hue.requested_params.transition_type == APP_TRANSITION_TYPE_MOVE_SET)) {
        p_app->state.initial_present_hue = p_app->state.snapshot_present_hue;
    }
    p_app->state.target_hue = p_app->state.snapshot_target_hue;

    NRF_LOG_INFO("%s(): element %d: starting transition: init H: %d delta: %d tt: %d",
          __func__,
          p_app->light_hsl_setup_srv.settings.element_index,
          p_app->state.initial_present_hue,
          p_params->required_delta,
          p_params->transition_time_ms);

    p_app->state.published_hue_ms = 0;

    if (p_app->app_light_hsl_transition_cb != NULL) {
        app_light_hsl_hw_state_t target_hsl_state;
        target_hsl_state.hue = p_app->state.target_hue;
        target_hsl_state.saturation = p_app->state.target_saturation;
        p_app->app_light_hsl_transition_cb(p_app, p_params->transition_time_ms, target_hsl_state);
    }
}


static void transition_hue_tick_cb(const app_transition_t *p_transition)
{
    app_light_hsl_setup_server_t *p_app;
    app_transition_params_t *p_params;
    int32_t total_time_ms;
    uint32_t elapsed_ms;
    uint32_t remaining_ms;
    int32_t delta;
    int32_t present_hue;
    light_hsl_range_set_params_t range;

    p_app = transition_hue_to_app(p_transition);
    p_params = app_transition_ongoing_get(&p_app->state.transition_hue);

    /* putting the uint32_t into an int32 for signed integer division
       no data is lost since in @tagMeshMdlSp section 3.1.3 gives the maximum
       transition time of 10.5 hours (that is < 38 million ms,
       which is well below 31 bits) */
    total_time_ms = p_params->transition_time_ms;
    elapsed_ms = app_transition_elapsed_time_get((app_transition_t *)p_transition);
    remaining_ms = total_time_ms - elapsed_ms;

    if (p_params->transition_type != APP_TRANSITION_TYPE_MOVE_SET) {
        /* lightness is handled in the other state machine */
        /* calculate new values using linear interpolation and provide to the application */
        delta = (int32_t)p_app->state.target_hue - (int32_t)p_app->state.initial_present_hue;
        present_hue = p_app->state.initial_present_hue +
            ((int64_t)delta * (int64_t)elapsed_ms / total_time_ms);
    } else {
        delta = p_params->required_delta;
        present_hue = p_app->state.initial_present_hue +
           (((int64_t)elapsed_ms * (int64_t)delta) /
                (int64_t)p_params->transition_time_ms);
    }

    ERROR_CHECK(light_hsl_mc_range_state_get(p_app->light_hsl_setup_srv.state.handle, &range));
    p_app->state.present_hue = present_hue == 0 ? 0 :
                               MIN((int32_t)MAX(present_hue, range.hue_range_min), range.hue_range_max);
    present_hsl_values_set(p_app, &range);

    /* abort move transitionn when the hue limit is reached */
    if (p_params->transition_type == APP_TRANSITION_TYPE_MOVE_SET &&
            hue_limit_check(p_app->state.present_hue, &range, p_params->required_delta)) {
        p_app->abort_move_hue = true;
        bearer_event_flag_set(m_transition_abort_flag);
    }

    /* while the transition has at least one half second remaining, publish intermediate HSL state
     * at one second intervals. */
    if ((remaining_ms > HSL_ADDITIONAL_PUBLISH_START_MARGIN_MS) &&
            (elapsed_ms - p_app->state.published_hue_ms > HSL_ADDITIONAL_PUBLISH_INTERVAL_MS)) {
        p_app->state.published_hue_ms = elapsed_ms;

        /* ignore status, as publish may fail due to several reasons and it is ok. */
        (void) hsl_publish(p_app, remaining_ms, PUBLISH_HUE);
    }
}


static void transition_hue_complete_cb(const app_transition_t *p_transition)
{
    app_light_hsl_setup_server_t *p_app;
    app_transition_params_t *p_params;
    light_hsl_range_set_params_t range;

    p_app = transition_hue_to_app(p_transition);
    p_params = app_transition_ongoing_get(&p_app->state.transition_hue);

    /* restore target from snapshots */
    p_app->state.target_hue = p_app->state.snapshot_target_hue;

    NRF_LOG_INFO("%s(): element %d: transition completed",
        __func__, p_app->light_hsl_setup_srv.settings.element_index);

    if (p_params->transition_type != APP_TRANSITION_TYPE_MOVE_SET) {
        p_app->state.present_hue = p_app->state.target_hue;
    }

    /* restrict and set present HSL Hue value to App */
    ERROR_CHECK(light_hsl_mc_range_state_get(p_app->light_hsl_setup_srv.state.handle, &range));
    present_hsl_values_set(p_app, &range);

    /* transition is complete */
    if (p_app->app_light_hsl_transition_cb != NULL) {
        app_light_hsl_hw_state_t target_hsl_state;
        target_hsl_state.hue = p_app->state.target_hue;
        target_hsl_state.saturation = p_app->state.target_saturation;
        p_app->app_light_hsl_transition_cb(p_app,
                    p_params->transition_time_ms,
                    target_hsl_state);
    }

    ERROR_CHECK(light_hsl_mc_hue_state_set(p_app->light_hsl_setup_srv.state.handle, p_app->state.present_hue));

    /* ignore status, as publish may fail due to several reasons and it is ok */
    (void) hsl_publish(p_app, 0, PUBLISH_HUE);

    if (p_app->hsl_state_saturation_set_active == false) {
        p_app->hsl_state_set_active = false;
    }
    p_app->hsl_state_hue_set_active = false;
}



/** saturation transition implementation */

static app_light_hsl_setup_server_t *transition_saturation_to_app(const app_transition_t *p_transition)
{
    app_light_hsl_state_t *p_state =
        PARENT_BY_FIELD_GET(app_light_hsl_state_t, transition_saturation, p_transition);
    return PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, state, p_state);
}


static inline bool saturation_limit_check(uint16_t present_saturation,
                                          light_hsl_range_set_params_t *p_range,
                                          int32_t required_delta)
{
    /* return true, if the hue present value is at the range limits */
    return ((required_delta > 0 && present_saturation == p_range->saturation_range_max) ||
            (required_delta < 0 && present_saturation == p_range->saturation_range_min));
}


static void transition_saturation_delay_start_cb(const app_transition_t *p_transition)
{
#if NRF_LOG_ENABLED
    app_light_hsl_setup_server_t *p_app = transition_saturation_to_app(p_transition);

    NRF_LOG_INFO("%s(): element %d: starting delay",
        __func__, p_app->light_hsl_setup_srv.settings.element_index);
#endif
}


static void transition_saturation_start_cb(const app_transition_t *p_transition)
{
    app_light_hsl_setup_server_t *p_app = transition_saturation_to_app(p_transition);
    app_transition_params_t *p_params = app_transition_ongoing_get(&p_app->state.transition_saturation);

    if ((p_app->state.transition_saturation.requested_params.transition_type == APP_TRANSITION_TYPE_DELTA_SET &&
                p_app->state.new_tid_saturation) ||
            (p_app->state.transition_saturation.requested_params.transition_type == APP_TRANSITION_TYPE_SET) ||
            (p_app->state.transition_saturation.requested_params.transition_type == APP_TRANSITION_TYPE_MOVE_SET)) {
        p_app->state.initial_present_saturation = p_app->state.snapshot_present_saturation;
    }
    p_app->state.target_saturation = p_app->state.snapshot_target_saturation;

    NRF_LOG_INFO("%s(): element %d: starting transition: init S: %d delta: %d tt: %d",
          __func__,
          p_app->light_hsl_setup_srv.settings.element_index,
          p_app->state.initial_present_saturation,
          p_params->required_delta,
          p_params->transition_time_ms);

    p_app->state.published_saturation_ms = 0;

    if (p_app->app_light_hsl_transition_cb != NULL) {
        app_light_hsl_hw_state_t target_hsl_state;
        target_hsl_state.hue = p_app->state.target_hue;
        target_hsl_state.saturation = p_app->state.target_saturation;
        p_app->app_light_hsl_transition_cb(p_app, p_params->transition_time_ms, target_hsl_state);
    }
}


static void transition_saturation_tick_cb(const app_transition_t *p_transition)
{
    app_light_hsl_setup_server_t *p_app;
    app_transition_params_t *p_params;
    uint32_t elapsed_ms;
    uint32_t remaining_ms;
    int32_t present_saturation;
    int32_t delta;
    int32_t total_time_ms;
    light_hsl_range_set_params_t range;

    p_app = transition_saturation_to_app(p_transition);
    p_params = app_transition_ongoing_get(&p_app->state.transition_saturation);

    /* putting the uint32_t into an int32 for signed integer division
     * no data is lost since in @tagMeshMdlSp section 3.1.3 gives the maximum
     * transition time of 10.5 hours (that is < 38 million ms,
     * which is well below 31 bits) */
    total_time_ms = p_params->transition_time_ms;
    elapsed_ms = app_transition_elapsed_time_get((app_transition_t *)p_transition);
    remaining_ms = total_time_ms - elapsed_ms;

    if (p_params->transition_type != APP_TRANSITION_TYPE_MOVE_SET) {
        delta =
            ((int32_t)p_app->state.target_saturation - (int32_t)p_app->state.initial_present_saturation);
        present_saturation = p_app->state.initial_present_saturation +
            ((int64_t)delta * (int64_t)elapsed_ms / total_time_ms);
    } else  {
        delta = p_params->required_delta;
        present_saturation = p_app->state.initial_present_saturation +
            (((int64_t)elapsed_ms * delta) / (int64_t)p_params->transition_time_ms);
    }

    ERROR_CHECK(light_hsl_mc_range_state_get(p_app->light_hsl_setup_srv.state.handle, &range));
    p_app->state.present_saturation = present_saturation == 0 ? 0 :
                                      MIN((int32_t)MAX(present_saturation, range.saturation_range_min), range.saturation_range_max);
    present_hsl_values_set(p_app, &range);

    /* abort move transition when the saturation limit is reached */
    if (p_params->transition_type == APP_TRANSITION_TYPE_MOVE_SET &&
            saturation_limit_check(p_app->state.present_saturation, &range, p_params->required_delta)) {
        p_app->abort_move_saturation = true;
        bearer_event_flag_set(m_transition_abort_flag);
    }

    /* while the transition has at least one half second remaining, publish intermediate HSL state
     * at one second intervals */
    if ((remaining_ms > HSL_ADDITIONAL_PUBLISH_START_MARGIN_MS) &&
        (elapsed_ms - p_app->state.published_saturation_ms > HSL_ADDITIONAL_PUBLISH_INTERVAL_MS)) {
        p_app->state.published_saturation_ms = elapsed_ms;

        /* Ignore status, as publish may fail due to several reasons and it is ok. */
        (void) hsl_publish(p_app, remaining_ms, PUBLISH_SATURATION);
    }
}


static void transition_saturation_complete_cb(const app_transition_t *p_transition)
{
    app_light_hsl_setup_server_t *p_app;
    app_transition_params_t *p_params;
    light_hsl_range_set_params_t range;

    p_app = transition_saturation_to_app(p_transition);
    p_params = app_transition_ongoing_get(&p_app->state.transition_saturation);

    /* update present and target */
    p_app->state.target_saturation = p_app->state.snapshot_target_saturation;
    if (p_params->transition_type != APP_TRANSITION_TYPE_MOVE_SET) {
        p_app->state.present_saturation = p_app->state.target_saturation;
    }

    NRF_LOG_INFO("%s(): element %d: transition completed",
        __func__, p_app->light_hsl_setup_srv.settings.element_index);

    /* restrict and set present HSL Saturation value to App */
    ERROR_CHECK(light_hsl_mc_range_state_get(p_app->light_hsl_setup_srv.state.handle, &range));
    present_hsl_values_set(p_app, &range);

    /* transition is complete */
    if (p_app->app_light_hsl_transition_cb != NULL) {
        app_light_hsl_hw_state_t target_hsl_state;
        target_hsl_state.hue = p_app->state.target_hue;
        target_hsl_state.saturation = p_app->state.target_saturation;
        p_app->app_light_hsl_transition_cb(p_app,
                    p_params->transition_time_ms,
                    target_hsl_state);
    }

    ERROR_CHECK(light_hsl_mc_saturation_state_set(p_app->light_hsl_setup_srv.state.handle,
                                                  p_app->state.present_saturation));

    /* ignore status, as publish may fail due to several reasons and it is ok */
    (void) hsl_publish(p_app, 0, PUBLISH_SATURATION);

    if (p_app->hsl_state_hue_set_active == false) {
        p_app->hsl_state_set_active = false;
    }
    p_app->hsl_state_saturation_set_active = false;
}



/*** scene functions */

#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0

static void app_light_hsl_scene_store(const app_scene_model_interface_t *p_app_scene_if,
                                      uint8_t scene_index)
{
    app_light_hsl_setup_server_t *p_app =
                PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, scene_if, p_app_scene_if);
    ERROR_CHECK(light_hsl_mc_scene_hue_state_store(
                    p_app->light_hsl_setup_srv.state.handle,
                    scene_index,
                    p_app->state.present_hue));
    ERROR_CHECK(light_hsl_mc_scene_saturation_state_store(
                    p_app->light_hsl_setup_srv.state.handle,
                    scene_index,
                    p_app->state.present_saturation));

    NRF_LOG_INFO("%s(): %d  scene index: %d  present hue: %d  present saturation: %d",
            __func__,
            p_app->light_hsl_setup_srv.state.handle,
            scene_index,
            p_app->state.present_hue,
            p_app->state.present_saturation);
}


static void app_light_hsl_scene_recall(const app_scene_model_interface_t *p_app_scene_if,
                                       uint8_t scene_index,
                                       uint32_t delay_ms,
                                       uint32_t transition_time_ms)
{
    app_light_hsl_setup_server_t *p_app;
    app_light_hsl_hw_state_t present_hsl_state;
    uint16_t recall_hue, recall_saturation;


    p_app = PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, scene_if, p_app_scene_if);

    /* set a module flag to let the code know there is a CTL state set occurring */
    p_app->hsl_state_set_active = true;

    p_app->app_light_hsl_get_cb(p_app, &present_hsl_state);

    ERROR_CHECK(light_hsl_mc_scene_hue_state_recall(p_app->light_hsl_setup_srv.state.handle,
                                                    scene_index, &recall_hue));
    ERROR_CHECK(light_hsl_mc_scene_saturation_state_recall(p_app->light_hsl_setup_srv.state.handle,
                                                    scene_index, &recall_saturation));

    p_app->state.snapshot_target_hue = recall_hue;
    p_app->state.snapshot_present_hue = present_hsl_state.hue;
    p_app->state.snapshot_target_saturation = recall_saturation;
    p_app->state.snapshot_present_saturation = present_hsl_state.saturation;


    // ToDo: check for current OnPowerOn status
    ERROR_CHECK(light_hsl_mc_hue_state_set(p_app->light_hsl_setup_srv.state.handle,
                                           p_app->state.snapshot_target_hue));
    ERROR_CHECK(light_hsl_mc_saturation_state_set(p_app->light_hsl_setup_srv.state.handle,
                                           p_app->state.snapshot_target_saturation));

    model_transition_t in_transition = {.delay_ms = delay_ms,
                                        .transition_time_ms = transition_time_ms};

    if (!p_app->light_hsl_setup_srv.state.initialized || (present_hsl_state.hue != recall_hue)) {
        app_transition_abort(&p_app->state.transition_hue);
        int32_t delta = (int32_t)p_app->state.snapshot_target_hue - (int32_t)present_hsl_state.hue;
        transition_parameters_set(p_app, &p_app->state.transition_hue,
                                  delta, &in_transition, APP_TRANSITION_TYPE_SET);

        app_transition_params_t *p_params = app_transition_requested_get(&p_app->state.transition_hue);
        NRF_LOG_INFO("%s(): hue: %d, delay: %d, tt: %d, req-delta: %d", __func__,
              recall_hue,
              p_app->state.transition_hue.delay_ms,
              p_params->transition_time_ms,
              p_params->required_delta);

        app_transition_trigger(&p_app->state.transition_hue);
    }

    if (!p_app->light_hsl_setup_srv.state.initialized || (present_hsl_state.saturation != recall_saturation)) {
        app_transition_abort(&p_app->state.transition_saturation);
        int32_t delta = (int32_t)p_app->state.snapshot_target_saturation - (int32_t)present_hsl_state.saturation;
        transition_parameters_set(p_app, &p_app->state.transition_saturation,
                                  delta, &in_transition, APP_TRANSITION_TYPE_SET);

        app_transition_params_t *p_params = app_transition_requested_get(&p_app->state.transition_saturation);
        NRF_LOG_INFO("%s(): saturation: %d, delay: %d, tt: %d, req-delta: %d", __func__,
              recall_saturation,
              p_app->state.transition_saturation.delay_ms,
              p_params->transition_time_ms,
              p_params->required_delta);

        app_transition_trigger(&p_app->state.transition_saturation);
    }
}


static void app_light_hsl_scene_delete(const app_scene_model_interface_t *p_app_scene_if,
                                       uint8_t scene_index)
{
    app_light_hsl_setup_server_t *p_app =
                PARENT_BY_FIELD_GET(app_light_hsl_setup_server_t, scene_if, p_app_scene_if);
    app_transition_abort(&p_app->state.transition_hue);
    app_transition_abort(&p_app->state.transition_saturation);
}

#endif /* SCENE_SETUP_SERVER_INSTANCES_MAX > 0 */



/*** interface functions */

uint32_t app_light_hsl_model_init(app_light_hsl_setup_server_t *p_app, uint8_t element_index,
                                  app_light_lightness_setup_server_t *p_app_ll)
{
    uint32_t status;

    if (p_app == NULL || p_app_ll == NULL) {
        return NRF_ERROR_NULL;
    }

    if (p_app->app_light_hsl_set_cb == NULL || 
            ((p_app->app_light_hsl_get_cb == NULL) && 
                (p_app->app_light_hsl_transition_cb == NULL))) {
        return NRF_ERROR_NULL;
    }

    p_app->p_app_ll = p_app_ll;

    /* set up the added callback from light lightness to HSL */
    p_app->p_app_ll->app_add_notify.p_app_publish_v = p_app;
    p_app->p_app_ll->app_add_notify.app_add_publish_cb = ll_add_publish_cb;

    /* initialize HSL Setup server */
    status = app_hsl_setup_server_init(p_app, element_index);
    if (status != NRF_SUCCESS) {
        return status;
    }

#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
    p_app->scene_if.p_callbacks = &m_scene_light_hsl_cbs;
#endif

    /* transition for hue value */
    p_app->state.transition_hue.delay_start_cb = transition_hue_delay_start_cb;
    p_app->state.transition_hue.transition_start_cb = transition_hue_start_cb;
    p_app->state.transition_hue.transition_tick_cb = transition_hue_tick_cb;
    p_app->state.transition_hue.transition_complete_cb = transition_hue_complete_cb;
    p_app->state.transition_hue.p_context = &p_app->state.transition_hue;
    status = app_transition_init(&p_app->state.transition_hue);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* transition for saturation value */
    p_app->state.transition_saturation.delay_start_cb = transition_saturation_delay_start_cb;
    p_app->state.transition_saturation.transition_start_cb = transition_saturation_start_cb;
    p_app->state.transition_saturation.transition_tick_cb = transition_saturation_tick_cb;
    p_app->state.transition_saturation.transition_complete_cb = transition_saturation_complete_cb;
    p_app->state.transition_saturation.p_context = &p_app->state.transition_saturation;
    status = app_transition_init(&p_app->state.transition_saturation);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* hue/saturation move transaction abort */
    p_app->abort_move_hue = false;
    p_app->abort_move_saturation = false;
    list_add(&mp_app_light_hsl_head, &p_app->node);
    m_transition_abort_flag = bearer_event_flag_add(m_transition_abort_flag_cb);

    return NRF_SUCCESS;
}


uint32_t app_light_hsl_binding_setup(app_light_hsl_setup_server_t *p_app)
{
    light_hsl_saved_values_t state;
    uint16_t index;
    uint16_t ll_index;

    if (p_app == NULL) {
        return NRF_ERROR_NULL;
    }

    index = p_app->light_hsl_setup_srv.state.handle;
    ll_index = p_app->p_app_ll->light_lightness_setup_server.state.handle;

    ERROR_CHECK(light_lightness_mc_onpowerup_state_get(ll_index, &state.onpowerup));
    ERROR_CHECK(light_hsl_mc_hue_state_get(index, &state.hue));
    ERROR_CHECK(light_hsl_mc_saturation_state_get(index, &state.saturation));
    ERROR_CHECK(light_hsl_mc_default_hue_state_get(index, &state.default_hue));
    ERROR_CHECK(light_hsl_mc_default_saturation_state_get(index, &state.default_saturation));
    ERROR_CHECK(light_hsl_mc_range_state_get(index, &state.range));

    return light_hsl_ponoff_binding_setup(&p_app->light_hsl_setup_srv, &state);
}


uint32_t app_light_hsl_current_value_publish(app_light_hsl_setup_server_t *p_app)
{
    app_light_hsl_hw_state_t present_hsl_state;
    uint16_t lightness;

    if (p_app == NULL) {
        return NRF_ERROR_NULL;
    }

    /* stop any transitions on hue, saturation, and lightness */
    app_transition_abort(&p_app->state.transition_hue);
    app_transition_abort(&p_app->state.transition_saturation);
    app_transition_abort(&p_app->p_app_ll->state.transition);

#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
    uint16_t old_present_hue = p_app->state.present_hue;
    uint16_t old_present_saturation = p_app->state.present_saturation;
#endif

    p_app->app_light_hsl_get_cb(p_app, &present_hsl_state);
    p_app->p_app_ll->app_light_lightness_get_cb(p_app->p_app_ll, &lightness);

    p_app->state.present_hue = present_hsl_state.hue;
    p_app->state.present_saturation = present_hsl_state.saturation;
    p_app->p_app_ll->state.present_lightness = lightness;

    /* set persistent memory for HSL and light lightness */
    ERROR_CHECK(light_hsl_mc_hue_state_set(p_app->light_hsl_setup_srv.state.handle,
                                           p_app->state.present_hue));
    ERROR_CHECK(light_hsl_mc_saturation_state_set(p_app->light_hsl_setup_srv.state.handle,
                                                  p_app->state.present_saturation));

    if (lightness >= LIGHT_LIGHTNESS_LAST_MIN) {
        ERROR_CHECK(light_lightness_mc_last_state_set(p_app->p_app_ll->light_lightness_setup_server.state.handle,
                                                      lightness,
                                                      LIGHT_LIGHTNESS_MC_WRITE_DESTINATION_ALL));
    }
    ERROR_CHECK(light_lightness_mc_actual_state_set(p_app->p_app_ll->light_lightness_setup_server.state.handle,
                                                    lightness));

#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
    if ((old_present_hue != p_app->state.present_hue) ||
            (old_present_saturation != p_app->state.present_saturation)) {
        app_scene_model_scene_changed(p_app->p_app_scene);
    }
#endif

    return hsl_publish(p_app, 0, PUBLISH_HSL);
}



/*** scene interface functions */

#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0

uint32_t app_light_hsl_scene_context_set(app_light_hsl_setup_server_t *p_app,
                                         app_scene_setup_server_t *p_app_scene)
{
    if (p_app == NULL || p_app_scene == NULL) {
        return NRF_ERROR_NULL;
    }

    p_app->p_app_scene = p_app_scene;
    return NRF_SUCCESS;
}

#endif /* SCENE_SETUP_SERVER_INSTANCES_MAX > 0 */
