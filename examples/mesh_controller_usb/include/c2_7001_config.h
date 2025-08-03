#ifndef E104_BT5032U_CONFIG_H
#define E104_BT5032U_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "nrf.h"

// LEDs definitions
#define LEDS_NUMBER             1

#define LED_1                   NRF_GPIO_PIN_MAP(0,0)
#define LED_2                   NRF_GPIO_PIN_MAP(0,1)
#define LED_3                   NRF_GPIO_PIN_MAP(0,2)
#define LED_4                   NRF_GPIO_PIN_MAP(0,3)
#define LED_5                   NRF_GPIO_PIN_MAP(0,4)
#define LED_6                   NRF_GPIO_PIN_MAP(0,5)
#define LED_7                   NRF_GPIO_PIN_MAP(0,6)
#define LED_8                   NRF_GPIO_PIN_MAP(0,7)
#define LED_START               LED_1
#define LED_STOP                LED_8

#define LEDS_ACTIVE_STATE       1

//#define LEDS_LIST               { LED_1 }
#define LEDS_LIST               { LED_1, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7, LED_8 }

#define LEDS_INV_MASK           LEDS_MASK

#define BSP_LED_0               LED_1
#define BSP_LED_1               LED_2
#define BSP_LED_2               LED_3
#define BSP_LED_3               LED_4
#define BSP_LED_4               LED_5
#define BSP_LED_5               LED_6
#define BSP_LED_6               LED_7
#define BSP_LED_7               LED_8


// buttons definitions
#define BUTTONS_NUMBER          1

#define BUTTON_1                NRF_GPIO_PIN_MAP(0,11)
#define BUTTON_PULL             NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE    0

#define BUTTONS_LIST            { BUTTON_1 }


#ifdef __cplusplus
}
#endif

#endif // E104_BT5032U_CONFIG_H
