/*
 *  Output blink provisioning implementation.
 *
 * (C)2021-2022 St(u)dio of Computer Games
 */

#ifndef __MESH_PROVISIONEE_OUTPUT_BLINK_H_
#define __MESH_PROVISIONEE_OUTPUT_BLINK_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup MESH_PROVISIONEE Mesh examples provisionee support module
 * @ingroup MESH_API_GROUP_APP_SUPPORT
 * Mesh examples support module for starting the provisioning process for a device in the
 * provisionee role using PB-ADV and static OOB authentication.
 * @{
 */

/**
 * Set BLE stack option callback type.
 *
 * This callback is called to indicate that BLE stack was restarted and any
 * BLE option can be set at this point through @link_sd_ble_opt_set function.
 */
typedef void (*mesh_provisionee_prov_sd_ble_opt_set_cb_t)(void);

/**
 * Provisioning complete callback type.
 *
 * This callback is called to indicate that the device has been successfully provisioned.
 */
typedef void (*mesh_provisionee_prov_complete_cb_t)(void);

/**
 * Start device identification callback type.
 *
 * This callback is called to indicate that the device can start identifying itself
 * using any means it can to attract attention.
 *
 * @note This callback is called every time when the device is asked to start identifying
 * itself.
 *
 * @param[in]   attention_duration_s    Time in seconds during which the device identifies
 *                                      itself using any means it can.
 */
typedef void (*mesh_provisionee_prov_device_identification_start)(uint8_t attention_duration_s);

/**
 * Stop device identification callback type.
 *
 * This callback is called to indicate that the device should stop identifying itself.
 *
 * @note This callback is called only once per provisioning process.
 */
typedef void (*mesh_provisionee_prov_device_identification_stop)(void);

/**
 * Provisioning abort callback type.
 *
 * This callback is called if the provisioning process is aborted.
 *
 * @note This callback can be used to stop the device from identifying itself if the provisioning process is aborted
 *       before @ref mesh_provisionee_prov_device_identification_stop is called.
 */
typedef void (*mesh_provisionee_prov_abort)(void);

/**
 * Show output PIN callback type.
 *
 * This callback is called to show PIN on device.
 *
 * @note This callback is called only once per provisioning process.
 */
typedef void (*mesh_provisionee_prov_output_request)(uint8_t action, uint8_t size, const uint8_t *p_data);



/**
 * Mesh stack configuration parameters.
 *
 * Some fields are optional; the description of the specific fields notes if the
 * value of a field can be omitted. In this case, the value of the field should be
 * set to 0 (@c NULL for pointer values).
 */
typedef struct
{
    /**
     * Pointer to a function used to provide ability to set any BLE option
     * after the BLE stack was restarted during the provisioning. Use it if you need
     * to set any BLE option through @link_sd_ble_opt_set function.
     * Can be set to @c NULL if not used.
     */
    mesh_provisionee_prov_sd_ble_opt_set_cb_t prov_sd_ble_opt_set_cb;

    /**
     * Pointer to a function used to signal the completion of the device provisioning
     * procedure. Can be set to @c NULL if not used.
     *
     * @note Getting this callback means that a device is at minimum _provisioned_, however,
     *       it does not imply anthing about model configuration, added keys, etc. That may
     *       be altered by a Configuration Client at any point in time.
     *       See @ref CONFIG_SERVER_EVENTS.
     */
    mesh_provisionee_prov_complete_cb_t prov_complete_cb;

    /**
     * Pointer to a function used to signal the device to start identifying itself.
     * Can be set to @c NULL if not used.
     */
    mesh_provisionee_prov_device_identification_start prov_device_identification_start_cb;

    /**
     * Pointer to a function used to signal the device to stop identifying itself.
     * Can be set to @c NULL if not used.
     */
    mesh_provisionee_prov_device_identification_stop prov_device_identification_stop_cb;

    /**
     * Pointer to a function used to signal the abort of the device provisioning
     * procedure. Can be set to @c NULL if not used.
     */
    mesh_provisionee_prov_abort prov_abort_cb;

    /**
     TODO: add comment
     */
    mesh_provisionee_prov_output_request prov_output_request_cb;

    /**
     * NULL-terminated device URI string.
     * This is an optional field that can be used to add additional data to the unprovisioned
     * node broadcast beacon.
     */
    const char * p_device_uri;
} mesh_provisionee_start_params_t;

/**
 * Start the provisioning process for a device in the provisionee role using static OOB
 * authentication.
 *
 * @param[in] p_start_params  Pointer to structure containing parameters related to the provisioning
 *                            procedure.
 *
 * @retval NRF_ERROR_INVALID_STATE  The provisioning module is not in idle state.
 * @retval NRF_ERROR_INVALID_PARAM  The p_static_data parameter is NULL.
 * @retval NRF_SUCCESS              The provisioning was started successfully.
 */
uint32_t mesh_provisionee_prov_start(const mesh_provisionee_start_params_t * p_start_params);

/**
 * Stops listening for incoming provisioning links.
 *
 * @retval NRF_ERROR_INVALID_STATE The provisionee is not currently listening.
 * @retval NRF_SUCCESS             Successfully stopped listening.
 */
uint32_t mesh_provisionee_prov_listen_stop(void);

/**
 * @}
 */

#endif /* __MESH_PROVISIONEE_H_OUTPUT_BLINK_ */
