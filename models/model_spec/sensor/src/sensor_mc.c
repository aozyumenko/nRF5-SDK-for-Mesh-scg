#include "sensor_mc.h"
#include "model_common.h"

#include "nrf_mesh_config_app.h"

#include "mesh_config_entry.h"
#include "mesh_config.h"
#include "mesh_opt.h"

/* Logging and RTT */
#include "log.h"



typedef struct {
    uint16_t property_id[SENSOR_SETUP_SERVER_STORED_STATES_MAX];
    uint8_t cadence[SENSOR_SETUP_SERVER_STORED_STATES_MAX][SENSOR_CADENCE_STATUS_MAX];
} state_t;



/* setter and getter declarations */
static uint32_t cadence_setter(mesh_config_entry_id_t id, const void *p_entry);
static void     cadence_getter(mesh_config_entry_id_t id, void *p_entry);


/* mesh config entry associates */
MESH_CONFIG_ENTRY(m_cadence_entry,
                  SENSOR_CADENCE_EID,
                  SENSOR_SETUP_SERVER_STORED_STATES_MAX,
                  SENSOR_CADENCE_STATUS_MAX,
                  cadence_setter,
                  cadence_getter,
                  NULL,
                  true);


/* array for mapping from a handle to a state pointer */
static state_t m_state_context[SENSOR_SETUP_SERVER_INSTANCES_MAX];
static uint8_t m_next_handle = 0;



static state_t *context_get(uint8_t index)
{
    NRF_MESH_ASSERT(index < m_next_handle);
    return &m_state_context[index];
}

static state_t *model_context_get(uint16_t address, uint16_t base)
{
    NRF_MESH_ASSERT(base <= address);
    return context_get(address - base);
}

static uint8_t property_index_get(uint8_t index, uint16_t property_id)
{
    state_t *state = context_get(index);
    uint8_t property_index;

    for (property_index = 0; property_index < ARRAY_SIZE(state->property_id); property_index++) {
        if (state->property_id[property_index] == property_id) {
            break;
        }
    }
    return property_index;
}

static void id_record_to_address_property_index(uint16_t id_record, uint16_t start,
                                                uint16_t *p_address, uint8_t *p_property_index)
{
    uint16_t shift = id_record - start;
    *p_property_index = shift / SENSOR_SETUP_SERVER_INSTANCES_MAX;
    *p_address = id_record - (*p_property_index * SENSOR_SETUP_SERVER_INSTANCES_MAX);
}

static uint16_t instance_index_property_index_to_id_record(uint8_t instance_index,
                                                            uint8_t property_index)
{
    return instance_index + (SENSOR_SETUP_SERVER_INSTANCES_MAX * property_index);
}


static void state_contexts_default_set(uint8_t index)
{
    for(uint32_t i = 0; i < SENSOR_SETUP_SERVER_STORED_STATES_MAX; i++) {
        m_state_context[index].property_id[i] = 0;
        memset(m_state_context[index].cadence[i], 0, SENSOR_CADENCE_STATUS_MAX);
    }
}

static void state_contexts_all_default_set()
{
    for (uint8_t i = 0; i < ARRAY_SIZE(m_state_context); i++) {
        state_contexts_default_set(i);
    }
}

static void sensor_flash_storage_config_clear(void)
{
    const struct {
        const mesh_config_entry_id_t *const id;
        const uint16_t start;
    } entries[] = {
        {&SENSOR_CADENCE_EID, SENSOR_CADENCE_EID_START},
    };

    state_contexts_all_default_set();

    for (uint16_t j = 0; j < ARRAY_SIZE(entries); j++) {
        mesh_config_entry_id_t id = *entries[j].id;
        for (uint8_t index = 0; index < m_next_handle; index++) {
            for (uint8_t property_index = 0; property_index < SENSOR_SETUP_SERVER_STORED_STATES_MAX; property_index++) {
                id.record = entries[j].start + (index * SENSOR_SETUP_SERVER_STORED_STATES_MAX) + property_index;
                (void) mesh_config_entry_delete(id);
            }
        }
    }
}



