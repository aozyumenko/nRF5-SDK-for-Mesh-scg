#ifndef LIGHT_HSL_SETUP_SERVER_H__
#define LIGHT_HSL_SETUP_SERVER_H__


#include <stdint.h>
#include "access.h"
#include "light_hsl_common.h"
#include "model_common.h"

#include "light_lightness_setup_server.h"
//#include "generic_ponoff_setup_server.h"
#include "generic_level_server.h"



/** Server model ID */
#define LIGHT_HSL_SERVER_MODEL_ID (0x1307)

/** Setup server model ID */
#define LIGHT_HSL_SETUP_SERVER_MODEL_ID (0x1308)

/** Hue server model ID */
#define LIGHT_HSL_HUE_SERVER_MODEL_ID (0x130a)

/** Saturation server model ID */
#define LIGHT_HSL_SATURATION_SERVER_MODEL_ID (0x130b)



/* forward declaration for light_ctl_server_t */
typedef struct __light_hsl_server_s light_hsl_server_t;

/* forward declaration for light_ctl_hue_server_t */
typedef struct __light_hsl_hue_server_s light_hsl_hue_server_t;

/* forward declaration for light_ctl_saturation_server_t */
typedef struct __light_hsl_saturation_server_s light_hsl_saturation_server_t;

/* forward declaration for light_ctl_setup_server_t */
typedef struct __light_hsl_setup_server_s light_hsl_setup_server_t;



/*** Light HSL server */


/* user provided settings and callbacks for the Light HSL Server model instance */
typedef struct {
    bool force_segmented;
    nrf_mesh_transmic_size_t transmic_size;
} light_hsl_server_settings_t;


struct __light_hsl_server_s {
    /* model handle assigned to this instance */
    access_model_handle_t model_handle;

    /* tid tracker structure */
    tid_tracker_t tid_tracker;

    /* settings and callbacks for this instance */
    light_hsl_server_settings_t settings;
};


/* publishes unsolicited Status message */
uint32_t light_hsl_server_status_publish(const light_hsl_server_t *p_server,
                                         const light_hsl_status_params_t *p_params);

uint32_t light_hsl_server_default_status_publish(const light_hsl_server_t *p_server,
                                                 const light_hsl_default_status_params_t *p_params);

uint32_t light_hsl_server_range_status_publish(const light_hsl_server_t *p_server,
                                               const light_hsl_range_status_params_t *p_params);



/*** Light HSL hue server */


/* user provided settings and callbacks for the HSL Hue Server model instance */
typedef struct {
    bool force_segmented;
    nrf_mesh_transmic_size_t transmic_size;
} light_hsl_hue_server_settings_t;


struct __light_hsl_hue_server_s {
    /* model handle assigned to this instance */
    access_model_handle_t model_handle;

    /* parent model context for - Level server */
    generic_level_server_t generic_level_srv;

    /* tid tracker structure */
    tid_tracker_t tid_tracker;

    /* settings and callbacks for this instance */
    light_hsl_hue_server_settings_t settings;
};


/* publishes unsolicited Status message */
uint32_t light_hsl_server_hue_status_publish(const light_hsl_hue_server_t *p_server,
                                             const light_hsl_hue_status_params_t *p_params);



/*** Light HSL saturation server */


/* user provided settings and callbacks for the HSL Saturation Server model instance */
typedef struct {
    bool force_segmented;
    nrf_mesh_transmic_size_t transmic_size;
} light_hsl_saturation_server_settings_t;


struct __light_hsl_saturation_server_s {
    /* model handle assigned to this instance */
    access_model_handle_t model_handle;

    /* parent model context for - Level server */
    generic_level_server_t generic_level_srv;

    /* tid tracker structure */
    tid_tracker_t tid_tracker;

    /* settings and callbacks for this instance */
    light_hsl_saturation_server_settings_t settings;
};


/* publishes unsolicited Status message */
uint32_t light_hsl_server_saturation_status_publish(const light_hsl_saturation_server_t *p_server,
                                                    const light_hsl_saturation_status_params_t *p_params);



