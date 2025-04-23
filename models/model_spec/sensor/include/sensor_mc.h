#ifndef SENSOR_MC_H__
#define SENSOR_MC_H__

#include <stdint.h>

#include "sensor_messages.h"
#include "sensor_common.h"
#include "mesh_config.h"
#include "mesh_opt.h"
#include "model_config_file.h"



#define SENSOR_CADENCE_EID_START                (MESH_APP_MODEL_SENSOR_SERVER_ID_START)
#define SENSOR_CADENCE_EID_END                  (SENSOR_CADENCE_EID_START + SENSOR_SETUP_SERVER_STORED_STATES - 1)

/** Sensor Cadence state entry ID */
#define SENSOR_CADENCE_EID                      MESH_CONFIG_ENTRY_ID(MESH_OPT_MODEL_FILE_ID, SENSOR_CADENCE_EID_START)



/* set internal Sensor Cadence configuration */
uint32_t sensor_mc_cadence_set(uint8_t index,
                               uint16_t property_id,
                               const sensor_cadence_set_msg_pkt_t *p_in,
                               uint16_t in_bytes);

/* get internal Sensor Cadence configuration */
uint32_t sensor_mc_cadence_get(uint8_t index,
                               uint16_t property_id,
                               sensor_cadence_set_msg_pkt_t *p_out,
                               uint16_t *p_out_bytes);

uint32_t sensor_mc_open(uint8_t *p_handle, const uint16_t *p_sensor_property_array);
void sensor_mc_clear(void);
void sensor_mc_init(void);



#endif /* SENSOR_MC_H__ */
