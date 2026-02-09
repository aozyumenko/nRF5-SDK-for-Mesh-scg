#ifndef __CTRL_LED_H_
#define __CTRL_LED_H_

#include <stdint.h>
#include <stdbool.h>
#include "hal.h"
#include "boards.h"
#include "utils.h"

#include "nrfx_gpiote.h"


void ctrl_led_init(void);

void ctrl_led_set(nrfx_gpiote_pin_t led_pin, bool value);
void ctrl_led_blink(nrfx_gpiote_pin_t led_pin, uint32_t delay_ms);



#endif /* __CTRL_LED_H_ */
