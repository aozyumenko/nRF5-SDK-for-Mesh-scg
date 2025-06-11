#ifndef __CTRL_LED_H_
#define __CTRL_LED_H_

#include <stdint.h>
#include <stdbool.h>
#include "hal.h"
#include "boards.h"
#include "utils.h"

#include "nrfx_gpiote.h"



bool ctrl_led_get(nrfx_gpiote_pin_t led_pin);
void ctrl_led_set(nrfx_gpiote_pin_t led_pin, bool value);
void ctrl_led_blink(nrfx_gpiote_pin_t led_pin, uint32_t delay_ms);
void ctrl_led_blink_count(nrfx_gpiote_pin_t led_pin, int count, uint32_t delay_ms, uint32_t pause_ms);

void ctrl_led_init(void);



#endif /* __CTRL_LED_H_ */
