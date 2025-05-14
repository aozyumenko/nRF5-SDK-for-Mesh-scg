#ifndef LIGHT_HSL_MESSAGES_H__
#define LIGHT_HSL_MESSAGES_H__

#include <stdint.h>


/** shortest allowed length for the Light HSL Set message */
#define LIGHT_HSL_SET_MINLEN 7
/** longest allowed length for the Light HSL Set message */
#define LIGHT_HSL_SET_MAXLEN 9

/** shortest allowed length for the Light HSL Hue Set message */
#define LIGHT_HSL_HUE_SET_MINLEN 3
/** songest allowed length for the Light HSL Hue Set message */
#define LIGHT_HSL_HUE_SET_MAXLEN 5

/** shortest allowed length for the Light HSL Saturation Set message */
#define LIGHT_HSL_SATURATION_SET_MINLEN 3
/** songest allowed length for the Light HSL Saturation Set message */
#define LIGHT_HSL_SATURATION_SET_MAXLEN 5

/** Shortest allowed length for the Status message. */
#define LIGHT_HSL_STATUS_MINLEN 6
/** Longest allowed length for the Status message. */
#define LIGHT_HSL_STATUS_MAXLEN 7

/** Shortest allowed length for the Target Status message. */
#define LIGHT_HSL_TARGET_STATUS_MINLEN 6
/** Longest allowed length for the Target Status message. */
#define LIGHT_HSL_TARGET_STATUS_MAXLEN 7

/** Length for the Light HSL Default Set message. */
#define LIGHT_HSL_DEFAULT_SET_LEN 6

/** Length for the Light HSL Default Status message. */
#define LIGHT_HSL_DEFAULT_STATUS_LEN 6

/** Shortest allowed length for the Hue Status message. */
#define LIGHT_HSL_HUE_STATUS_MINLEN 2
/** Longest allowed length for the Hue Status message. */
#define LIGHT_HSL_HUE_STATUS_MAXLEN 5

/** Shortest allowed length for the Saturation Status message. */
#define LIGHT_HSL_SATURATION_STATUS_MINLEN 2
/** Longest allowed length for the Saturation Status message. */
#define LIGHT_HSL_SATURATION_STATUS_MAXLEN 5

/** Length for the Light HSL Range Status message. */
#define LIGHT_HSL_RANGE_STATUS_LEN 9

/** Length for the Light CTL Temperature Range Set message. */
#define LIGHT_HSL_RANGE_SET_LEN 8



/** Light HSL model message opcodes. */
typedef enum {
    LIGHT_HSL_OPCODE_GET = 0x826d,
    LIGHT_HSL_OPCODE_HUE_GET = 0x826e,
    LIGHT_HSL_OPCODE_HUE_SET = 0x826f,
    LIGHT_HSL_OPCODE_HUE_SET_UNACKNOWLEDGED = 0x8270,
    LIGHT_HSL_OPCODE_HUE_STATUS = 0x8271,
    LIGHT_HSL_OPCODE_SATURATION_GET = 0x8272,
    LIGHT_HSL_OPCODE_SATURATION_SET = 0x8273,
    LIGHT_HSL_OPCODE_SATURATION_SET_UNACKNOWLEDGED = 0x8274,
    LIGHT_HSL_OPCODE_SATURATION_STATUS = 0x8275,
    LIGHT_HSL_OPCODE_SET = 0x8276,
    LIGHT_HSL_OPCODE_SET_UNACKNOWLEDGED = 0x8277,
    LIGHT_HSL_OPCODE_STATUS = 0x8278,
    LIGHT_HSL_OPCODE_TARGET_GET = 0x8279,
    LIGHT_HSL_OPCODE_TARGET_STATUS = 0x827a,
    LIGHT_HSL_OPCODE_DEFAULT_GET = 0x827b,
    LIGHT_HSL_OPCODE_DEFAULT_STATUS = 0x827c,
    LIGHT_HSL_OPCODE_RANGE_GET = 0x827d,
    LIGHT_HSL_OPCODE_RANGE_STATUS = 0x827e,
    LIGHT_HSL_OPCODE_DEFAULT_SET = 0x827f,
    LIGHT_HSL_OPCODE_DEFAULT_SET_UNACKNOWLEDGED = 0x8280,
    LIGHT_HSL_OPCODE_RANGE_SET = 0x8281,
    LIGHT_HSL_OPCODE_RANGE_SET_UNACKNOWLEDGED = 0x8282
} light_hsl_opcode_t;

/** Packed message structure typedefs are used for packing and unpacking byte stream. */

