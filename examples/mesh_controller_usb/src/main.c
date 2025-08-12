/* Copyright (c) 2025, Alexander Ozumenko
 * All rights reserved.
 */

#include <stdint.h>

#include "nrf_delay.h"
#include "boards.h"
#include "log.h"
#include "serial.h"
#include "ad_data_proxy.h"
#include "mesh_app_utils.h"
#include "mesh_stack.h"
#include "ble_softdevice_support.h"
#include "nrf_mesh_config_examples.h"
#include "mesh_opt_prov.h"
#include "app_timer.h"
#include "app_usbd.h"
#include "ctrl_led.h"
#include "example_common.h"
#include "nrf_mesh_configure.h"

/* logging */
#include "nrf5_sdk_log.h"



/*definitions */


/* static variables */

static bool m_device_provisioned;


static void mesh_init(void)
{
    mesh_stack_init_params_t init_params =
    {
        .core.irq_priority = NRF_MESH_IRQ_PRIORITY_LOWEST,
        .core.lfclksrc     = DEV_BOARD_LF_CLK_CFG
    };

    uint32_t status = mesh_stack_init(&init_params, &m_device_provisioned);
    switch (status)
    {
        case NRF_ERROR_INVALID_DATA:
            NRF_LOG_INFO("Data in the persistent memory was corrupted. Device starts as unprovisioned.");
            NRF_LOG_INFO("Reboot device before starting of the provisioning process.");
            break;
        case NRF_SUCCESS:
            break;
        default:
            ERROR_CHECK(status);
    }

    NRF_LOG_INFO("Initializing BLE Advertizing Data proxy interface...");
    ERROR_CHECK(ad_data_proxy_init());
}


static void initialize(void)
{
    ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    NRF_LOG_INFO("----- Bluetooth Mesh Controller Application -----");

    ERROR_CHECK(app_timer_init());
    ctrl_led_init();
    ctrl_led_set(LED_1, true);

    NRF_LOG_INFO("Initializing serial interface...");
    ERROR_CHECK(serial_init());

    ble_stack_init();
    mesh_init();

    NRF_LOG_INFO("Initialization complete!");
}


static void start(void)
{
    mesh_app_uuid_print(nrf_mesh_configure_device_uuid_get());

    ERROR_CHECK(mesh_stack_start());

    NRF_LOG_INFO("Statrting serial interface...");
    ERROR_CHECK(serial_start());

    NRF_LOG_INFO("Bluetooth Mesh Controller Application started!");
}


/* entry point */
int main(void)
{
    initialize();
    start();

    for (;;) {
        while (app_usbd_event_queue_process()) {}
        UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
        (void)sd_app_evt_wait();
    }
}