/*** Light HSL Setup server */


/* callback type for the Light HSL Get message */
typedef void (*light_hsl_state_get_cb_t)(const light_hsl_setup_server_t *p_self,
                                         const access_message_rx_meta_t *p_meta,
                                         light_hsl_status_params_t *p_out);

/* callback type for the Light HSL Set/Set Unacknowledged message */
typedef void (*light_hsl_state_set_cb_t)(const light_hsl_setup_server_t *p_self,
                                         const access_message_rx_meta_t *p_meta,
                                         const light_hsl_set_params_t *p_in,
                                         const model_transition_t *p_in_transition,
                                         light_hsl_status_params_t *p_out);


/* callback type for Light HSL Hue Get message */
typedef void (*light_hsl_state_hue_get_cb_t)(const light_hsl_setup_server_t *p_self,
                                             const access_message_rx_meta_t *p_meta,
                                             light_hsl_hue_status_params_t *p_out);

/* callback type for Light HSL Hue Set message */
typedef void (*light_hsl_state_hue_set_cb_t)(const light_hsl_setup_server_t *p_self,
                                             const access_message_rx_meta_t *p_meta,
                                             const light_hsl_hue_set_params_t *p_in,
                                             const model_transition_t *p_in_transition,
                                             light_hsl_hue_status_params_t *p_out);


/* callback type for Light HSL Saturation Get message */
typedef void (*light_hsl_state_saturation_get_cb_t)(const light_hsl_setup_server_t *p_self,
                                                    const access_message_rx_meta_t *p_meta,
                                                    light_hsl_saturation_status_params_t *p_out);

/* callback type for Light HSL Saturation Set message */
typedef void (*light_hsl_state_saturation_set_cb_t)(const light_hsl_setup_server_t *p_self,
                                                    const access_message_rx_meta_t *p_meta,
                                                    const light_hsl_saturation_set_params_t *p_in,
                                                    const model_transition_t *p_in_transition,
                                                    light_hsl_saturation_status_params_t *p_out);


/* callback type for the Light HSL Default Get message */
typedef void (*light_hsl_state_default_get_cb_t)(const light_hsl_setup_server_t *p_self,
                                                 const access_message_rx_meta_t *p_meta,
                                                 light_hsl_default_status_params_t *p_out);

/* callback type for the Light HSL Gefault Set/Set Unacknowledged message */
typedef void (*light_hsl_state_default_set_cb_t)(const light_hsl_setup_server_t *p_self,
                                               const access_message_rx_meta_t *p_meta,
                                               const light_hsl_default_set_params_t *p_in,
                                               light_hsl_default_status_params_t *p_out);


/* callback type for the Light HSL Range Get message */
typedef void (*light_hsl_state_range_get_cb_t)(const light_hsl_setup_server_t *p_self,
                                               const access_message_rx_meta_t *p_meta,
                                               light_hsl_range_status_params_t *p_out);

/* callback type for the Light HSL Range Set/Set Unacknowledged message */
typedef void (*light_hsl_state_range_set_cb_t)(const light_hsl_setup_server_t *p_self,
                                               const access_message_rx_meta_t *p_meta,
                                               const light_hsl_range_set_params_t *p_in,
                                               light_hsl_range_status_params_t *p_out);

/* callback type for Light HSL delta hue Set/Set Unacknowledged "message" */
typedef void (*light_hsl_state_hue_delta_set_cb_t)(const light_hsl_setup_server_t *p_self,
                                                   const access_message_rx_meta_t *p_meta,
                                                   const light_hsl_delta_set_params_t *p_in,
                                                   const model_transition_t *p_in_transition,
                                                   light_hsl_hue_status_params_t *p_out);

