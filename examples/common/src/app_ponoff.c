#include "app_ponoff.h"

#include "utils.h"
#include "mesh_app_utils.h"
#include "sdk_config.h"
#include "example_common.h"
#include "generic_onoff_server.h"
#include "generic_onoff_mc.h"
#include "generic_ponoff_setup_server.h"
#include "generic_ponoff_mc.h"
#include "generic_dtt_server.h"
#include "generic_dtt_mc.h"
#include "app_transition.h"

#include "log.h"



/** Generic Power OnOff Setup server implementation.
 */


/* Forward declaration */

static void generic_ponoff_state_set_cb(const generic_ponoff_setup_server_t * p_self,
                                        const access_message_rx_meta_t * p_meta,
                                        const generic_ponoff_set_params_t * p_in,
                                        generic_ponoff_status_params_t * p_out);
static void generic_ponoff_state_get_cb(const generic_ponoff_setup_server_t * p_self,
                                        const access_message_rx_meta_t * p_meta,
                                        generic_ponoff_status_params_t * p_out);


static void generic_onoff_state_get_cb(const generic_onoff_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       generic_onoff_status_params_t * p_out);
static void generic_onoff_state_set_cb(const generic_onoff_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       const generic_onoff_set_params_t * p_in,
                                       const model_transition_t * p_in_transition,
                                       generic_onoff_status_params_t * p_out);

static void generic_dtt_state_get_cb(const generic_dtt_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       generic_dtt_status_params_t * p_out);
static void generic_dtt_state_set_cb(const generic_dtt_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       const generic_dtt_set_params_t * p_in,
                                       generic_dtt_status_params_t * p_out);


#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0

static void app_onoff_scene_store(const app_scene_model_interface_t *p_app_scene_if,
                                  uint8_t scene_index);
static void app_onoff_scene_recall(const app_scene_model_interface_t * p_app_scene_if,
                                   uint8_t scene_index,
                                   uint32_t delay_ms,
                                   uint32_t transition_time_ms);
static void app_onoff_scene_delete(const app_scene_model_interface_t * p_app_scene_if,
                                   uint8_t scene_index);

const app_scene_callbacks_t m_scene_onoff_cbs = {
    .scene_store_cb = app_onoff_scene_store,
    .scene_recall_cb = app_onoff_scene_recall,
    .scene_delete_cb = app_onoff_scene_delete
};

#endif


const generic_ponoff_setup_server_callbacks_t m_ponoff_setup_srv_cbs =
{
    .ponoff_cbs.set_cb = generic_ponoff_state_set_cb,
    .ponoff_cbs.get_cb = generic_ponoff_state_get_cb
};

const generic_onoff_server_callbacks_t m_onoff_srv_cbs =
{
    .onoff_cbs.set_cb = generic_onoff_state_set_cb,
    .onoff_cbs.get_cb = generic_onoff_state_get_cb
};

const generic_dtt_server_callbacks_t m_dtt_cbs =
{
    .dtt_cbs.set_cb = generic_dtt_state_set_cb,
    .dtt_cbs.get_cb = generic_dtt_state_get_cb
};



static void transition_parameters_set(app_ponoff_setup_server_t *p_app,
                                      const model_transition_t * p_in_transition)
{
    app_transition_params_t *p_params = app_transition_requested_get(&p_app->state.transition);

    p_params->transition_type = APP_TRANSITION_TYPE_SET;
    p_params->minimum_step_ms = TRANSITION_STEP_MIN_MS;

    if (p_in_transition == NULL) {
#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
        generic_dtt_status_params_t dtt_params = {0};
        /* For this implementation, the app_ponoff uses the DTT instance available with Scene
         * Setup server */
        p_app->p_app_scene->scene_setup_server.p_gen_dtt_server->settings.p_callbacks->dtt_cbs.get_cb(p_app->p_app_scene->scene_setup_server.p_gen_dtt_server,
                                                                                                      NULL, &dtt_params);
        p_params->transition_time_ms = dtt_params.transition_time_ms;
#else
        p_params->transition_time_ms = p_app->transition_time_ms;
#endif
        p_app->state.transition.delay_ms = 0;
    } else {
        p_app->state.transition.delay_ms = p_in_transition->delay_ms;
        p_params->transition_time_ms = p_in_transition->transition_time_ms;
    }
}


