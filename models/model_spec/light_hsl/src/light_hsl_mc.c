#include "light_hsl_mc.h"
#include "nrf_mesh_config_app.h"
#include "mesh_config_entry.h"
#include "mesh_opt.h"
#include "light_hsl_common.h"


#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
#define STORED_WITH_SCENE_STATE    (1 + SCENE_REGISTER_ARRAY_SIZE)
#else
#define STORED_WITH_SCENE_STATE    (1)
#endif

typedef struct {
    uint16_t hue[STORED_WITH_SCENE_STATE];
    uint16_t saturation[STORED_WITH_SCENE_STATE];
    uint16_t hue_default;
    uint16_t saturation_default;
    light_hsl_range_set_params_t range;
} state_t;


/* setter and getter declarations */

static uint32_t hue_setter(mesh_config_entry_id_t id, const void *p_entry);
static void     hue_getter(mesh_config_entry_id_t id, void *p_entry);

static uint32_t saturation_setter(mesh_config_entry_id_t id, const void *p_entry);
static void     saturation_getter(mesh_config_entry_id_t id, void *p_entry);

static uint32_t hue_default_setter(mesh_config_entry_id_t id, const void *p_entry);
static void     hue_default_getter(mesh_config_entry_id_t id, void *p_entry);

static uint32_t saturation_default_setter(mesh_config_entry_id_t id, const void *p_entry);
static void     saturation_default_getter(mesh_config_entry_id_t id, void *p_entry);

static uint32_t range_setter(mesh_config_entry_id_t id, const void *p_entry);
static void     range_getter(mesh_config_entry_id_t id, void *p_entry);


NRF_MESH_STATIC_ASSERT((MESH_APP_MODEL_LIGHT_HSL_SERVER_ID_START + \
                       (6 * LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX) - 1) \
                       <= MESH_APP_MODEL_LIGHT_HSL_SERVER_ID_END);

NRF_MESH_STATIC_ASSERT(LIGHT_HSL_DEFAULT_RANGE_HUE_MIN >= LIGHT_HSL_RANGE_HUE_MIN);
NRF_MESH_STATIC_ASSERT(LIGHT_HSL_DEFAULT_RANGE_HUE_MAX <= LIGHT_HSL_RANGE_HUE_MAX);
NRF_MESH_STATIC_ASSERT(LIGHT_HSL_DEFAULT_RANGE_SATURATION_MIN >= LIGHT_HSL_RANGE_SATURATION_MIN);
NRF_MESH_STATIC_ASSERT(LIGHT_HSL_DEFAULT_RANGE_SATURATION_MAX <= LIGHT_HSL_RANGE_SATURATION_MAX);
NRF_MESH_STATIC_ASSERT(LIGHT_HSL_DEFAULT_RANGE_HUE_MAX >= LIGHT_HSL_DEFAULT_RANGE_HUE_MIN);
NRF_MESH_STATIC_ASSERT(LIGHT_HSL_DEFAULT_RANGE_SATURATION_MAX >= LIGHT_HSL_DEFAULT_RANGE_SATURATION_MIN);

/* Each element can host only one ctl instance.*/
STATIC_ASSERT(LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX \
              <= ACCESS_ELEMENT_COUNT, \
              "LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX is out of range");

/* mesh config entry associates */
MESH_CONFIG_ENTRY(m_hue_entry,
                  LIGHT_HSL_HUE_EID,
                  LIGHT_HSL_SETUP_SERVER_STORED_WITH_SCENE_STATES,
                  sizeof(uint16_t),
                  hue_setter,
                  hue_getter,
                  NULL,
                  true);

MESH_CONFIG_ENTRY(m_saturation_entry,
                  LIGHT_HSL_SATURATION_EID,
                  LIGHT_HSL_SETUP_SERVER_STORED_WITH_SCENE_STATES,
                  sizeof(uint16_t),
                  saturation_setter,
                  saturation_getter,
                  NULL,
                  true);

MESH_CONFIG_ENTRY(m_hue_default_entry,
                  LIGHT_HSL_HUE_DEFAULT_EID,
                  LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX,
                  sizeof(uint16_t),
                  hue_default_setter,
                  hue_default_getter,
                  NULL,
                  true);

