/* (C) 2024 Alexander Ozumenko
 * St(u)dio of Computer Games (https://stdio.ru/)
 */

#ifndef GENERIC_BATTERY_SERVER_H__
#define GENERIC_BATTERY_SERVER_H__

#include <stdint.h>
#include "access.h"
#include "generic_battery_common.h"
#include "model_common.h"



/** Server model ID */
#define GENERIC_BATTERY_SERVER_MODEL_ID 0x100C



/* Forward declaration */
typedef struct __generic_battery_server_t generic_battery_server_t;

/**
 * Callback type for Generic NatteryGet message.
 *
 * @param[in]     p_self                   Pointer to the model structure.
 * @param[in]     p_meta                   Access metadata for the received message.
 * @param[out]    p_out                    Pointer to store the output parameters from the user application.
 */
typedef void (*generic_battery_state_get_cb_t)(const generic_battery_server_t *p_self,
                                               const access_message_rx_meta_t *p_meta,
                                               generic_battery_status_params_t *p_out);

/**
 * Transaction callbacks for the Battery state.
 */
typedef struct
{
    generic_battery_state_get_cb_t    get_cb;
} generic_battery_server_state_cbs_t;

/**
 * Battery server callback list.
 */
typedef struct
{
    /** Callbacks for the Battery state. */
    generic_battery_server_state_cbs_t battery_cbs;
} generic_battery_server_callbacks_t;

/**
 * User provided settings and callbacks for the model instance.
 */
typedef struct
{
    /** If server should force outgoing messages as segmented messages.
     *  See @ref mesh_model_force_segmented. */
    bool force_segmented;
    /** TransMIC size used by the outgoing server messages.
     * See @ref nrf_mesh_transmic_size_t and @ref mesh_model_large_mic. */
    nrf_mesh_transmic_size_t transmic_size;

    /** Callback list. */
    const generic_battery_server_callbacks_t *p_callbacks;
} generic_battery_server_settings_t;


/** Generic Battery madel instance  */
struct __generic_battery_server_t
{
    /** Model handle assigned to this instance. */
    access_model_handle_t model_handle;

    /** Model settings and callbacks for this instance. */
    generic_battery_server_settings_t settings;
};

/**
 * Initializes Generic Battery server.
 *
 * @note The server handles the model allocation and adding.
 *
 * @param[in]     p_server                 Generic Battery server context pointer.
 * @param[in]     element_index            Element index to add the model to.
 *
 * @retval NRF_SUCCESS                  The model is initialized successfully.
 * @retval NRF_ERROR_NULL               NULL pointer given to function.
 * @retval NRF_ERROR_NO_MEM             @ref ACCESS_MODEL_COUNT number of models already allocated
 *                                      or no more subscription lists available in memory pool
 *                                      (see @ref ACCESS_SUBSCRIPTION_LIST_COUNT).
 * @retval NRF_ERROR_FORBIDDEN          Multiple model instances per element are not allowed
 *                                      or changes to device composition are not allowed.
 *                                      Adding a new model after device is provisioned is not allowed.
 * @retval NRF_ERROR_NOT_FOUND          Invalid access element index.
 */
uint32_t generic_battery_server_init(generic_battery_server_t *p_server, uint8_t element_index);

/**
 * Publishes unsolicited Status message.
 *
 * This function can be used to send unsolicited messages to report the updated state value as a result of local action.
 *
 * @param[in]     p_server                 Status server context pointer.
 * @param[in]     p_params                 Message parameters.
 *
 * @retval NRF_SUCCESS              If the message is published successfully.
 * @retval NRF_ERROR_NULL           NULL pointer given to function.
 * @retval NRF_ERROR_NO_MEM         No memory available to send the message at this point.
 * @retval NRF_ERROR_NOT_FOUND      The model is not initialized.
 * @retval NRF_ERROR_INVALID_PARAM  Invalid message parameters,
 *                                  the model not bound to application key,
 *                                  or publish address not set.
 * @retval NRF_ERROR_FORBIDDEN      Failed to allocate a sequence number from network.
 * @retval NRF_ERROR_INVALID_STATE  There's already a segmented packet that is
 *                                  being to sent to this destination. Wait for
 *                                  the transmission to finish before sending
 *                                  new segmented packets.
 */
uint32_t generic_battery_server_status_publish(generic_battery_server_t *p_server, const generic_battery_status_params_t *p_params);



#endif /* GENERIC_BATTERY_SERVER_H__ */