/* setter and getter definitions */

static uint32_t cadence_setter(mesh_config_entry_id_t id, const void *p_entry)
{
    uint16_t address;
    uint8_t property_index;

    id_record_to_address_property_index(id.record, SENSOR_CADENCE_EID_START, &address, &property_index);

    memcpy(model_context_get(address, SENSOR_CADENCE_EID_START)->cadence[property_index],
           p_entry, SENSOR_CADENCE_STATUS_MAX);
    return NRF_SUCCESS;
}

static void cadence_getter(mesh_config_entry_id_t id, void *p_entry)
{
    uint16_t address;
    uint8_t property_index;
    sensor_cadence_set_msg_pkt_t *p_value = (sensor_cadence_set_msg_pkt_t *)p_entry;

    id_record_to_address_property_index(id.record, SENSOR_CADENCE_EID_START, &address, &property_index);
    state_t *state = model_context_get(address, SENSOR_CADENCE_EID_START);

    memcpy(p_entry, state->cadence[property_index], SENSOR_CADENCE_STATUS_MAX);
    p_value->property_id = state->property_id[property_index];
}



/* API functions to set and get flash states */

uint32_t sensor_mc_cadence_get(uint8_t index,
                               uint16_t property_id,
                               sensor_cadence_set_msg_pkt_t *p_out,
                               uint16_t *p_out_bytes)
{
    uint8_t property_index = property_index_get(index, property_id);
    if (property_index == SENSOR_SETUP_SERVER_STORED_STATES_MAX) {
        return NRF_ERROR_NOT_FOUND;
    }

    mesh_config_entry_id_t id = SENSOR_CADENCE_EID;
    id.record += instance_index_property_index_to_id_record(index, property_index);

    *p_out_bytes = SENSOR_CADENCE_STATUS_MAX;
    return mesh_config_entry_get(id, p_out);
}


uint32_t sensor_mc_cadence_set(uint8_t index,
                               uint16_t property_id,
                               const sensor_cadence_set_msg_pkt_t *p_in,
                               uint16_t in_bytes)
{
    if (in_bytes > SENSOR_CADENCE_STATUS_MAX) {
        __LOG(LOG_SRC_APP, LOG_LEVEL_ERROR,
              "ERR: Sensor server: invalid cadence configuration for property 0x%x\n",
              property_id);
        return NRF_SUCCESS;
    }

    uint8_t property_index = property_index_get(index, property_id);
    if (property_index == SENSOR_SETUP_SERVER_STORED_STATES_MAX) {
        return NRF_ERROR_NOT_FOUND;
    }

    mesh_config_entry_id_t id = SENSOR_CADENCE_EID;
    id.record += instance_index_property_index_to_id_record(index, property_index);

    uint8_t value[SENSOR_CADENCE_STATUS_MAX];
    memcpy(value, p_in, in_bytes);
    memset(value + in_bytes, 0, sizeof(value) - in_bytes);

    return mesh_config_entry_set(id, &value);
}


uint32_t sensor_mc_open(uint8_t *p_handle, const uint16_t *p_sensor_property_array)
{
    if (p_handle == NULL) {
        return NRF_ERROR_NULL;
    }

    if (SENSOR_SETUP_SERVER_INSTANCES_MAX <= m_next_handle) {
        return NRF_ERROR_RESOURCES;
    }

    *p_handle = m_next_handle++;

    state_t *state = context_get(*p_handle);
    for (uint8_t i = 0; i < (uint8_t) p_sensor_property_array[0]; i++) {
        state->property_id[i] = p_sensor_property_array[i + 1];
    }

    return NRF_SUCCESS;
}


void sensor_mc_clear(void)
{
    sensor_flash_storage_config_clear();
}


void sensor_mc_init(void)
{
    state_contexts_all_default_set();
}