MESH_CONFIG_ENTRY(m_saturation_default_entry,
                  LIGHT_HSL_SATURATION_DEFAULT_EID,
                  LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX,
                  sizeof(uint16_t),
                  saturation_default_setter,
                  saturation_default_getter,
                  NULL,
                  true);

MESH_CONFIG_ENTRY(m_range_entry,
                  LIGHT_HSL_RANGE_EID,
                  LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX,
                  sizeof(light_hsl_range_set_params_t),
                  range_setter,
                  range_getter,
                  NULL,
                  true);




/* array for mapping from a handle to a state pointer */
static state_t m_state_context[LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX];
static uint8_t m_next_handle = 0;


static state_t *context_get(uint8_t i)
{
    NRF_MESH_ASSERT(i < m_next_handle);
    return &m_state_context[i];
}


static state_t *model_context_get(uint16_t address, uint16_t base)
{
    NRF_MESH_ASSERT(base <= address);
    return context_get(address - base);
}


static void state_contexts_default_set(uint8_t handle)
{
    for(uint32_t i = 0; i < STORED_WITH_SCENE_STATE; i++) {
        m_state_context[handle].hue[i] = LIGHT_HSL_DEFAULT_HUE;
        m_state_context[handle].saturation[i] = LIGHT_HSL_DEFAULT_SATURATION;
    }
    m_state_context[handle].hue_default = LIGHT_HSL_DEFAULT_HUE_DEFAULT;
    m_state_context[handle].saturation_default = LIGHT_HSL_DEFAULT_SATURATION_DEFAULT;
    m_state_context[handle].range.hue_range_min = LIGHT_HSL_DEFAULT_RANGE_HUE_MIN;
    m_state_context[handle].range.hue_range_max = LIGHT_HSL_DEFAULT_RANGE_HUE_MAX;
    m_state_context[handle].range.saturation_range_min = LIGHT_HSL_DEFAULT_RANGE_SATURATION_MIN;
    m_state_context[handle].range.saturation_range_max = LIGHT_HSL_DEFAULT_RANGE_SATURATION_MAX;
}


static void state_contexts_all_default_set()
{
    for (uint8_t i = 0; i < ARRAY_SIZE(m_state_context); i++) {
        state_contexts_default_set(i);
    }
}


static void hsl_flash_storage_config_clear(void)
{
    const struct {
        const mesh_config_entry_id_t *const id;
        const uint16_t start;
    } entries[] = {
        {&LIGHT_HSL_HUE_EID, LIGHT_HSL_HUE_EID_START},
        {&LIGHT_HSL_SATURATION_EID, LIGHT_HSL_SATURATION_EID_START},
        {&LIGHT_HSL_HUE_DEFAULT_EID, LIGHT_HSL_HUE_DEFAULT_EID_START},
        {&LIGHT_HSL_SATURATION_DEFAULT_EID, LIGHT_HSL_SATURATION_DEFAULT_EID_START},
        {&LIGHT_HSL_RANGE_EID, LIGHT_HSL_RANGE_EID_START},
    };

    state_contexts_all_default_set();

    for (uint16_t j = 0; j < ARRAY_SIZE(entries); j++) {
        mesh_config_entry_id_t id = *entries[j].id;
        for (uint8_t i = 0; i < m_next_handle; i++) {
            if (entries[j].start == LIGHT_HSL_HUE_EID_START ||
                    entries[j].start == LIGHT_HSL_SATURATION_EID_START) {
                for (uint32_t k = 0; k < STORED_WITH_SCENE_STATE; k++) {
                    id.record = entries[j].start + (i * STORED_WITH_SCENE_STATE) + k;
                    (void) mesh_config_entry_delete(id);
                }
            } else {
                id.record = entries[j].start + i;
                (void) mesh_config_entry_delete(id);
            }
        }
    }
}


static void id_record_to_address_array_index(uint16_t id_record, uint16_t start,
                                             uint16_t *p_address, uint8_t *p_array_index)
{
    uint16_t shift = id_record - start;
    *p_array_index = shift / LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX;
    *p_address = id_record - (*p_array_index * LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX);
}


#if SCENE_SETUP_SERVER_INSTANCES_MAX > 0
static uint16_t hsl_instance_index_array_index_to_id_record(uint8_t hsl_instance_index,
                                                            uint8_t scene_index)
{
    uint8_t array_index = scene_index % SCENE_REGISTER_ARRAY_SIZE;
    return hsl_instance_index  + (LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX * (array_index + 1));
}
#endif



