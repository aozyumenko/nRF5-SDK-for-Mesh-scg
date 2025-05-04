#ifndef GENERIC_BATTERY_COMMON_H__
#define GENERIC_BATTERY_COMMON_H__

#include <stdint.h>



#define GENERIC_BATTERY_LEVEL_MIN               (0)
#define GENERIC_BATTERY_LEVEL_MAX               (100)
#define GENERIC_BATTERY_LEVEL_UNKNOWN           (0xff)

#define GENERIC_BATTERY_TIME_MAX                (0xffffff)
#define GENERIC_BATTERY_TIME_UNKNOWN            (0xffffff)

#define GENERIC_BATTERY_FLAGS_PRESENCE_MAX      (3)
#define GENERIC_BATTERY_FLAGS_INDICATOR_MAX     (3)
#define GENERIC_BATTERY_FLAGS_CHARGING_MAX      (3)
#define GENERIC_BATTERY_FLAGS_SERVICE_MAX       (3)


/**
 * Unpacked message structure typedefs are used for API interfaces and for implementing model code.
 */

/** Parameters for the Generic Battery Status message. */
typedef enum
{
    GENERIC_BATTERY_FLAGS_PRESENCE_NOT_PRESENT = 0,
    GENERIC_BATTERY_FLAGS_PRESENCE_REMOVABLE = 1,
    GENERIC_BATTERY_FLAGS_PRESENCE_NON_REMOVABLE = 2,
    GENERIC_BATTERY_FLAGS_PRESENCE_UNKNOWN = 3
} generic_battery_flags_presence_t;

typedef enum
{
    GENERIC_BATTERY_FLAGS_INDICATOR_CRITICAL = 0,
    GENERIC_BATTERY_FLAGS_INDICATOR_LOW = 1,
    GENERIC_BATTERY_FLAGS_INDICATOR_GOOD = 2,
    GENERIC_BATTERY_FLAGS_INDICATOR_UNKNOWN = 3
} generic_battery_flags_indicator_t;

typedef enum
{
    GENERIC_BATTERY_FLAGS_CHARGING_NOT_CHARGEABLE = 0,
    GENERIC_BATTERY_FLAGS_CHARGING_NOT_CHARGING = 1,
    GENERIC_BATTERY_FLAGS_CHARGING_CHARGING = 2,
    GENERIC_BATTERY_FLAGS_CHARGING_UNKNOWN = 3
} generic_battery_flags_charging_t;

typedef enum
{
    GENERIC_BATTERY_FLAGS_SERVICE_RSVD = 0,
    GENERIC_BATTERY_FLAGS_SERVICE_NOT_REQUIRE = 1,
    GENERIC_BATTERY_FLAGS_SERVICE_REQUIRE = 2,
    GENERIC_BATTERY_FLAGS_SERVICE_UNKNOWN = 3
} generic_battery_flags_service_t;

typedef struct __attribute((packed))
{
    uint8_t battery_level;                                  /**< battery level value in percentage */
    uint32_t discharge_time;                                /**< time to discharge in minutes */
    uint32_t charge_time;                                   /**< time to charge in minutes */
    generic_battery_flags_presence_t flags_presence;        /**< presence flags */
    generic_battery_flags_indicator_t flags_indicator;      /**< indicator flags */
    generic_battery_flags_charging_t flags_charging;        /**< charging flags */
    generic_battery_flags_service_t flags_service;          /**< serviceability flags */
} generic_battery_status_params_t;


/** Packing time parameters to the Generic Battery Status message. */
inline void generic_battery_time_pack(uint8_t *dest, uint32_t src)
{
    dest[0] = (src >> 16) & 0xff;
    dest[1] = (src >> 8) & 0xff;
    dest[1] = (src) & 0xff;
}



#endif /* GENERIC_BATTERY_COMMON_H__ */
