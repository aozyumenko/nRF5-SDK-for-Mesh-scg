/*
 * Control LED driver.
 */

#include "ctrl_led.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "boards.h"
#include "nrf.h"
#include "nrf_error.h"

#include "nrf_mesh_defines.h"
#include "nrf_mesh_assert.h"
#include "app_timer.h"
#include "app_error.h"
#include "nrfx_gpiote.h"



#ifndef LEDS_ACTIVE_STATE
#define LEDS_ACTIVE_STATE 1
#endif /* LEDS_ACTIVE_STATE */



/* declarations */

enum led_mode_e {
    LED_MODE_ONOFF = 0,
    LED_MODE_BLINK,
    LED_MODE_COUNT
};


typedef struct led_state_s {
    /* LED pin */
    nrfx_gpiote_pin_t pin;

    /* LED mode */
    enum led_mode_e mode;

    /* blink and counter configuration */
    uint32_t delay_ticks;
    uint32_t pause_ticks;
    int count;

    /* counter status */
    int cur_count;

    /* APP timer instance */
    app_timer_id_t timer_id;
    app_timer_t timer_id_data;
} led_state_t;



/* static variables */

static const uint8_t m_leds_list[LEDS_NUMBER] = LEDS_LIST;
static led_state_t m_led_state[LEDS_NUMBER];

/* Since SDK does not allow to create APP_TIMERs as members of a structure or
   as arrays, we create one timer instance to use it as a template for
   initializing built-in timers. */
APP_TIMER_DEF(m_led_timer);



/* internal functions */

static led_state_t *ctrl_led_get_state(nrfx_gpiote_pin_t led_pin)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(m_led_state); i++) {
        if (m_led_state[i].pin == led_pin) {
            return &m_led_state[i];
        }
    }

    return NULL;
}


static bool ctrl_led_pin_get(nrfx_gpiote_pin_t led_pin)
{
#if LEDS_ACTIVE_STATE == 1
    return (nrf_gpio_pin_out_read(led_pin) != 0);
#else /* LEDS_ACTIVE_STATE == 1 */
    return (nrf_gpio_pin_out_read(led_pin) == 0);
#endif /* LEDS_ACTIVE_STATE == 1 */
}


static void ctrl_led_pin_set(nrfx_gpiote_pin_t led_pin, bool value)
{
#if LEDS_ACTIVE_STATE == 1
    if (value) {
        nrfx_gpiote_out_set(led_pin);
    } else {
        nrfx_gpiote_out_clear(led_pin);
    }
#else /* LEDS_ACTIVE_STATE == 1 */
    if (value) {
        nrfx_gpiote_out_clear(led_pin);
    } else {
        nrfx_gpiote_out_set(led_pin);
    }
#endif /* LEDS_ACTIVE_STATE == 1 */
}


static void led_timer_cb(void * p_context)
{
    led_state_t *led_state = (led_state_t *)p_context;

    switch (led_state->mode) {
    case LED_MODE_BLINK:
        ctrl_led_pin_set(led_state->pin, !ctrl_led_pin_get(led_state->pin));
        APP_ERROR_CHECK(app_timer_start(led_state->timer_id, led_state->delay_ticks, led_state));
        break;

    case LED_MODE_COUNT:
        if (ctrl_led_pin_get(led_state->pin)) {
            ctrl_led_pin_set(led_state->pin, false);
            led_state->cur_count++;
        } else {
            ctrl_led_pin_set(led_state->pin, true);
        }

        if (led_state->cur_count < led_state->count) {
            APP_ERROR_CHECK(app_timer_start(led_state->timer_id, led_state->delay_ticks, led_state));
        } else {
            if (led_state->pause_ticks > 0) {
                APP_ERROR_CHECK(app_timer_start(led_state->timer_id, led_state->pause_ticks, led_state));
                led_state->cur_count = 0;
            } else {
                led_state->mode = LED_MODE_ONOFF;
            }
        }
        break;

    default:
        break;
    }
}



/* public API */

bool ctrl_led_get(nrfx_gpiote_pin_t led_pin)
{
    led_state_t *led_state = ctrl_led_get_state(led_pin);
    APP_ERROR_CHECK_BOOL(led_state != NULL);

    return ctrl_led_pin_get(led_state->pin);
}


void ctrl_led_set(nrfx_gpiote_pin_t led_pin, bool value)
{
    led_state_t *led_state = ctrl_led_get_state(led_pin);
    APP_ERROR_CHECK_BOOL(led_state != NULL);

    if (led_state->mode != LED_MODE_ONOFF) {
        app_timer_stop(led_state->timer_id);
    }
    led_state->mode = LED_MODE_ONOFF;
    ctrl_led_pin_set(led_state->pin, value);
}


void ctrl_led_blink(nrfx_gpiote_pin_t led_pin, uint32_t delay_ms)
{
    led_state_t *led_state = ctrl_led_get_state(led_pin);
    APP_ERROR_CHECK_BOOL(led_state != NULL);

    if (led_state->mode != LED_MODE_ONOFF) {
        app_timer_stop(led_state->timer_id);
    }

    led_state->mode = LED_MODE_BLINK;
    led_state->delay_ticks = APP_TIMER_TICKS(delay_ms);
    APP_ERROR_CHECK(app_timer_start(led_state->timer_id, led_state->delay_ticks, led_state));
}


void ctrl_led_blink_count(nrfx_gpiote_pin_t led_pin, int count, uint32_t delay_ms, uint32_t pause_ms)
{
    led_state_t *led_state = ctrl_led_get_state(led_pin);
    APP_ERROR_CHECK_BOOL(led_state != NULL);

    if (led_state->mode != LED_MODE_ONOFF) {
        app_timer_stop(led_state->timer_id);
    }

    led_state->mode = LED_MODE_COUNT;
    led_state->count = count;
    led_state->delay_ticks = APP_TIMER_TICKS(delay_ms);
    led_state->pause_ticks = APP_TIMER_TICKS(pause_ms);

    led_state->cur_count = 0;
    APP_ERROR_CHECK(app_timer_start(led_state->timer_id, led_state->delay_ticks, led_state));
}


/* initialize LEDs driver */
void ctrl_led_init(void)
{
    /* mark timers main structure as unused */
    const app_timer_id_t m_timer0 __attribute__((unused)) = m_led_timer;

    if (!nrfx_gpiote_is_init()) {
        APP_ERROR_CHECK(nrfx_gpiote_init());
    }

    nrfx_gpiote_out_config_t out_config = {
#if LEDS_ACTIVE_STATE == 1
        .init_state = NRF_GPIOTE_INITIAL_VALUE_LOW,
#else /* LEDS_ACTIVE_STATE == 1 */
        .init_state = NRF_GPIOTE_INITIAL_VALUE_HIGH,
#endif /* LEDS_ACTIVE_STATE == 1 */
        .task_pin = false
    };

    for (uint32_t i = 0; i < ARRAY_SIZE(m_led_state); i++) {
        m_led_state[i].pin = m_leds_list[i];
        APP_ERROR_CHECK(nrfx_gpiote_out_init(m_led_state[i].pin, &out_config));
        m_led_state[i].mode = LED_MODE_ONOFF;
        memcpy(&m_led_state[i].timer_id_data, &m_led_timer_data, sizeof(m_led_timer_data));
        m_led_state[i].timer_id = &m_led_state[i].timer_id_data;
        APP_ERROR_CHECK(app_timer_create(&m_led_state[i].timer_id, APP_TIMER_MODE_SINGLE_SHOT, led_timer_cb));
    }
}