/* setter and getter definitions */

static uint32_t hue_setter(mesh_config_entry_id_t id, const void *p_entry)
{
    uint16_t address;
    uint8_t array_index;
    const uint16_t *p_value = (const uint16_t *)p_entry;

    id_record_to_address_array_index(id.record, LIGHT_HSL_HUE_EID_START, &address, &array_index);

    model_context_get(address, LIGHT_HSL_HUE_EID_START)->hue[array_index] = *p_value;
    return NRF_SUCCESS;
}

static void hue_getter(mesh_config_entry_id_t id, void *p_entry)
{
    uint16_t address;
    uint8_t array_index;
    uint16_t *p_value = (uint16_t *)p_entry;

    id_record_to_address_array_index(id.record, LIGHT_HSL_HUE_EID_START, &address, &array_index);
    *p_value = model_context_get(address, LIGHT_HSL_HUE_EID_START)->hue[array_index];
}


static uint32_t saturation_setter(mesh_config_entry_id_t id, const void *p_entry)
{
    uint16_t address;
    uint8_t array_index;
    const uint16_t *p_value = (const uint16_t *)p_entry;

    id_record_to_address_array_index(id.record, LIGHT_HSL_SATURATION_EID_START, &address, &array_index);
 
    model_context_get(address, LIGHT_HSL_SATURATION_EID_START)->saturation[array_index] = *p_value;
    return NRF_SUCCESS;
}

static void saturation_getter(mesh_config_entry_id_t id, void *p_entry)
{
    uint16_t address;
    uint8_t array_index;
    uint16_t *p_value = (uint16_t *)p_entry;

    id_record_to_address_array_index(id.record, LIGHT_HSL_SATURATION_EID_START, &address, &array_index);
    *p_value = model_context_get(address, LIGHT_HSL_SATURATION_EID_START)->saturation[array_index];
}


static uint32_t hue_default_setter(mesh_config_entry_id_t id, const void *p_entry)
{
    const uint16_t *p_value = (const uint16_t *)p_entry;

    model_context_get(id.record, LIGHT_HSL_HUE_DEFAULT_EID_START)->hue_default = *p_value;
    return NRF_SUCCESS;
}

static void hue_default_getter(mesh_config_entry_id_t id, void *p_entry)
{
    uint16_t *p_value = (uint16_t *)p_entry;

    *p_value = model_context_get(id.record, LIGHT_HSL_HUE_DEFAULT_EID_START)->hue_default;
}


static uint32_t saturation_default_setter(mesh_config_entry_id_t id, const void *p_entry)
{
    const uint16_t *p_value = (const uint16_t *)p_entry;

    model_context_get(id.record, LIGHT_HSL_SATURATION_DEFAULT_EID_START)->saturation_default = *p_value;
    return NRF_SUCCESS;
}

static void saturation_default_getter(mesh_config_entry_id_t id, void *p_entry)
{
    uint16_t *p_value = (uint16_t *)p_entry;

    *p_value = model_context_get(id.record, LIGHT_HSL_SATURATION_DEFAULT_EID_START)->saturation_default;
}


static uint32_t range_setter(mesh_config_entry_id_t id, const void *p_entry)
{
    const light_hsl_range_set_params_t *p_value =
        (const light_hsl_range_set_params_t  *)p_entry;

    model_context_get(id.record, LIGHT_HSL_RANGE_EID_START)->range = *p_value;
    return NRF_SUCCESS;
}

static void range_getter(mesh_config_entry_id_t id, void *p_entry)
{
    light_hsl_range_set_params_t *p_value = (light_hsl_range_set_params_t *)p_entry;

    *p_value = model_context_get(id.record, LIGHT_HSL_RANGE_EID_START)->range;
}



/* API functions to set and get flash states */

uint32_t light_hsl_mc_hue_state_set(uint8_t index, uint16_t value)
{
    mesh_config_entry_id_t id = LIGHT_HSL_HUE_EID;

    id.record += index;
    return mesh_config_entry_set(id, &value);
}


uint32_t light_hsl_mc_hue_state_get(uint8_t index, uint16_t *p_value)
{
    mesh_config_entry_id_t id = LIGHT_HSL_HUE_EID;

    id.record += index;
    return mesh_config_entry_get(id, p_value);
}


