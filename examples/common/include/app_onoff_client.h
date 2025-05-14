#ifndef __APP_ONOFF_CLIENT_H_
#define __APP_ONOFF_CLIENT_H_



#include <stdint.h>
#include "generic_onoff_client.h"
#include "app_timer.h"



/* forward declaration */
typedef struct __app_onoff_client_t app_onoff_client_t;


/* on/off client FSM states */
enum onoff_client_task_state_e {
    ONOFF_CLIENT_IDLE = 0,

    ONOFF_CLIENT_SINGLE_GET_START,
    ONOFF_CLIENT_SINGLE_GET_SEND,

    ONOFF_CLIENT_TOGGLE_START,
    ONOFF_CLIENT_TOGGLE_GET_SEND,
    ONOFF_CLIENT_TOGGLE_SET,
    ONOFF_CLIENT_TOGGLE_SET_SEND,
};

/* application Light HSL (hue/saturation) state set callback prototype */
typedef void (*app_onoff_client_status_cb_t)(const app_onoff_client_t *p_app,
                                             bool onoff_status);

struct __app_onoff_client_t {
    generic_onoff_client_t client;
    enum onoff_client_task_state_e state;
    bool onoff_state;
    app_onoff_client_status_cb_t status_cb;
    app_timer_t timer_id_data;
    app_timer_id_t timer_id;
    uint8_t tid;
};



uint32_t app_onoff_client_init(app_onoff_client_t *app,
                               uint8_t element_index,
                               app_onoff_client_status_cb_t status_cb);
void app_onoff_client_get(app_onoff_client_t *app);
void app_onoff_client_toggle(app_onoff_client_t *app);



#endif /* __APP_ONOFF_CLIENT_H_ */
