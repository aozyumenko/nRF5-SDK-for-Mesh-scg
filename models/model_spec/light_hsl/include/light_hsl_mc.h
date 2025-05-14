#ifndef LIGHT_HSL_MC_H__
#define LIGHT_HSL_MC_H__

#include <stdint.h>

#include "light_hsl_common.h"
#include "mesh_config.h"
#include "mesh_opt.h"
#include "model_config_file.h"

#if (SCENE_SETUP_SERVER_INSTANCES_MAX > 0)
#include "scene_common.h"
#endif


#if (SCENE_SETUP_SERVER_INSTANCES_MAX > 0)
#define LIGHT_HSL_SETUP_SERVER_STORED_WITH_SCENE_STATES \
                                    (LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX + (SCENE_REGISTER_ARRAY_SIZE * LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX))
#else
#define LIGHT_HSL_SETUP_SERVER_STORED_WITH_SCENE_STATES \
                                    (LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX)
#endif

#define LIGHT_HSL_HUE_EID_START                 (MESH_APP_MODEL_LIGHT_HSL_SERVER_ID_START)
#define LIGHT_HSL_HUE_EID_END                   (LIGHT_HSL_HUE_EID_START + LIGHT_HSL_SETUP_SERVER_STORED_WITH_SCENE_STATES - 1)
#define LIGHT_HSL_SATURATION_EID_START          (LIGHT_HSL_HUE_EID_END + 1)
#define LIGHT_HSL_SATURATION_EID_END            (LIGHT_HSL_SATURATION_EID_START + LIGHT_HSL_SETUP_SERVER_STORED_WITH_SCENE_STATES - 1)
#define LIGHT_HSL_HUE_DEFAULT_EID_START         (LIGHT_HSL_SATURATION_EID_END + 1)
#define LIGHT_HSL_HUE_DEFAULT_EID_END           (LIGHT_HSL_HUE_DEFAULT_EID_START + LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX - 1)
#define LIGHT_HSL_SATURATION_DEFAULT_EID_START  (LIGHT_HSL_HUE_DEFAULT_EID_END + 1)
#define LIGHT_HSL_SATURATION_DEFAULT_EID_END    (LIGHT_HSL_SATURATION_DEFAULT_EID_START + LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX - 1)
#define LIGHT_HSL_RANGE_EID_START               (LIGHT_HSL_SATURATION_DEFAULT_EID_END + 1)
#define LIGHT_HSL_RANGE_EID_END                 (LIGHT_HSL_RANGE_EID_START + LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX - 1)

/** Light HSL Hue state entry ID */
#define LIGHT_HSL_HUE_EID                       MESH_CONFIG_ENTRY_ID(MESH_OPT_MODEL_FILE_ID, LIGHT_HSL_HUE_EID_START)

/** Light HSL Hue Default state entry ID */
#define LIGHT_HSL_HUE_DEFAULT_EID               MESH_CONFIG_ENTRY_ID(MESH_OPT_MODEL_FILE_ID, LIGHT_HSL_HUE_DEFAULT_EID_START)

/** Light HSL Saturation state entry ID */
#define LIGHT_HSL_SATURATION_EID                MESH_CONFIG_ENTRY_ID(MESH_OPT_MODEL_FILE_ID, LIGHT_HSL_SATURATION_EID_START)

/** Light HSL Saturation Default state entry ID */
#define LIGHT_HSL_SATURATION_DEFAULT_EID        MESH_CONFIG_ENTRY_ID(MESH_OPT_MODEL_FILE_ID, LIGHT_HSL_SATURATION_DEFAULT_EID_START)

/** Light HSL Range state entry ID */
#define LIGHT_HSL_RANGE_EID                     MESH_CONFIG_ENTRY_ID(MESH_OPT_MODEL_FILE_ID, LIGHT_HSL_RANGE_EID_START)



/* set internal Light HSL Hue state variable */
uint32_t light_hsl_mc_hue_state_set(uint8_t index, uint16_t value);

/* get internal Light HSL Hue state variable */
uint32_t light_hsl_mc_hue_state_get(uint8_t index, uint16_t *p_value);

/* set internal Light HSL Saturation state variable */
uint32_t light_hsl_mc_saturation_state_set(uint8_t index, uint16_t value);

/* get internal Light HSL Saturation state variable */
uint32_t light_hsl_mc_saturation_state_get(uint8_t index, uint16_t *p_value);

/* set internal Light HSL Hue Default state variable */
uint32_t light_hsl_mc_default_hue_state_set(uint8_t index, uint16_t value);

/* get internal Light HSL Hue Default state variable */
uint32_t light_hsl_mc_default_hue_state_get(uint8_t index, uint16_t *p_value);

/* set internal Light HSL Saturation Default state variable */
uint32_t light_hsl_mc_default_saturation_state_set(uint8_t index, uint16_t value);

/* get internal Light HSL Saturation Default state variable */
uint32_t light_hsl_mc_default_saturation_state_get(uint8_t index, uint16_t *p_value);

/* set internal Range state variable */
uint32_t light_hsl_mc_range_state_set(uint8_t index, light_hsl_range_set_params_t *p_value);

/* get internal Range Status state variable */
uint32_t light_hsl_mc_range_state_get(uint8_t index, light_hsl_range_set_params_t *p_value);


#if (SCENE_SETUP_SERVER_INSTANCES_MAX > 0) || (DOXYGEN)

/* stores internal Light HSL Hue state variable for a specific scene index */
uint32_t light_hsl_mc_scene_hue_state_store(uint8_t index, uint8_t scene_index, uint16_t value);

/* recalls internal Light HSL Hue state variable for a specific scene index */
uint32_t light_hsl_mc_scene_hue_state_recall(uint8_t index, uint8_t scene_index, uint16_t *p_value);

/* stores internal Light HSL Saturation state variable for a specific scene index */
uint32_t light_hsl_mc_scene_saturation_state_store(uint8_t index, uint8_t scene_index, uint16_t value);

/* recalls internal Light HSL Saturation state variable for a specific scene index */
uint32_t light_hsl_mc_scene_saturation_state_recall(uint8_t index, uint8_t scene_index, uint16_t *p_value);

#endif /* (SCENE_SETUP_SERVER_INSTANCES_MAX > 0) */


uint32_t light_hsl_mc_open(uint8_t * p_handle);
void light_hsl_mc_clear(void);
void light_hsl_mc_init(void);



#endif /* LIGHT_HSL_MC_H__ */
