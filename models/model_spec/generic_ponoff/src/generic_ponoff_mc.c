#include "generic_ponoff_mc.h"
#include "model_common.h"

#include "nrf_mesh_config_app.h"

#include "mesh_config_entry.h"
#include "mesh_config.h"
#include "mesh_opt.h"

#include "nrf_mesh_assert.h"


NRF_MESH_STATIC_ASSERT((MESH_APP_MODEL_GENERIC_PONOFF_ID_START + \
                        GENERIC_PONOFF_SETUP_SERVER_INSTANCES_MAX - 1) \
                       <= MESH_APP_MODEL_GENERIC_PONOFF_ID_END);


typedef struct {
    uint8_t on_powerup;
} ponoff_flash_storage_state_t;

static uint32_t ponoff_setter(mesh_config_entry_id_t id, const void *p_entry);
static void ponoff_getter(mesh_config_entry_id_t id, void *p_entry);


MESH_CONFIG_ENTRY(m_ponoff_entry,
                  GENERIC_PONOFF_PONOFF_EID,
                  GENERIC_PONOFF_SETUP_SERVER_INSTANCES_MAX,
                  sizeof(uint8_t),
                  ponoff_setter,
                  ponoff_getter,
                  NULL,
                  true);

/* An array for mapping from a handle to a state pointer.
 */
static ponoff_flash_storage_state_t m_state_contexts[GENERIC_PONOFF_SETUP_SERVER_INSTANCES_MAX];
static uint8_t m_next_handle = 0;


static ponoff_flash_storage_state_t *context_get(uint8_t i)
{
    NRF_MESH_ASSERT(i < m_next_handle);
    return &m_state_contexts[i];
}

//  maps from (ID record, ID record base)-pairs to state pointer
static ponoff_flash_storage_state_t *model_context_get(uint16_t address, uint16_t base)
{
    NRF_MESH_ASSERT(base <= address);
    return context_get(address - base);
}


// default value
static void state_contexts_default_set(uint8_t handle)
{
    m_state_contexts[handle].on_powerup = GENERIC_ON_POWERUP_DEFAULT;
}

static void state_contexts_all_default_set()
{
    for (uint8_t i = 0; i < ARRAY_SIZE(m_state_contexts); i++) {
        state_contexts_default_set(i);
    }
}


// clear nv storage
static void ponoff_flash_storage_config_clear(void)
{
    const struct {
        const mesh_config_entry_id_t *const id;
        const uint16_t start;
    } entries[] = {
        {&GENERIC_PONOFF_PONOFF_EID, GENERIC_PONOFF_PONOFF_EID_START},
    };

    state_contexts_all_default_set();

    for (uint16_t j=0; j < ARRAY_SIZE(entries); j++) {
        mesh_config_entry_id_t id = *entries[j].id;
        for (uint8_t i = 0; i < m_next_handle; i++) {
            id.record = entries[j].start + i;
            (void) mesh_config_entry_delete(id);
        }
    }
}




// getter/setter definition
static uint32_t ponoff_setter(mesh_config_entry_id_t id, const void * p_entry)
{
    const uint8_t *p_value = (const uint8_t *)p_entry;

    model_context_get(id.record, GENERIC_PONOFF_PONOFF_EID_START)->on_powerup = *p_value;

    return NRF_SUCCESS;
}

static void ponoff_getter(mesh_config_entry_id_t id, void *p_entry)
{
    uint8_t * p_value = (uint8_t *) p_entry;

    *p_value = model_context_get(id.record, GENERIC_PONOFF_PONOFF_EID_START)->on_powerup;
}



// interface function

uint32_t generic_ponoff_mc_ponoff_state_set(uint8_t index, uint8_t value)
{
    mesh_config_entry_id_t id = GENERIC_PONOFF_PONOFF_EID;

    id.record += index;
    return mesh_config_entry_set(id, &value);
}


uint32_t generic_ponoff_mc_ponoff_state_get(uint8_t index, uint8_t *p_value)
{
    mesh_config_entry_id_t id = GENERIC_PONOFF_PONOFF_EID;

    id.record += index;
    return mesh_config_entry_get(id, p_value);
}


uint32_t generic_ponoff_mc_open(uint8_t *p_handle)
{
    if (p_handle == NULL) {
        return NRF_ERROR_NULL;
    }

    if (GENERIC_DTT_SERVER_INSTANCES_MAX <= m_next_handle) {
        return NRF_ERROR_RESOURCES;
    }

    *p_handle = m_next_handle++;
    return NRF_SUCCESS;
}


void generic_ponoff_mc_clear(void)
{
    ponoff_flash_storage_config_clear();
}


void generic_ponoff_mc_init(void)
{
    state_contexts_all_default_set();
}