static void onoff_current_value_update(app_ponoff_setup_server_t *p_app)
{
    if (p_app->onoff_set_cb != NULL) {
        p_app->onoff_set_cb(p_app, p_app->state.present_onoff);
    }
}



/***** Generic Power OnOff model interface callbacks *****/

static void generic_ponoff_state_get_cb(const generic_ponoff_setup_server_t * p_self,
                                        const access_message_rx_meta_t * p_meta,
                                        generic_ponoff_status_params_t * p_out)
{
    app_ponoff_setup_server_t *p_app = PARENT_BY_FIELD_GET(app_ponoff_setup_server_t,
                                                           server, p_self);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "msg OnPowerUp GET\n");
    p_out->on_powerup = p_app->state.on_powerup;
}


static void generic_ponoff_state_set_cb(const generic_ponoff_setup_server_t * p_self,
                                        const access_message_rx_meta_t * p_meta,
                                        const generic_ponoff_set_params_t * p_in,
                                        generic_ponoff_status_params_t * p_out)
{
    app_ponoff_setup_server_t *p_app = PARENT_BY_FIELD_GET(app_ponoff_setup_server_t,
                                                           server, p_self);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "msg OnPowerUp SET\n");

    ERROR_CHECK(generic_ponoff_mc_ponoff_state_set(p_app->state_handle, p_in->on_powerup));
    p_app->state.on_powerup = p_in->on_powerup;
    if (p_out != NULL) {
        p_out->on_powerup = p_in->on_powerup;
    }

    if (p_app->state.on_powerup == GENERIC_ON_POWERUP_RESTORE) {
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "store OnPowerUp state\n");
        ERROR_CHECK(generic_onoff_mc_onoff_state_set(
            p_app->server.generic_ponoff_srv.generic_onoff_srv.state_handle,
            p_app->state.target_onoff));
    }
}


/***** Generic OnOff model interface callbacks *****/

static void generic_onoff_state_get_cb(const generic_onoff_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       generic_onoff_status_params_t * p_out)
{
    app_ponoff_setup_server_t *p_app =
        PARENT_BY_FIELD_GET(app_ponoff_setup_server_t,
                            server.generic_ponoff_srv.generic_onoff_srv,
                            p_self);

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "msg OnOff GET\n");

    /* Requirement: Provide the current value of the OnOff state */
    p_app->onoff_get_cb(p_app, &p_app->state.present_onoff);
    p_out->present_on_off = p_app->state.present_onoff;
    p_out->target_on_off = p_app->state.target_onoff;

    /* Requirement: Always report remaining time */
    p_out->remaining_time_ms = app_transition_remaining_time_get(&p_app->state.transition);
}