/* callback type for Light HSL delta hue Set/Set Unacknowledged "message" */
typedef void (*light_hsl_state_hue_move_set_cb_t)(const light_hsl_setup_server_t *p_self,
                                                  const access_message_rx_meta_t *p_meta,
                                                  const light_hsl_move_set_params_t *p_in,
                                                  const model_transition_t *p_in_transition,
                                                  light_hsl_hue_status_params_t *p_out);

/* callback type for Light HSL delta saturation Set/Set Unacknowledged "message" */
typedef void (*light_hsl_state_saturation_delta_set_cb_t)(const light_hsl_setup_server_t *p_self,
                                                          const access_message_rx_meta_t *p_meta,
                                                          const light_hsl_delta_set_params_t *p_in,
                                                          const model_transition_t *p_in_transition,
                                                          light_hsl_saturation_status_params_t *p_out);

/* callback type for Light HSL delta saturation Set/Set Unacknowledged "message" */
typedef void (*light_hsl_state_saturation_move_set_cb_t)(const light_hsl_setup_server_t *p_self,
                                                         const access_message_rx_meta_t *p_meta,
                                                         const light_hsl_move_set_params_t *p_in,
                                                         const model_transition_t *p_in_transition,
                                                         light_hsl_saturation_status_params_t *p_out);


/* transaction callbacks for the Light CTL states */
typedef struct {
    light_hsl_state_get_cb_t                    get_cb;
    light_hsl_state_set_cb_t                    set_cb;
    light_hsl_state_hue_get_cb_t                hue_get_cb;
    light_hsl_state_hue_set_cb_t                hue_set_cb;
    light_hsl_state_saturation_get_cb_t         saturation_get_cb;
    light_hsl_state_saturation_set_cb_t         saturation_set_cb;
    light_hsl_state_default_get_cb_t            default_get_cb;
    light_hsl_state_default_set_cb_t            default_set_cb;
    light_hsl_state_range_get_cb_t              range_get_cb;
    light_hsl_state_range_set_cb_t              range_set_cb;
    light_hsl_state_hue_delta_set_cb_t          hue_delta_set_cb;
    light_hsl_state_hue_move_set_cb_t           hue_move_set_cb;
    light_hsl_state_saturation_delta_set_cb_t   saturation_delta_set_cb;
    light_hsl_state_saturation_move_set_cb_t    saturation_move_set_cb;
} light_hsl_setup_server_state_cbs_t;


/* Light CTL Setup server callback list */
typedef struct {
    /* transaction callbacks for the Light HSL states */
    light_hsl_setup_server_state_cbs_t light_hsl_cbs;
} light_hsl_setup_server_callbacks_t;


/* user provided settings and callbacks for the HSL Setup Server model instance  */
typedef struct {
    uint8_t element_index;
    bool force_segmented;
    nrf_mesh_transmic_size_t transmic_size;

    /* callback list */
    const light_hsl_setup_server_callbacks_t *p_callbacks;
} light_hsl_setup_server_settings_t;


typedef struct {
    uint8_t handle;
    bool initialized;
} light_hsl_setup_server_state_t;


struct __light_hsl_setup_server_s {
    /* model handle assigned to this instance */
    access_model_handle_t model_handle;

    /* parent model context for - HSL server. */
    light_hsl_server_t hsl_srv;

    /* parent model context for - HSL hue server. */
    light_hsl_hue_server_t hsl_hue_srv;

    /* parent model context for - HSL saturation server. */
    light_hsl_saturation_server_t hsl_saturation_srv;

    /* settings and callbacks for this instance */
    light_hsl_setup_server_settings_t settings;

    /* state for this instance */
    light_hsl_setup_server_state_t state;
};


uint32_t light_hsl_setup_server_init(light_hsl_setup_server_t *p_s_server,
                                     light_lightness_setup_server_t *p_ll_s_server,
                                     uint8_t element_index);

uint32_t light_hsl_ponoff_binding_setup(light_hsl_setup_server_t *p_s_server,
                                        light_hsl_saved_values_t * p_saved_states);



#endif /* LIGHT_HSL_SETUP_SERVER_H__ */
