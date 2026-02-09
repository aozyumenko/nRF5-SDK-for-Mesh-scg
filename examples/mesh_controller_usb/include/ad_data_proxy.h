/* Copyright (c) 2025, Alexander Ozumenko
 * All rights reserved.
 *
 */
#ifndef AD_DATA_PROXY_H__
#define AD_DATA_PROXY_H__

#include <stdint.h>
//#include <stdbool.h>
#include "nrf_mesh.h"
//#include "net_packet.h"
//#include "nrf_mesh_config_core.h"



/**
 * Initializes the BLE Advertizing Data proxy.
 */
uint32_t ad_data_proxy_init(void);

/**
 * Enables the mesh proxy service.
 *
 * @note This only enables the service, calling proxy_start() is required
 *       to start advertising with the network ID.
 * @note If the node ID is already active, calling this API will still enable the state, but
 *       will not start advertising the network ID until the node ID is finished.
 *
 * @retval NRF_SUCCESS The advertisement was started successfully.
 * @retval NRF_ERROR_INVALID_STATE In an invalid state for enabling the proxy state.
 */
//uint32_t proxy_enable(void);

/**
 * Starts the mesh proxy service.
 *
 * Advertises with network ID to allow proxy clients to establish a connection to a mesh node.
 *
 * @note If the proxy state is enabled and the node ID is active, calling this function will have
 *       no effect. The network ID will not be advertised until the node ID is finished.
 *
 * @retval NRF_SUCCESS Successfully started the proxy.
 * @retval NRF_ERROR_INVALID_STATE The proxy is currently not enabled or not initialized.
 */
uint32_t proxy_start(void);


/**
 * Stops the mesh proxy service.
 *
 * The service disconnects from all connected Proxy Clients.
 * The disconnection is initiated once all packets are transmitted.
 * When the mesh proxy service is stopped, the @ref NRF_MESH_EVT_PROXY_STOP 
 * event is generated.
 *
 * @note This API is intended to be used to safely stop the proxy service before a reset. It will
 * not modify the stored state of the proxy, s.t. the original state will be loaded after the reset.
 *
 * @warning Calling this function will disable the proxy state to stop it from restarting the
 * Network ID beacons after a potential disconnection. To re-enable the proxy, the proxy_enable()
 * function should be used before proxy_start().
 *
 * @retval NRF_SUCCESS Successfully stopped and disabled the proxy state.
 * @retval NRF_ERROR_INVALID_STATE The proxy state is currently not enabled or not initialized.
 */
uint32_t proxy_stop(void);

/**
 * Sending advertising packet.
 *
 * @param[in] token                     Packet token.
                                        Used in the response to identify the command.
 * @param[in] p_data                    Advertising packet data.
 * @param[in] data_len                  Advertising packet length.
 */
void ad_data_proxy_tx(nrf_mesh_tx_token_t token, const uint8_t *p_data, int length);


#endif /* AD_DATA_PROXY_H__ */