static void generic_onoff_state_set_cb(const generic_onoff_server_t * p_self,
                                       const access_message_rx_meta_t * p_meta,
                                       const generic_onoff_set_params_t * p_in,
                                       const model_transition_t * p_in_transition,
                                       generic_onoff_status_params_t * p_out)
{
    app_ponoff_setup_server_t *p_app =
        PARENT_BY_FIELD_GET(app_ponoff_setup_server_t,
                            server.generic_ponoff_srv.generic_onoff_srv,
                            p_self);
    bool present_on_off;

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "msg OnOff SET, %d\n", p_in->on_off);

    p_app->onoff_get_cb(p_app, &present_on_off);

    /* Update internal representation of OnOff value, process timing */
    p_app->state.target_onoff = p_in->on_off;
    uint32_t transition_time_ms = 0;
    if (present_on_off != p_in->on_off) {
        transition_parameters_set(p_app, p_in_transition);
        transition_time_ms = app_transition_requested_get(&p_app->state.transition)->transition_time_ms;
        app_transition_trigger(&p_app->state.transition);
#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
        app_scene_model_scene_changed(p_app->p_app_scene);
#endif

        if (p_app->state.on_powerup == GENERIC_ON_POWERUP_RESTORE) {
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "store On/Off state\n");
            ERROR_CHECK(generic_onoff_mc_onoff_state_set(
                p_app->server.generic_ponoff_srv.generic_onoff_srv.state_handle,
                p_app->state.target_onoff));
        }
    }

    /* prepare response */
    if (p_out != NULL) {
        p_out->present_on_off = p_app->state.present_onoff;
        p_out->target_on_off = p_app->state.target_onoff;
        p_out->remaining_time_ms = transition_time_ms;
    }
}


/***** Generic DTT model interface callbacks *****/

void generic_dtt_state_get_cb(const generic_dtt_server_t * p_self,
                              const access_message_rx_meta_t * p_meta,
                              generic_dtt_status_params_t * p_out)
{
    app_ponoff_setup_server_t *p_app =
        PARENT_BY_FIELD_GET(app_ponoff_setup_server_t,
                            server.generic_dtt_srv,
                            p_self);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "msg DTT GET\n");
    p_out->transition_time_ms = p_app->transition_time_ms;
}


void generic_dtt_state_set_cb(const generic_dtt_server_t * p_self,
                              const access_message_rx_meta_t * p_meta,
                              const generic_dtt_set_params_t * p_in,
                              generic_dtt_status_params_t * p_out)
{
    app_ponoff_setup_server_t *p_app =
        PARENT_BY_FIELD_GET(app_ponoff_setup_server_t,
                            server.generic_dtt_srv,
                            p_self);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "msg DTT SET\n");
    p_app->transition_time_ms = p_in->transition_time_ms;
    ERROR_CHECK(generic_dtt_mc_dtt_state_set(p_app->server.generic_dtt_srv.state_handle,
                                             p_app->transition_time_ms));
    if (p_out != NULL) {
        p_out->transition_time_ms = p_app->transition_time_ms;
    }
}



/***** Transition application interface callbacks *****/

static inline app_ponoff_setup_server_t *transition_to_app(const app_transition_t * p_transition)
{
    app_onoff_state_t *p_state = PARENT_BY_FIELD_GET(app_onoff_state_t,
                                  transition, p_transition);
    return PARENT_BY_FIELD_GET(app_ponoff_setup_server_t,
                               state, p_state);
}


static void transition_start_cb(const app_transition_t *p_transition)
{
    app_ponoff_setup_server_t *p_app = transition_to_app(p_transition);
    app_transition_params_t *p_params = app_transition_ongoing_get(&p_app->state.transition);

    if (p_app->state.initial_present_onoff == 0 && p_app->state.target_onoff == 1) {
        p_app->state.present_onoff = p_app->state.target_onoff;
        onoff_current_value_update(p_app);
        generic_onoff_status_params_t status =
        {
            .present_on_off = p_app->state.present_onoff,
            .target_on_off = p_app->state.target_onoff,
        };
        status.remaining_time_ms = app_transition_ongoing_get(&p_app->state.transition)->transition_time_ms;
        (void)generic_onoff_server_status_publish(&p_app->server.generic_ponoff_srv.generic_onoff_srv,
                                                  &status);
    }

    /* Inform the application that transition have started. */
    if (p_app->onoff_transition_cb != NULL) {
        p_app->onoff_transition_cb(p_app, p_params->transition_time_ms, p_app->state.target_onoff);
    }
}


