#ifndef E104_BT5032U_CONFIG_H
#define E104_BT5032U_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "nrf.h"

// LEDs definitions
#define LEDS_NUMBER             1

#define LED_1                   NRF_GPIO_PIN_MAP(0,13)
#define LED_START               LED_1
#define LED_STOP                LED_1

#define LEDS_ACTIVE_STATE       1

#define LEDS_LIST               { LED_1 }

#define LEDS_INV_MASK           LEDS_MASK

#define BSP_LED_0               LED_1


// buttons definitions
#define BUTTONS_NUMBER          1

#define BUTTON_1                NRF_GPIO_PIN_MAP(0,11)
#define BUTTON_PULL             NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE    0

#define BUTTONS_LIST            { BUTTON_1 }


// UART definitions
#define RX_PIN_NUMBER           NRF_GPIO_PIN_MAP(0,8)
#define TX_PIN_NUMBER           NRF_GPIO_PIN_MAP(0,6)
#define CTS_PIN_NUMBER          NRF_GPIO_PIN_MAP(0,7)
#define RTS_PIN_NUMBER          NRF_GPIO_PIN_MAP(0,5)
#define HWFC                    true
#define SERIAL_UART_BAUDRATE    UART_BAUDRATE_BAUDRATE_Baud1M
//#define SERIAL_UART_BAUDRATE    UART_BAUDRATE_BAUDRATE_Baud460800

#ifdef __cplusplus
}
#endif

#endif // E104_BT5032U_CONFIG_H