/** Message format for the Light HSL Set. */
typedef struct __attribute((packed)) {
    uint16_t lightness;                                     /**< Lightness state */
    uint16_t hue;                                           /**< Hue state */
    uint16_t saturation;                                    /**< Saturation state */
    uint8_t tid;                                            /**< Transaction number for application */
    uint8_t transition_time;                                /**< Encoded transition time value */
    uint8_t delay;                                          /**< Encoded message execution delay in 5 millisecond steps */
} light_hsl_set_msg_pkt_t;


/** Message format for the Light HSL Status. */
typedef struct __attribute((packed)) {
    uint16_t lightness;                                     /**< Lightness present value */
    uint16_t hue;                                           /**< Hue present value */
    uint16_t saturation;                                    /**< Saturation present value */
    uint8_t remaining_time;                                 /**< Encoded remaining time */
} light_hsl_status_msg_pkt_t;


/** Message format for the Light HSL Target Status. */
typedef struct __attribute((packed)) {
    uint16_t lightness;                                     /**< Lightness target value */
    uint16_t hue;                                           /**< Hue target value */
    uint16_t saturation;                                    /**< Saturation target value */
    uint8_t remaining_time;                                 /**< Encoded remaining time */
} light_hsl_target_status_msg_pkt_t;


/** Message format for the Light HSL Hue Set. */
typedef struct __attribute((packed)) {
    uint16_t hue;                                           /**< Hue target value */
    uint8_t tid;                                            /**< Transaction number for application */
    uint8_t transition_time;                                /**< Encoded transition time value */
    uint8_t delay;                                          /**< Encoded message execution delay in 5 millisecond steps */
} light_hsl_hue_set_msg_pkt_t;


/** Message format for the Light HSL Hue Status. */
typedef struct __attribute((packed)) {
    uint16_t present_hue;                               /**< Hue present value. */
    uint16_t target_hue;                                /**< Hue target value */
    uint8_t remaining_time;                             /**< Encoded remaining time */
} light_hsl_hue_status_msg_pkt_t;


/** Message format for the Light HSL Saturation Set. */
typedef struct __attribute((packed)) {
    uint16_t saturation;                                    /**< Saturation target value */
    uint8_t tid;                                            /**< Transaction number for application */
    uint8_t transition_time;                                /**< Encoded transition time value */
    uint8_t delay;                                          /**< Encoded message execution delay in 5 millisecond steps */
} light_hsl_saturation_set_msg_pkt_t;


/** Message format for the Light HSL Saturation Status. */
typedef struct __attribute((packed)) {
    uint16_t present_saturation;                        /**< Saturation present value. */
    uint16_t target_saturation;                         /**< Saturation target value */
    uint8_t remaining_time;                             /**< Encoded remaining time */
} light_hsl_saturation_status_msg_pkt_t;


/** Message format for the Light HSL Default Set. */
typedef struct __attribute((packed)) {
    uint16_t lightness;                                     /**< Lightness default state */
    uint16_t hue;                                           /**< Hue default state */
    uint16_t saturation;                                    /**< Saturation default state */
} light_hsl_default_set_msg_pkt_t;


/** Message format for the Light HSL Default Status. */
typedef struct __attribute((packed)) {
    uint16_t lightness;                                     /**< Lightness default state */
    uint16_t hue;                                           /**< Hue default state */
    uint16_t saturation;                                    /**< Saturation default state */
} light_hsl_default_status_msg_pkt_t;


/** Message format for the Light HSL Range Set. */
typedef struct __attribute((packed)) {
    uint16_t hue_range_min;                                 /**< Hue Range MIN value */
    uint16_t hue_range_max;                                 /**< Hue Range MAX value */
    uint16_t saturation_range_min;                          /**< Saturation Range MIN vaue */
    uint16_t saturation_range_max;                          /**< Saturation Range MAX vaue */
} light_hsl_range_set_msg_pkt_t;


/** Message format for the Light HSL Range Status. */
typedef struct __attribute((packed)) {
    uint8_t status_code;                                    /**< Status code for the requesting message */
    uint16_t hue_range_min;                                 /**< Hue Range MIN value */
    uint16_t hue_range_max;                                 /**< Hue Range MAX value */
    uint16_t saturation_range_min;                          /**< Saturation Range MIN vaue */
    uint16_t saturation_range_max;                          /**< Saturation Range MAX vaue */
} light_hsl_range_status_msg_pkt_t;



#endif /* LIGHT_HSL_MESSAGES_H__ */