static void transition_complete_cb(const app_transition_t *p_transition)
{
    app_ponoff_setup_server_t *p_app = transition_to_app(p_transition);
    app_transition_params_t *p_params = app_transition_ongoing_get(&p_app->state.transition);

    if (p_app->state.present_onoff != p_app->state.target_onoff) {
        p_app->state.present_onoff = p_app->state.target_onoff;
        onoff_current_value_update(p_app);
    }

    /* Requirement: Status message is published immediately after the state transition ends (see
     * @tagMeshSp section 3.7.6.1.2). */
    generic_onoff_status_params_t status = {
                .present_on_off = p_app->state.present_onoff,
                .target_on_off = p_app->state.target_onoff,
                .remaining_time_ms = 0
            };
    (void)generic_onoff_server_status_publish(&p_app->server.generic_ponoff_srv.generic_onoff_srv,
                                              &status);

    /* Inform the application that the transition is complete */
    if (p_app->onoff_transition_cb != NULL) {
        p_app->onoff_transition_cb(p_app, p_params->transition_time_ms, p_app->state.target_onoff);
    }
}



/***** Scene Interface functions *****/

#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0

static void app_onoff_scene_store(const app_scene_model_interface_t *p_app_scene_if,
                                  uint8_t scene_index)
{
    app_ponoff_setup_server_t *p_app = PARENT_BY_FIELD_GET(app_ponoff_setup_server_t,
                                                           scene_if, p_app_scene_if);
    ERROR_CHECK(generic_onoff_mc_scene_onoff_store(scene_index,
                                                   p_app->server.generic_ponoff_srv.generic_onoff_srv.state_handle,
                                                   p_app->state.present_onoff));
}


static void app_onoff_scene_recall(const app_scene_model_interface_t *p_app_scene_if,
                                   uint8_t scene_index,
                                   uint32_t delay_ms,
                                   uint32_t transition_time_ms)
{
    app_ponoff_setup_server_t *p_app = PARENT_BY_FIELD_GET(app_ponoff_setup_server_t,
                                                           scene_if, p_app_scene_if);
    bool present_on_off;
    p_app->onoff_get_cb(p_app, &present_on_off);

    ERROR_CHECK(generic_onoff_mc_scene_onoff_recall(scene_index,
                                                    p_app->server.generic_ponoff_srv.generic_onoff_srv.state_handle,
                                                    &p_app->state.target_onoff));
    model_transition_t in_transition = {
        .delay_ms = delay_ms,
        .transition_time_ms = transition_time_ms
    };

    if (present_on_off != p_app->state.target_onoff) {
        app_transition_abort(&p_app->state.transition);
        transition_parameters_set(p_app, &in_transition);
        app_transition_trigger(&p_app->state.transition);
    }
}


static void app_onoff_scene_delete(const app_scene_model_interface_t *p_app_scene_if,
                                   uint8_t scene_index)
{
    app_ponoff_setup_server_t *p_app = PARENT_BY_FIELD_GET(app_ponoff_setup_server_t, scene_if, p_app_scene_if);
    app_transition_abort(&p_app->state.transition);
    /* No need to do anything else */
}

#endif /* SCENE_SETUP_SERVER_INSTANCES_MAX > 0 */



/***** Interface functions *****/

void app_ponoff_status_publish_onoff(app_ponoff_setup_server_t * p_app)
{
    app_transition_abort(&p_app->state.transition);

#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
    bool old_present_onoff = p_app->state.present_onoff;
#endif

    p_app->onoff_get_cb(p_app, &p_app->state.present_onoff);
    p_app->state.target_onoff = p_app->state.present_onoff;

    generic_onoff_status_params_t status = {
        .present_on_off = p_app->state.present_onoff,
        .target_on_off = p_app->state.target_onoff,
        .remaining_time_ms = 0
    };
    (void)generic_onoff_server_status_publish(&p_app->server.generic_ponoff_srv.generic_onoff_srv,
                                              &status);

    if (p_app->state.on_powerup == GENERIC_ON_POWERUP_RESTORE) {
        ERROR_CHECK(generic_onoff_mc_onoff_state_set(
            p_app->server.generic_ponoff_srv.generic_onoff_srv.state_handle,
            p_app->state.present_onoff));
    }

#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
    if (old_present_onoff != p_app->state.present_onoff) {
        app_scene_model_scene_changed(p_app->p_app_scene);
    }
#endif
}


