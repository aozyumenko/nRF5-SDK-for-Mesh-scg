/* (C) 2024 Alexander Ozumenko
 * St(u)dio of Computer Games (https://stdio.ru/)
 *
 * Time client header file.
 */

#ifndef __APP_TIME_CLIENT_H_
#define __APP_TIME_CLIENT_H_


#include <stdint.h>
#include "time_client.h"



/* forward declaration */
typedef struct app_time_client_s app_time_client_t;



typedef void (*app_clock_client_set_cb_t)(const app_time_client_t *p_app,
                                          uint64_t device_seconds);


typedef struct app_time_client_s {
    time_client_t client;

    app_clock_client_set_cb_t set_cb;
} app_time_client_t;



/* macro to create application level app_time_client_t context  */
#define APP_TIME_CLIENT_DEF(_name, _force_segmented, _mic_size, _set_cb)        \
    static app_time_client_t _name =                                            \
    {                                                                           \
        .client.settings.force_segmented = _force_segmented,                    \
        .client.settings.transmic_size = _mic_size,                             \
        .set_cb = _set_cb,                                                      \
    };



uint32_t app_time_client_init(app_time_client_t *app, uint8_t element_index);
uint32_t app_time_client_get(app_time_client_t *app);



#endif /* __APP_TIME_CLIENT_H_ */
