#ifndef APP_PONOFF_H__
#define APP_PONOFF_H__

#include <stdint.h>

#include "generic_ponoff_setup_server.h"
#include "generic_onoff_server.h"
#include "app_transition.h"
#include "app_timer.h"
#if (SCENE_SETUP_SERVER_INSTANCES_MAX > 0) || (DOXYGEN)
#include "app_scene.h"
#endif


/**
 * @defgroup APP_ONOFF Generic OnOff server behaviour
 * @ingroup MESH_API_GROUP_APP_SUPPORT
 * Application level OnOff server behavioral structures, functions, and callbacks.
 *
 * This module implements the behavioral requirements of the Generic OnOff server model.
 *
 * The application should use the set/transition callback provided by this module to set the
 * hardware state. The hardware state could be changed by reflecting the value provided by the
 * set/transiton_time callback on the GPIO or by sending this value to the connected lighting
 * peripheral using some other interface (e.g. serial interface). Similarly, the application should
 * use the get callback provided by this module to read the hardware state.
 *
 * This module triggers the set/transition callback only when it determins that it is time to
 * inform the user application. It is possible that the client can send multiple overlapping
 * set/transition commands. In such case any transition in progress will be abandoned and fresh
 * transition will be started if required.
 *
 * Using transition_cb:
 * If the underlaying hardware does not support setting of the instantaneous value provided via
 * `set_cb`, the `transition_cb` can be used to implement the transition effect according to
 * provided transition parameters. This callback will be called when transition start with the
 * required transition time and target value. When the transition is complete this callback will be
 * called again with transition time set to 0 and the desired target value.
 * <br>
 * @warning To comply with the @tagMeshMdlSp test cases, the application must adhere to
 * the requirements defined in the following sections:
 * - @tagMeshMdlSp section 3.1.1 (Generic OnOff) and section 3.3.1.2 (Generic OnOff state behaviour).
 * - @tagMeshSp section 3.7.6.1 (Publish).
 *
 * These requirements are documented at appropriate places in the module source code.
 *
 * @{
 */

/**
 * Macro to create application level app_onoff_server_t context.
 *
 * Individual timer instances are created for each model instance.
 *
 * @param[in] _name                 Name of the app_onoff_server_t instance
 * @param[in] _force_segmented      If the Generic OnOff server shall use force segmentation of messages
 * @param[in] _mic_size             MIC size to be used by Generic OnOff server
 * @param[in] _set_cb               Callback for setting the application state to given value.
 * @param[in] _get_cb               Callback for reading the state from the application.
 * @param[in] _transition_cb        Callback for setting the application transition time and state value to given values.
 */
#define APP_PONOFF_SETUP_SERVER_DEF(_name, _force_segmented, _mic_size, _set_cb, _get_cb, _transition_cb)  \
    APP_TIMER_DEF(_name ## _timer); \
    static app_ponoff_setup_server_t _name = \
    {  \
        .server.settings.force_segmented = _force_segmented, \
        .server.settings.transmic_size = _mic_size, \
        .server.generic_ponoff_srv.generic_onoff_srv.settings.force_segmented = _force_segmented, \
        .server.generic_ponoff_srv.generic_onoff_srv.settings.transmic_size = _mic_size, \
        .server.generic_dtt_srv.settings.force_segmented = _force_segmented, \
        .server.generic_dtt_srv.settings.transmic_size = _mic_size, \
        .state.transition.timer.p_timer_id = &_name ## _timer, \
        .onoff_set_cb = _set_cb, \
        .onoff_get_cb = _get_cb, \
        .onoff_transition_cb = _transition_cb \
    };


typedef struct {
    bool present_onoff;
    bool initial_present_onoff;
    bool target_onoff;
    app_transition_t transition;
    uint8_t on_powerup;
} app_onoff_state_t;

/* Forward declaration */
typedef struct __app_ponoff_setup_server_t app_ponoff_setup_server_t;


typedef void (*app_onoff_set_cb_t)(const app_ponoff_setup_server_t * p_app, bool onoff);
typedef void (*app_onoff_get_cb_t)(const app_ponoff_setup_server_t * p_app, bool *p_present_onoff);
typedef void (*app_onoff_transition_cb_t)(const app_ponoff_setup_server_t * p_app,
                                          uint32_t transition_time_ms,
                                          bool target_onoff);

/** Application level structure holding the OnOff server model context and OnOff state representation */
struct __app_ponoff_setup_server_t
{
    // Power OnOff Setup server model interface context structure
    generic_ponoff_setup_server_t server;

    // Power OnOff Setup server state handle
    uint8_t state_handle;

    // callaback to be called for informing the user application to update the value
    app_onoff_set_cb_t  onoff_set_cb;
    // callback to be called for requesting current value from the user application
    app_onoff_get_cb_t onoff_get_cb;
    // callaback to be called for informing the user application to update the value
    app_onoff_transition_cb_t onoff_transition_cb;

    app_onoff_state_t state;

    // cached Default Transition Time value
    uint32_t transition_time_ms;

#if (SCENE_SETUP_SERVER_INSTANCES_MAX > 0)
    app_scene_model_interface_t scene_if;
    app_scene_setup_server_t  *p_app_scene;
#endif
};

void app_ponoff_status_publish_onoff(app_ponoff_setup_server_t * p_app);
uint32_t app_ponoff_init(app_ponoff_setup_server_t *p_app, uint8_t element_index);
uint32_t app_ponoff_value_restore(app_ponoff_setup_server_t *p_app);

#if (SCENE_SETUP_SERVER_INSTANCES_MAX > 0)
uint32_t app_ponoff_scene_context_set(app_ponoff_setup_server_t *p_app,
                                      app_scene_setup_server_t *p_app_scene);
#endif



#endif /* APP_ONOFF_H__ */
