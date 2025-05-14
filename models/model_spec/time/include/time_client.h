/* (C) 2024, Alexander Ozumenko
 * St(u)dio of Computer Games (https://stdio.ru/)
 */

#ifndef TIME_CLIENT_H__
#define TIME_CLIENT_H__


#include <stdint.h>
#include "access.h"
#include "access_reliable.h"
#include "time_common.h"
#include "model_common.h"



/* client model ID */
#define TIME_CLIENT_MODEL_ID 0x1202


/* forward declaration */
typedef struct time_client_s time_client_t;


/* callback type for Time state related transactions */
typedef void (*time_status_cb_t)(const time_client_t *p_self,
                                 const access_message_rx_meta_t *p_meta,
                                 const time_status_param_t *p_in);

/* callback type for Time Role state related transactions */
typedef void (*time_role_status_cb_t)(const time_client_t *p_self,
                                      const access_message_rx_meta_t *p_meta,
                                      const time_role_status_param_t *p_in);

/* callback type for Time Zone state related transactions */
typedef void (*time_zone_status_cb_t)(const time_client_t *p_self,
                                      const access_message_rx_meta_t *p_meta,
                                      const time_zone_status_param_t *p_in);

/* callback type for TAI-UTC Delta state related transactions */
typedef void (*tai_utc_delta_cb_t)(const time_client_t *p_self,
                                   const access_message_rx_meta_t *p_meta,
                                   const tai_utc_delta_status_param_t *p_in);




typedef struct
{
    /* client model response message callbacks */
    time_status_cb_t time_status_cb;
    time_role_status_cb_t time_role_status_cb;
    time_zone_status_cb_t time_zone_status_cb;
    tai_utc_delta_cb_t tai_utc_delta_cb;

    /* callback to call after the acknowledged transaction has ended */
    access_reliable_cb_t ack_transaction_status_cb;

    /* callback called at the end of the each period for the publishing */
    access_publish_timeout_cb_t periodic_publish_cb;
} time_client_callbacks_t;



/* user provided settings and callbacks for the model instance */
typedef struct
{
    /* reliable message timeout in microseconds */
    uint32_t timeout;

    /* if server should force outgoing messages as segmented messages */
    bool force_segmented;

    /* transMIC size used by the outgoing server messages */
    nrf_mesh_transmic_size_t transmic_size;

    /* callback list */
    const time_client_callbacks_t * p_callbacks;
} time_client_settings_t;




struct time_client_s
{
    /* model handle assigned to this instance */
    access_model_handle_t model_handle;

    /* acknowledged message context variable */
    access_reliable_t access_message;

    /** model settings and callbacks for this instance */
    time_client_settings_t settings;
};




/* initializes Time client */
uint32_t time_client_init(time_client_t *p_client, uint8_t element_index);

/* ... */
uint32_t time_client_get(time_client_t *p_client);



#endif /* TIME_CLIENT_H__ */