uint32_t light_hsl_mc_saturation_state_set(uint8_t index, uint16_t value)
{
    mesh_config_entry_id_t id = LIGHT_HSL_SATURATION_EID;

    id.record += index;
    return mesh_config_entry_set(id, &value);
}


uint32_t light_hsl_mc_saturation_state_get(uint8_t index, uint16_t *p_value)
{
    mesh_config_entry_id_t id = LIGHT_HSL_SATURATION_EID;

    id.record += index;
    return mesh_config_entry_get(id, p_value);
}


uint32_t light_hsl_mc_default_hue_state_set(uint8_t index, uint16_t value)
{
    mesh_config_entry_id_t id = LIGHT_HSL_HUE_DEFAULT_EID;

    id.record += index;
    return mesh_config_entry_set(id, &value);
}


uint32_t light_hsl_mc_default_hue_state_get(uint8_t index, uint16_t *p_value)
{
    mesh_config_entry_id_t id = LIGHT_HSL_HUE_DEFAULT_EID;

    id.record += index;
    return mesh_config_entry_get(id, p_value);
}


uint32_t light_hsl_mc_default_saturation_state_set(uint8_t index, uint16_t value)
{
    mesh_config_entry_id_t id = LIGHT_HSL_SATURATION_DEFAULT_EID;

    id.record += index;
    return mesh_config_entry_set(id, &value);
}


uint32_t light_hsl_mc_default_saturation_state_get(uint8_t index, uint16_t *p_value)
{
    mesh_config_entry_id_t id = LIGHT_HSL_SATURATION_DEFAULT_EID;

    id.record += index;
    return mesh_config_entry_get(id, p_value);
}


uint32_t light_hsl_mc_range_state_set(uint8_t index, light_hsl_range_set_params_t *p_value)
{
    mesh_config_entry_id_t id = LIGHT_HSL_RANGE_EID;

    id.record += index;
    return mesh_config_entry_set(id, p_value);
}


uint32_t light_hsl_mc_range_state_get(uint8_t index, light_hsl_range_set_params_t *p_value)
{
    mesh_config_entry_id_t id = LIGHT_HSL_RANGE_EID;

    id.record += index;
    return mesh_config_entry_get(id, p_value);
}


#if (SCENE_SETUP_SERVER_INSTANCES_MAX > 0)

uint32_t light_hsl_mc_scene_hue_state_store(uint8_t index, uint8_t scene_index, uint16_t value)
{
    mesh_config_entry_id_t id = LIGHT_HSL_HUE_EID;

    id.record += hsl_instance_index_array_index_to_id_record(index, scene_index);

    return mesh_config_entry_set(id, &value);
}


uint32_t light_hsl_mc_scene_hue_state_recall(uint8_t index, uint8_t scene_index, uint16_t *p_value)
{
    mesh_config_entry_id_t id = LIGHT_HSL_HUE_EID;

    id.record += hsl_instance_index_array_index_to_id_record(index, scene_index);

    return mesh_config_entry_get(id, p_value);
}


uint32_t light_hsl_mc_scene_saturation_state_store(uint8_t index, uint8_t scene_index, uint16_t value)
{
    mesh_config_entry_id_t id = LIGHT_HSL_SATURATION_EID;

    id.record += hsl_instance_index_array_index_to_id_record(index, scene_index);

    return mesh_config_entry_set(id, &value);
}


uint32_t light_hsl_mc_scene_saturation_state_recall(uint8_t index, uint8_t scene_index, uint16_t *p_value)
{
    mesh_config_entry_id_t id = LIGHT_HSL_SATURATION_EID;

    id.record += hsl_instance_index_array_index_to_id_record(index, scene_index);

    return mesh_config_entry_get(id, p_value);
}

#endif /* (SCENE_SETUP_SERVER_INSTANCES_MAX > 0) || (DOXYGEN) */


uint32_t light_hsl_mc_open(uint8_t *p_handle)
{
    if (p_handle == NULL) {
        return NRF_ERROR_NULL;
    }

    if (LIGHT_HSL_SETUP_SERVER_INSTANCES_MAX <= m_next_handle) {
        return NRF_ERROR_RESOURCES;
    }

    *p_handle = m_next_handle++;
    return NRF_SUCCESS;
}


void light_hsl_mc_clear(void)
{
    hsl_flash_storage_config_clear();
}


void light_hsl_mc_init(void)
{
    state_contexts_all_default_set();
}