uint32_t app_ponoff_init(app_ponoff_setup_server_t * p_app, uint8_t element_index)
{
    uint32_t status = NRF_ERROR_INTERNAL;

    if (p_app == NULL) {
        return NRF_ERROR_NULL;
    }

    if ((p_app->onoff_get_cb == NULL) ||
            ((p_app->onoff_set_cb == NULL) &&
            (p_app->onoff_transition_cb == NULL))) {
        return NRF_ERROR_NULL;
    }

    /* Power OnOff Setup server */
    p_app->server.settings.p_callbacks = &m_ponoff_setup_srv_cbs;

    /* OnOff server */
    p_app->server.generic_ponoff_srv.generic_onoff_srv.settings.p_callbacks = &m_onoff_srv_cbs;

    /* DTT server */
    p_app->server.generic_dtt_srv.settings.p_callbacks = &m_dtt_cbs;

    status = generic_ponoff_setup_server_init(&p_app->server, element_index);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* set Power OnOff default state */
    status = generic_ponoff_mc_open(&p_app->state_handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* set OnOff default state */
    status = generic_onoff_mc_open(&p_app->server.generic_ponoff_srv.generic_onoff_srv.state_handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

    /* set DTT default state */
    status = generic_dtt_mc_open(&p_app->server.generic_dtt_srv.state_handle);
    if (status != NRF_SUCCESS) {
        return status;
    }

#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
    p_app->scene_if.p_callbacks = &m_scene_onoff_cbs;
#endif

    p_app->state.transition.delay_start_cb = NULL;
    p_app->state.transition.transition_start_cb = transition_start_cb;
    p_app->state.transition.transition_tick_cb = NULL;
    p_app->state.transition.transition_complete_cb = transition_complete_cb;
    p_app->state.transition.p_context = &p_app->state.transition;

    /* For any onoff transitions, required_delta is always 1 */
    app_transition_requested_get(&p_app->state.transition)->required_delta = 1;

    return app_transition_init(&p_app->state.transition);
}


/* restore on/off value */
uint32_t app_ponoff_value_restore(app_ponoff_setup_server_t *p_app)
{
    bool onoff;

    /* restore DTT value */
    ERROR_CHECK(generic_dtt_mc_dtt_state_get(p_app->state_handle, &p_app->transition_time_ms));

    /* restore Power OnOff value */
    ERROR_CHECK(generic_ponoff_mc_ponoff_state_get(p_app->state_handle, &p_app->state.on_powerup));

    /* restore OnOff value */
    switch (p_app->state.on_powerup) {
    case GENERIC_ON_POWERUP_OFF:
        onoff = false;
        break;
    case GENERIC_ON_POWERUP_DEFAULT:
        onoff = true;
        break;
    case GENERIC_ON_POWERUP_RESTORE:
        ERROR_CHECK(generic_onoff_mc_onoff_state_get(
                        p_app->server.generic_ponoff_srv.generic_onoff_srv.state_handle,
                        &onoff));
        break;
    default:
        onoff = false;
    }

    return (generic_onoff_server_state_set(&p_app->server.generic_ponoff_srv.generic_onoff_srv, onoff));
}


#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0

uint32_t app_ponoff_scene_context_set(app_ponoff_setup_server_t *p_app,
                                      app_scene_setup_server_t *p_app_scene)
{
    if (p_app == NULL || p_app_scene == NULL) {
        return NRF_ERROR_NULL;
    }

    p_app->p_app_scene = p_app_scene;
    return NRF_SUCCESS;
}

#endif
