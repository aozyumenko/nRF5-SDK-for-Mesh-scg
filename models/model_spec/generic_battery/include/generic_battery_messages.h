/* (C) 2024 Alexander Ozumenko
 * St(u)dio of Computer Games (https://stdio.ru/)
 */

#ifndef GENERIC_BATTERY_MESSAGES_H__
#define GENERIC_BATTERY_MESSAGES_H__

#include <stdint.h>



/** Length for the Status message. */
#define GENERIC_BATTERY_STATUS_LEN      (8)



/** Generic Battery model message opcodes. */
typedef enum
{
    GENERIC_BATTERY_OPCODE_GET = 0x8223,
    GENERIC_BATTERY_OPCODE_STATUS = 0x8224
} generic_battery_opcode_t;

/** Packed message structure typedefs are used for packing and unpacking byte stream. */

/** Message format for the Generic Battery Status message. */
typedef struct __attribute((packed))
{
    uint8_t battery_level;                                  /**< Value of the Generic Battery Level state */
    uint8_t discharge_time[3];                              /**< Value of the Generic Battery Time to Discharge state */
    uint8_t charge_time[3];                                 /**< Value of the Generic Battery Time to Charge state */
    uint8_t flags_presence      : 2;                        /**< Generic Battery Flags Presence */
    uint8_t flags_indicator     : 2;                        /**< Generic Battery Flags Indicator */
    uint8_t flags_charging      : 2;                        /**< Generic Battery Flags Charging */
    uint8_t flags_service       : 2;                        /**< Generic Battery Flags Serviceability */
} generic_battery_status_msg_pkt_t;



#endif /* GENERIC_BATTERY_MESSAGES_H__ */
