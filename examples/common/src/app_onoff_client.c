#include "app_onoff_client.h"

#include <stdint.h>
#include <string.h>

#include "app_timer.h"
#include "app_error.h"
#include "utils.h"
#include "mesh_app_utils.h"
#include "sdk_config.h"
#include "generic_onoff_client.h"

#include "nrf5_sdk_log.h"



/* forward declarations */
static void app_onoff_client_task_stop(app_onoff_client_t *app);

static void app_gen_onoff_client_publish_interval_cb(access_model_handle_t handle, void * p_self);
static void app_gen_onoff_client_transaction_status_cb(access_model_handle_t model_handle,
                                                       void * p_args,
                                                       access_reliable_status_t status);
static void app_generic_onoff_client_status_cb(const generic_onoff_client_t *p_self,
                                               const access_message_rx_meta_t *p_meta,
                                               const generic_onoff_status_params_t *p_in);

static void app_onoff_client_single_get(app_onoff_client_t *app);

static void app_onoff_client_toggle_get(app_onoff_client_t *app);
static void app_onoff_client_toggle_set(app_onoff_client_t *app);


/* APP TIMER pattern */
APP_TIMER_DEF(m_onoff_client_timer);


/* Generic OnOff client callbacks */
const generic_onoff_client_callbacks_t onoff_client_cbs = {
    .onoff_status_cb = app_generic_onoff_client_status_cb,
    .ack_transaction_status_cb = app_gen_onoff_client_transaction_status_cb,
    .periodic_publish_cb = app_gen_onoff_client_publish_interval_cb
};



/* this callback is called periodically if model is configured for periodic publishing */
static void app_gen_onoff_client_publish_interval_cb(access_model_handle_t handle, void *p_self)
{
    app_onoff_client_t *app = PARENT_BY_FIELD_GET(app_onoff_client_t, client, p_self);

    if (app->state == ONOFF_CLIENT_IDLE) {
        NRF_LOG_INFO("%s(): publish OnOff state", __func__);

        app_onoff_client_single_get(app);
    }
}


/* acknowledged transaction status callback */
static void app_gen_onoff_client_transaction_status_cb(access_model_handle_t model_handle,
                                                       void *p_args,
                                                       access_reliable_status_t status)
{
    app_onoff_client_t *app = PARENT_BY_FIELD_GET(app_onoff_client_t, client, p_args);

    if (status != ACCESS_RELIABLE_TRANSFER_SUCCESS) {
        NRF_LOG_INFO("%s(): acknowledged transfer fail, state: %d", __func__, app->state);

        app_onoff_client_task_stop(app);
    }
}


/* processing received status message */
static void app_generic_onoff_client_status_cb(const generic_onoff_client_t *p_self,
                                               const access_message_rx_meta_t *p_meta,
                                               const generic_onoff_status_params_t *p_in)
{
    app_onoff_client_t *app = PARENT_BY_FIELD_GET(app_onoff_client_t, client, p_self);
    bool onoff_status;

    if (p_in->remaining_time_ms == 0) {
        onoff_status = p_in->present_on_off;

        NRF_LOG_INFO("%s(): received get status value: (%04x->%04x) %d, state: %d", __func__, 
            p_meta->src.value, p_meta->dst.value, onoff_status, app->state);

        switch (app->state) {
        case ONOFF_CLIENT_IDLE:
            app->onoff_state = onoff_status;
            app->status_cb(app, onoff_status);
            break;

        case ONOFF_CLIENT_SINGLE_GET_SEND:
            app->onoff_state = onoff_status;
            app->status_cb(app, onoff_status);
            app_onoff_client_task_stop(app);
            break;

        case ONOFF_CLIENT_TOGGLE_GET_SEND:
            app->onoff_state = onoff_status;
            app->state = ONOFF_CLIENT_TOGGLE_SET;
            APP_ERROR_CHECK(app_timer_start(app->timer_id, APP_TIMER_MIN_TIMEOUT_TICKS, app));
            break;

        case ONOFF_CLIENT_TOGGLE_SET_SEND:
            app->onoff_state = onoff_status;
            app->status_cb(app, onoff_status);
            app_onoff_client_task_stop(app);
            break;

        default:
            app_onoff_client_task_stop(app);
        }
    }
}


