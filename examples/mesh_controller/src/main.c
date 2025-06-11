/* Copyright (c) 2025, Alexander Ozumenko
 * All rights reserved.
 *
 */

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
#include "ctrl_led.h"
#include "example_common.h"
#include "nrf_mesh_configure.h"

/*definitions */


/*forward declaration of static functions */


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
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Data in the persistent memory was corrupted. Device starts as unprovisioned.\n");
            __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Reboot device before starting of the provisioning process.\n");
            break;
        case NRF_SUCCESS:
            break;
        default:
            ERROR_CHECK(status);
    }

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initializing serial interface...\n");
    ERROR_CHECK(serial_init());

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initializing BLE Advertizing Data proxy interface...\n");
    ERROR_CHECK(ad_data_proxy_init());
}


static void initialize(void)
{
#if defined(NRF51) && defined(NRF_MESH_STACK_DEPTH)
    stack_depth_paint_stack();
#endif
    __LOG_INIT(LOG_MSK_DEFAULT | LOG_SRC_ACCESS | LOG_SRC_SERIAL | LOG_SRC_APP, LOG_LEVEL_INFO, log_callback_rtt);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- Bluetooth Mesh Controller Application -----\n");

    ERROR_CHECK(app_timer_init());

    ctrl_led_init();

    ble_stack_init();

    mesh_init();

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initialization complete!\n");
}


static void start(void)
{
    ERROR_CHECK(serial_start());

    mesh_app_uuid_print(nrf_mesh_configure_device_uuid_get());

    ERROR_CHECK(mesh_stack_start());

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Bluetooth Mesh Controller Application started!\n");
}


/* entry point */
int main(void)
{
    initialize();
    start();

    ctrl_led_set(LED_1, true);

    for (;;) {
        (void)sd_app_evt_wait();
    }
}
