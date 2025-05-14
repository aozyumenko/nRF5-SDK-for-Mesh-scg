/* (C) 2024 Alexander Ozumenko
 * St(u)dio of Computer Games (https://stdio.ru/)
 *
 * Simple Time client application implementation.
 */

#include "app_time_client.h"

#include <stdint.h>

//#include "access.h"
//#include "access_config.h"
//#include "app_timer.h"
#include "app_error.h"
#include "utils.h"
#include "mesh_app_utils.h"
#include "sdk_config.h"
#include "time_client.h"

#include "nrf5_sdk_log.h"


// FIXME!!!!!!!!!!!!!!!!!
//#include <time.h>
//extern time_t m_time;


/* time client callbacks */

static void time_status_cb(const time_client_t *p_self,
                           const access_message_rx_meta_t *p_meta,
                           const time_status_param_t *p_in)
{
    app_time_client_t *p_app = PARENT_BY_FIELD_GET(app_time_client_t, client, p_self);

    int tai_utc_delta = p_in->tai_utc_delta - 255;
    int time_zone_offset = (p_in->time_zone_offset - 64) * SECONDS_15_MINUTES;

    // update time
    uint64_t utc_seconds = p_in->TAI_seconds + MESH_UNIX_EPOCH_DIFF + tai_utc_delta;
    uint64_t local_seconds = utc_seconds + time_zone_offset;
//    uint64_t device_seconds = local_seconds - p_app->tz_offset;

    NRF_LOG_INFO("    tai_utc_delta=%d, tai_seconds=%d, utc_seconds=%llu, local_seconds=%d",
            tai_utc_delta, p_in->TAI_seconds + MESH_UNIX_EPOCH_DIFF, utc_seconds, local_seconds);

    p_app->set_cb(p_app, local_seconds);
}


/* callback type for Time Role state related transactions */
static void time_role_status_cb(const time_client_t *p_self,
                                const access_message_rx_meta_t *p_meta,
                                const time_role_status_param_t *p_in)
{
    /* do nothing */
}


/* callback type for Time Zone state related transactions */
static void time_zone_status_cb(const time_client_t *p_self,
                                const access_message_rx_meta_t *p_meta,
                                const time_zone_status_param_t *p_in)
{
    /* do nothing */
}


/* callback type for Time Zone state related transactions */
static void tai_utc_delta_cb(const time_client_t *p_self,
                             const access_message_rx_meta_t *p_meta,
                             const tai_utc_delta_status_param_t *p_in)
{
    /* do nothing */
}


static void time_client_periodic_publish_cb(access_model_handle_t handle, void *p_self)
{
    time_client_t *p_client = (time_client_t *)p_self;
    (void)time_client_get(p_client);
}


static void time_client_ack_transaction_status_cb(access_model_handle_t model_handle,
                                                  void * p_args,
                                                  access_reliable_status_t status)
{
    /* do nothing */
}


/* Time client callbacks */
const time_client_callbacks_t time_client_cbs = {
    .time_status_cb = time_status_cb,
    .time_role_status_cb = time_role_status_cb,
    .time_zone_status_cb = time_zone_status_cb,
    .tai_utc_delta_cb = tai_utc_delta_cb,
    .ack_transaction_status_cb = time_client_ack_transaction_status_cb,
    .periodic_publish_cb = time_client_periodic_publish_cb
};



/* interface functions */

uint32_t app_time_client_init(app_time_client_t *app, uint8_t element_index)
{
    app->client.settings.p_callbacks = &time_client_cbs;
    APP_ERROR_CHECK(time_client_init(&app->client, element_index));

    return NRF_SUCCESS;
}


uint32_t app_time_client_get(app_time_client_t *app)
{
    return time_client_get(&app->client);
}
