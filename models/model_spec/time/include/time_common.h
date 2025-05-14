/* (C) 2024, Alexander Ozumenko
 * St(u)dio of Computer Games (https://stdio.ru/)
 */

#ifndef TIME_COMMON_H__
#define TIME_COMMON_H__


#include <stdint.h>
#include <stdbool.h>


/** Model Company ID */
#define TIME_COMPANY_ID         (0xFFFF)

/** Maximum value of the Time Role state. */
#define GENERIC_ON_POWERUP_MAX  (0x03)

/** ... */
#define SECONDS_15_MINUTES      (15 * 60)
#define MESH_UNIX_EPOCH_DIFF    (946684800)



/** Valid values of Time Role state. */
typedef enum
{
    /** The element does not participate in propagation of time information. */
    TIME_ROLE_NONE = 0,
    /** The element publishes Time Status messages but does not process received Time Status messages. */
    TIME_ROLE_MESH_TIME_AUTHORITY,
    /** The element processes received and publishes Time Status messages. */
    TIME_ROLE_MESH_TIME_RELAY,
    /** The element does not publish but processes received Time Status messages. */
    TIME_ROLE_TIME_TIME_CLIENT
} time_role_values_t;


/* unpacked message structure typedefs are used for API interfaces and for implementing model code */

/* parameters for the Time Status message */
typedef struct {
    uint64_t TAI_seconds;
    uint8_t subsecond;
    uint8_t uncertainty;
    bool time_authority;
    uint16_t tai_utc_delta;
    uint8_t time_zone_offset;
} time_status_param_t;


/* parameters for the Time Role Status message */
typedef struct {
    time_role_values_t time_role;
} time_role_status_param_t;


/* parameters for the Time Zone Status message */
typedef struct {
    uint8_t time_zone_offset_current;
    uint8_t time_zone_offset_new;
    uint64_t tai_zone_change;
} time_zone_status_param_t;


/* parameters for the TAI-UTC Delta Status message */
typedef struct {
    uint16_t tai_utc_delta_current;
    uint16_t tai_utc_delta_new;
    uint64_t tai_delta_change;
} tai_utc_delta_status_param_t;


#endif /* TIME_COMMON_H__ */
