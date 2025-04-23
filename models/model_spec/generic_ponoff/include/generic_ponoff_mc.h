#ifndef __GENERIC_PONOFF_MC_
#define __GENERIC_PONOFF_MC_


#include <stdint.h>

#include "generic_ponoff_common.h"
#include "mesh_config.h"
#include "mesh_opt.h"
#include "model_config_file.h"



/** Defines number of state instances required to store current state.
 */
#define GENERIC_PONOFF_PONOFF_EID_START         (MESH_APP_MODEL_GENERIC_PONOFF_ID_START)

/** Default Transition Time state entry ID */
#define GENERIC_PONOFF_PONOFF_EID               MESH_CONFIG_ENTRY_ID(MESH_OPT_MODEL_FILE_ID, GENERIC_PONOFF_PONOFF_EID_START)

uint32_t generic_ponoff_mc_ponoff_state_set(uint8_t index, uint8_t value);
uint32_t generic_ponoff_mc_ponoff_state_get(uint8_t index, uint8_t *p_value);
uint32_t generic_ponoff_mc_open(uint8_t *p_handle);
void generic_ponoff_mc_clear(void);
void generic_ponoff_mc_init(void);


#endif /* __GENERIC_PONOFF_MC_ */