/* runtime context isolation delay */
static void app_onoff_client_delay_cb(void * p_context)
{
    app_onoff_client_t *app = (app_onoff_client_t *)p_context;

    NRF_LOG_INFO("%s(): client delay timer, state: %d", __func__, app->state);

    switch(app->state) {
    case ONOFF_CLIENT_SINGLE_GET_START:
        app_onoff_client_single_get(app);
        break;

    case ONOFF_CLIENT_TOGGLE_START:
        app_onoff_client_toggle_get(app);
        break;

    case ONOFF_CLIENT_TOGGLE_SET:
        app_onoff_client_toggle_set(app);
        break;

    default:
        // do nothing
        break;
    }
}


static void app_onoff_client_task_stop(app_onoff_client_t *app)
{
    if (app->state != ONOFF_CLIENT_IDLE) {
        access_model_reliable_cancel(app->client.model_handle);
        app_timer_stop(app->timer_id);
        app->state = ONOFF_CLIENT_IDLE;
    }
}



/***** Get task functions *****/

static void app_onoff_client_single_get(app_onoff_client_t *app)
{
    uint32_t status;

    access_model_reliable_cancel(app->client.model_handle);
    status = generic_onoff_client_get(&app->client);
    if (status == NRF_SUCCESS) {
        app->state = ONOFF_CLIENT_SINGLE_GET_SEND;
    } else {
        app->state = ONOFF_CLIENT_IDLE;
        NRF_LOG_INFO("%s(): failed to send get status", __func__);
    }
}



/***** Toggle task functions *****/

static void app_onoff_client_toggle_get(app_onoff_client_t *app)
{
    uint32_t status;

    access_model_reliable_cancel(app->client.model_handle);
    status = generic_onoff_client_get(&app->client);
    if (status == NRF_SUCCESS) {
        app->state = ONOFF_CLIENT_TOGGLE_GET_SEND;
    } else {
        app->state = ONOFF_CLIENT_IDLE;
        NRF_LOG_INFO("%s(): failed to send get status", __func__);
    }
}


static void app_onoff_client_toggle_set(app_onoff_client_t *app)
{
    uint32_t status;
    generic_onoff_set_params_t set_params;

    set_params.tid = app->tid++;
    set_params.on_off = app->onoff_state ? false : true;

    NRF_LOG_INFO("%s(): OnOff set %d", __func__, set_params.on_off);

    access_model_reliable_cancel(app->client.model_handle);
    status = generic_onoff_client_set(&app->client, &set_params, NULL);
    if (status == NRF_SUCCESS) {
        app->state = ONOFF_CLIENT_TOGGLE_SET_SEND;
    } else {
        app->state = ONOFF_CLIENT_IDLE;
        NRF_LOG_INFO("%s(): failed to sent set message", __func__);
    }
}




/***** Interface functions *****/

uint32_t app_onoff_client_init(app_onoff_client_t *app,
                               uint8_t element_index,
                               app_onoff_client_status_cb_t status_cb)
{
    const app_timer_id_t m_timer __attribute__((unused)) = m_onoff_client_timer;
    uint32_t status;

    app->state = ONOFF_CLIENT_IDLE;

    app->status_cb = status_cb;

    app->client.settings.p_callbacks = &onoff_client_cbs;
    app->client.settings.timeout = 0;
    status = generic_onoff_client_init(&app->client, element_index);
    if (status != NRF_SUCCESS) {
        return status;
    }

    memcpy(&app->timer_id_data, &m_onoff_client_timer_data, sizeof(m_onoff_client_timer_data));
    app->timer_id = &app->timer_id_data;
    APP_ERROR_CHECK(app_timer_create(&app->timer_id, APP_TIMER_MODE_SINGLE_SHOT, app_onoff_client_delay_cb));

    return NRF_SUCCESS;
}


void app_onoff_client_get(app_onoff_client_t *app)
{
    NRF_LOG_INFO("---- %s: start sigle get task, client handle: %d ----", __func__, app->client.model_handle);

    app_onoff_client_task_stop(app);
    app->state = ONOFF_CLIENT_SINGLE_GET_START;
    APP_ERROR_CHECK(app_timer_start(app->timer_id, APP_TIMER_MIN_TIMEOUT_TICKS, app));
}


void app_onoff_client_toggle(app_onoff_client_t *app)
{
    NRF_LOG_INFO("---- %s: start toggle task, client handle: %d ----", __func__, app->client.model_handle);

    app_onoff_client_task_stop(app);
    app->state = ONOFF_CLIENT_TOGGLE_START;
    APP_ERROR_CHECK(app_timer_start(app->timer_id, APP_TIMER_MIN_TIMEOUT_TICKS, app));
}
