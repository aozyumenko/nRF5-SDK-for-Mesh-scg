/* (C) 2025 Alexander Ozumenko
 * St(u)dio of Computer Games (https://stdio.ru/)
 */

#include "app_battery.h"

#include "utils.h"
#include "mesh_app_utils.h"
#include "sdk_config.h"
#include "example_common.h"
#include "generic_battery_server.h"

#include "log.h"



/** This sample implementation shows how the model behavior requirements of Generic Battery server can
 * be implemented.
 */

/* Forward declaration */
static void generic_battery_state_get_cb(const generic_battery_server_t *p_self,
                                         const access_message_rx_meta_t *p_meta,
                                         generic_battery_status_params_t *p_out);

const generic_battery_server_callbacks_t m_battery_srv_cbs =
{
    .battery_cbs.get_cb = generic_battery_state_get_cb
};

/***** Generic Battery model interface callbacks *****/

static void generic_battery_state_get_cb(const generic_battery_server_t *p_self,
                                       const access_message_rx_meta_t *p_meta,
                                       generic_battery_status_params_t *p_out)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "msg: GET \n");

    app_battery_server_t *p_app = PARENT_BY_FIELD_GET(app_battery_server_t, server, p_self);

    /* Requirement: Provide the current value of the Battery state */
    p_app->battery_get_cb(p_app, p_out);
}

/***** Interface functions *****/

void app_battery_status_publish(app_battery_server_t *p_app)
{
    generic_battery_status_params_t status;

    p_app->battery_get_cb(p_app, &status);
    (void) generic_battery_server_status_publish(&p_app->server, &status);
}

uint32_t app_battery_init(app_battery_server_t *p_app, uint8_t element_index)
{
    if (p_app == NULL) {
        return NRF_ERROR_NULL;
    }

    p_app->server.settings.p_callbacks = &m_battery_srv_cbs;
    if (p_app->battery_get_cb == NULL) {
        return NRF_ERROR_NULL;
    }

    return generic_battery_server_init(&p_app->server, element_index);
}
