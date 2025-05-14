#ifndef __APP_LIGHT_HSL_H_
#define __APP_LIGHT_HSL_H_

#include <stdint.h>

#include "light_hsl_setup_server.h"
#include "light_lightness_setup_server.h"
#include "app_timer.h"
#include "app_transition.h"
#include "app_light_lightness.h"
#include "list.h"
#if (SCENE_SETUP_SERVER_INSTANCES_MAX > 0) || (DOXYGEN)
#include "app_scene.h"
#endif



/* Light HSL hardware state format for transfer of hue and saturation between 
   mid app and top (main.c) */
typedef struct app_light_hsl_hw_state_s {
    uint16_t hue;
    uint16_t saturation;
} app_light_hsl_hw_state_t;


/* internal structure to hold state and timing information */
typedef struct app_light_hsl_state_s {
    // hue state
    uint16_t present_hue;
    uint16_t target_hue;
    uint16_t snapshot_present_hue;
    uint16_t snapshot_target_hue;
    uint16_t initial_present_hue;
    app_transition_t transition_hue;
    bool new_tid_hue;                   /* used for Generic Level delta/move transaction identifier */
    uint32_t published_hue_ms;          /* control published time for transition */

    // saturation state
    uint16_t present_saturation;
    uint16_t target_saturation;
    uint16_t snapshot_present_saturation;
    uint16_t snapshot_target_saturation;
    uint16_t initial_present_saturation;
    app_transition_t transition_saturation;
    bool new_tid_saturation;
    uint32_t published_saturation_ms;

} app_light_hsl_state_t;



/* forward declaration */
typedef struct app_light_hsl_setup_server_s app_light_hsl_setup_server_t;



/* application Light HSL (hue/saturation) state set callback prototype */
typedef void (*app_light_hsl_set_cb_t)(const app_light_hsl_setup_server_t *p_app,
                                       const app_light_hsl_hw_state_t *hsl_state);

/* application Light HSL (hue/saturation) state get callback prototype */
typedef void (*app_light_hsl_get_cb_t)(const app_light_hsl_setup_server_t *p_app,
                                       app_light_hsl_hw_state_t *p_present_hsl_state);

/* aplication Light HSL transition time callback prototype */
typedef void (*app_light_hsl_transition_cb_t)(const app_light_hsl_setup_server_t *p_app,
                                              uint32_t transition_time_ms,
                                              app_light_hsl_hw_state_t target_hsl_state);



/* application level structure holding the Light HSL Setup Server model context */
struct app_light_hsl_setup_server_s {
    /** internal variable */
    list_node_t node;
    /* HSL setup server context */
    light_hsl_setup_server_t light_hsl_setup_srv;

    /** app server for light lightness */
    app_light_lightness_setup_server_t *p_app_ll;

    /** representation of the Light HSL state */
    app_light_hsl_state_t state;

    /* keep track if HSL, hue, and saturation state set is active */
    bool hsl_state_set_active;
    bool hsl_state_hue_set_active;
    bool hsl_state_saturation_set_active;

    /** Internal variable. Used for scheduling transition abort. */
    bool abort_move_hue;
    bool abort_move_saturation;

#if (SCENE_SETUP_SERVER_INSTANCES_MAX > 0)
    /** Internal variable. Scene callback interface.
     * @note Available only if  @ref SCENE_SETUP_SERVER_INSTANCES_MAX is equal or larger than 1. */
    app_scene_model_interface_t scene_if;
    /** Internal variable. Pointer to app_scene context.
     * @note Available only if  @ref SCENE_SETUP_SERVER_INSTANCES_MAX is equal or larger than 1. */
    app_scene_setup_server_t  *p_app_scene;
#endif

    /* set the device temp/duv values callback */
    app_light_hsl_set_cb_t app_light_hsl_set_cb;

    /* get the device hue/saturation value callback */
    app_light_hsl_get_cb_t app_light_hsl_get_cb;

    /* device Light HSL transition time callback */
    app_light_hsl_transition_cb_t app_light_hsl_transition_cb;
};


/* macro to create application level app_light_hsl_setup_server_t context  */
#define APP_LIGHT_HSL_SETUP_SERVER_DEF(_name, _force_segmented, _mic_size, _light_hsl_set_cb, _light_hsl_get_cb, _light_hsl_transition_cb) \
    APP_TIMER_DEF(_name ## _timer_hue);                                                 \
    APP_TIMER_DEF(_name ## _timer_saturation);                                          \
    static app_light_hsl_setup_server_t _name =                                         \
    {                                                                                   \
        .light_hsl_setup_srv.settings.force_segmented = _force_segmented,               \
        .light_hsl_setup_srv.settings.transmic_size = _mic_size,                        \
        .state.transition_hue.timer.p_timer_id = &_name ## _timer_hue,                  \
        .state.transition_saturation.timer.p_timer_id = &_name ## _timer_saturation,    \
        .app_light_hsl_set_cb = _light_hsl_set_cb,                                      \
        .app_light_hsl_get_cb = _light_hsl_get_cb,                                      \
        .app_light_hsl_transition_cb = _light_hsl_transition_cb                         \
    };


uint32_t app_light_hsl_model_init(app_light_hsl_setup_server_t *p_app, uint8_t element_index,
                                  app_light_lightness_setup_server_t *p_app_ll);

uint32_t app_light_hsl_binding_setup(app_light_hsl_setup_server_t *p_app);

uint32_t app_light_hsl_current_value_publish(app_light_hsl_setup_server_t *p_app);

#if (SCENE_SETUP_SERVER_INSTANCES_MAX > 0)
uint32_t app_light_hsl_scene_context_set(app_light_hsl_setup_server_t *p_app,
                                         app_scene_setup_server_t *p_app_scene);
#endif


#endif /*__APP_LIGHT_HSL_H_ */
