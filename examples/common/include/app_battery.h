/* (C) 2025 Alexander Ozumenko
 * St(u)dio of Computer Games (https://stdio.ru/)
 */

#ifndef APP_BATTERY_H__
#define APP_BATTERY_H__

#include <stdint.h>

#include "generic_battery_server.h"

/**
 * Macro to create application level app_battery_server_t context.
 *
 * Individual timer instances are created for each model instance.
 *
 * @param[in] _name                 Name of the app_battery_server_t instance
 * @param[in] _force_segmented      If the Generic Battery server shall use force segmentation of messages
 * @param[in] _mic_size             MIC size to be used by Generic Battery server
 * @param[in] _set_cb               Callback for setting the application state to given value.
 * @param[in] _get_cb               Callback for reading the state from the application.
 * @param[in] _transition_cb        Callback for setting the application transition time and state value to given values.
 */
#define APP_BATTERY_SERVER_DEF(_name, _force_segmented, _mic_size, _get_cb)  \
    static app_battery_server_t _name =  \
    {  \
        .server.settings.force_segmented = _force_segmented,  \
        .server.settings.transmic_size = _mic_size,  \
        .battery_get_cb = _get_cb  \
    };

/* Forward declaration */
typedef struct __app_battery_server_t app_battery_server_t;

/** Application state read callback prototype.
 * This callback is called by the app_model_behaviour.c whenever application battery state is required
 * to be read.
 *
 * @param[in]  p_app             Pointer to [app_battery_server_t](@ref __app_battery_server_t) context.
 * @param[out] p_present_battery User application fills this value with the value retrived from
 *                               the hardware interface. See @ref model_callback_pointer_note.
 */
typedef void (*app_battery_get_cb_t)(const app_battery_server_t *p_app, generic_battery_status_params_t *p_status);

/** Application level structure holding the Battery server model context and Battery state representation */
struct __app_battery_server_t
{
    /** Battery server model interface context structure */
    generic_battery_server_t server;
    /** Callback to be called for requesting current value from the user application */
    app_battery_get_cb_t battery_get_cb;
};

/** Initiates value fetch from the user application by calling a get callback and publishes the
 * Generic Battery Status message.
 *
 * This API must always be called by an application when user initiated action (e.g. button press)
 * results in the local Battery state change. This API should never be called from transition
 * callback. @tagMeshSp mandates that, every local state change must be
 * published if model publication state is configured. If model publication is not configured this
 * API call will not generate any error condition.
 *
 * @param[in] p_app             Pointer to [app_battery_server_t](@ref __app_battery_server_t) context.
 */
void app_battery_status_publish(app_battery_server_t *p_app);

/** Initializes the behavioral module for the generic Battery model
 *
 * @param[in] p_app                 Pointer to [app_battery_server_t](@ref __app_battery_server_t)
 *                                  context.
 * @param[in] element_index         Element index on which this server will be instantiated.
 *
 * @retval NRF_SUCCESS              If initialization is successful.
 * @retval NRF_ERROR_NO_MEM         @ref ACCESS_MODEL_COUNT number of models already allocated, or
 *                                  no more subscription lists available in memory pool.
 * @retval NRF_ERROR_NULL           NULL pointer is supplied to the function or to the required
 *                                  member variable pointers.
 * @retval NRF_ERROR_NOT_FOUND      Invalid access element index, or access handle invalid.
 * @retval NRF_ERROR_FORBIDDEN      Multiple model instances per element are not allowed or changes
 *                                  to device composition are not allowed. Adding a new model after
 *                                  device is provisioned is not allowed.
 * @retval  NRF_ERROR_INVALID_PARAM Model not bound to appkey, publish address not set or wrong
 *                                  opcode format. The application timer module has not been
 *                                  initialized or timeout handler is not provided.
 * @retval NRF_ERROR_INVALID_STATE  If the application timer is running.
*/
uint32_t app_battery_init(app_battery_server_t *p_app, uint8_t element_index);

#endif /* APP_BATTERY_H__ */
