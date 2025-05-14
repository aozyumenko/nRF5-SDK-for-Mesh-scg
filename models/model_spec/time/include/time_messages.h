/* (C) 2024, Alexander Ozumenko
 * St(u)dio of Computer Games (https://stdio.ru/)
 */

#ifndef TIME_MESSAGES_H__
#define TIME_MESSAGES_H__

#include <stdint.h>



/** Time model message opcodes. */
typedef enum {
    TIME_OPCODE_TIME_GET = 0x8237,
    TIME_OPCODE_TIME_SET = 0x5c,
    TIME_OPCODE_TIME_STATUS = 0x5d,
    TIME_OPCODE_TIME_ROLE_GET = 0x8238,
    TIME_OPCODE_TIME_ROLE_SET = 0x8239,
    TIME_OPCODE_TIME_ROLE_STATUS = 0x823a,
    TIME_OPCODE_TIME_ZONE_GET = 0x823b,
    TIME_OPCODE_TIME_ZONE_SET = 0x823c,
    TIME_OPCODE_TIME_ZONE_STATUS = 0x823d,
    TIME_OPCODE_TAI_UTC_DELTA_GET = 0x823e,
    TIME_OPCODE_TAI_UTC_DELTA_SET = 0x823f,
    TIME_OPCODE_TAI_UTC_DELTA_STATUS = 0x8240
} time_opcode_t;

/** Packed message structure typedefs are used for packing and unpacking byte stream. */

/** Message format for the Time Set. */
typedef struct __attribute((packed)) {
    uint8_t TAI_seconds[5];
    uint8_t subsecond;
    uint8_t uncertainty;
    uint16_t time_authority     : 1;
    uint16_t tai_utc_delta      : 15;
    uint8_t time_zone_offset;
} time_set_msg_pkt_t;

/** Message format for the Time Status. */
typedef struct __attribute((packed)) {
    uint8_t TAI_seconds[5];
    uint8_t subsecond;
    uint8_t uncertainty;
    uint16_t time_authority     : 1;
    uint16_t tai_utc_delta      : 15;
    uint8_t time_zone_offset;
} time_status_msg_pkt_t;

/** Message format for the Time Zone Set. */
typedef struct __attribute((packed)) {
    uint8_t time_zone_offset_new;
    uint8_t tai_zone_change[5];
} time_zone_set_msg_pkt_t;

/** Message format for the Time Zone Status. */
typedef struct __attribute((packed)) {
    uint8_t time_zone_offset_current;
    uint8_t time_zone_offset_new;
    uint8_t tai_zone_change[5];
} time_zone_status_msg_pkt_t;

/** Message format for the TAI-UTC Delta Set. */
typedef struct __attribute((packed)) {
    uint16_t tai_utc_delta_new  : 15;
    uint16_t padding            : 1;
    uint8_t tai_delta_change[5];
} TAI_UTC_delta_set_msg_pkt_t;

/** Message format for the TAI-UTC Delta Status. */
typedef struct __attribute((packed)) {
    uint16_t tai_utc_delta_current      : 15;
    uint16_t padding1                   : 1;
    uint16_t tai_utc_delta_new          : 15;
    uint16_t padding2                   : 1;
    uint8_t tai_delta_change[5];
} TAI_UTC_delta_status_msg_pkt_t;

/** Message format for the Time Role Set. */
typedef struct __attribute((packed)) {
    uint8_t time_role;
} time_role_set_msg_pkt_t;

/** Message format for the Time Role Status. */
typedef struct __attribute((packed)) {
    uint8_t time_role;
} time_role_status_msg_pkt_t;



#endif /* TIME_MESSAGES_H__ */
