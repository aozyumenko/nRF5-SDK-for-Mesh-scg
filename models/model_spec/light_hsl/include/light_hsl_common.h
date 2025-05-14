/* Copyright (C)2021, St(u)dio of Computer Games
 * All rights reserved.
 */

#ifndef LIGHT_HSL_COMMON_H__
#define LIGHT_HSL_COMMON_H__

#include <stdint.h>


/**
 * Minimum and maximum allowed values of the Light HSL Hue Range state defined in
 * @tagMeshMdlSp section 6.1.4.3
 * */
#define LIGHT_HSL_RANGE_HUE_MIN (0)
#define LIGHT_HSL_RANGE_HUE_MAX (0xffff)

/**
 * Minimum and maximum allowed values of the Light HSL Saturation Range state defined in
 * @tagMeshMdlSp section 6.1.4.6
 * */
#define LIGHT_HSL_RANGE_SATURATION_MIN (0)
#define LIGHT_HSL_RANGE_SATURATION_MAX (0xffff)


/** Defines the default value for the Light HSL Hue state from @tagMeshMdlSp section 6.1.4.1 */
#ifndef LIGHT_HSL_DEFAULT_HUE
#define LIGHT_HSL_DEFAULT_HUE (0)
#endif

/** Defines the default value for the Light HSL Hue Default state from @tagMeshMdlSp section 6.1.4.2 */
#ifndef LIGHT_HSL_DEFAULT_HUE_DEFAULT
#define LIGHT_HSL_DEFAULT_HUE_DEFAULT (0)
#endif

/** Defines the application specific value for the minumum Light HSL Hue from
 * @tagMeshMdlSp section 6.1.4.3 */
#ifndef LIGHT_HSL_DEFAULT_RANGE_HUE_MIN
#define LIGHT_HSL_DEFAULT_RANGE_HUE_MIN (0)
#endif

/** Defines the application specific value for the maximum Light HSL Hue from
 * @tagMeshMdlSp section 6.1.4.3 */
#ifndef LIGHT_HSL_DEFAULT_RANGE_HUE_MAX
#define LIGHT_HSL_DEFAULT_RANGE_HUE_MAX (0xffff)
#endif

/** Defines the default value for the Light HSL Saturation state from @tagMeshMdlSp section 6.1.4.4 */
#ifndef LIGHT_HSL_DEFAULT_SATURATION
#define LIGHT_HSL_DEFAULT_SATURATION (0)
#endif

/** Defines the default value for the Light HSL Saturation Default state from @tagMeshMdlSp section 6.1.4.5 */
#ifndef LIGHT_HSL_DEFAULT_SATURATION_DEFAULT
#define LIGHT_HSL_DEFAULT_SATURATION_DEFAULT (0)
#endif

/** Defines the application specific value for the minumum Light HSL Saturation from
 * @tagMeshMdlSp section 6.1.4.6 */
#ifndef LIGHT_HSL_DEFAULT_RANGE_SATURATION_MIN
#define LIGHT_HSL_DEFAULT_RANGE_SATURATION_MIN (0)
#endif

/** Defines the application specific value for the maximum Light HSL Saturation from
 * @tagMeshMdlSp section 6.1.4.6 */
#ifndef LIGHT_HSL_DEFAULT_RANGE_SATURATION_MAX
#define LIGHT_HSL_DEFAULT_RANGE_SATURATION_MAX (0xffff)
#endif


/** Status values for @ref light_hsl_range_status_params_t  */
typedef enum {
    /** The provided range set values are valid */
    LIGHT_HSL_RANGE_STATUS_SUCCESS,
    /** The provided MINIMUM range set value is invalid */
    LIGHT_HSL_RANGE_STATUS_CANNOT_SET_RANGE_MIN,
    /** The provided MAXIMUM range set value is invalid */
    LIGHT_HSL_RANGE_STATUS_CANNOT_SET_RANGE_MAX
} light_hsl_range_status_t;



/**
 * Unpacked message structure typedefs are used for API interfaces and for implementing model code.
 * This helps to minimize code footprint.
 */

/** Message format for the HSL Set message. */
typedef struct {
    uint16_t lightness;                 /**< Value of the Lightness state */
    uint16_t hue;                       /**< Value of the Hue state */
    uint16_t saturation;                /**< Value of the Saturation state */
    uint8_t tid;                        /**< Transaction ID */
} light_hsl_set_params_t;


/** Message format for the HSL Hue Set message. */
typedef struct {
    uint16_t hue;                       /**< Value of the Hue state */
    uint8_t tid;                        /**< Transaction ID */
} light_hsl_hue_set_params_t;


/** Message format for the HSL Saturation Set message. */
typedef struct {
    uint16_t saturation;                /**< Value of the Saturation state */
    uint8_t tid;                        /**< Transaction ID */
} light_hsl_saturation_set_params_t;


/** Message format for the HSL Default Set message. */
typedef struct {
    uint16_t lightness;                 /**< The default value of the Lightness state */
    uint16_t hue;                       /**< The default value of the Hue state */
    uint16_t saturation;                /**< The default value of the Saturation state */
} light_hsl_default_set_params_t;


/** Message format for the HSL Range Set message. */
typedef struct {
    uint16_t hue_range_min;             /**< Value of the Hue range min state */
    uint16_t hue_range_max;             /**< Value of the Hue range max state */
    uint16_t saturation_range_min;      /**< Value of the Hue range min state */
    uint16_t saturation_range_max;      /**< Value of the Hue range max state */
} light_hsl_range_set_params_t;



/** Message format for the Light HSL hue/saturation delta set "message". */
/** There is no official delta set message for Light HSL server. */
/** This structure supports internal messages, specifically for server<->app interaction. */
typedef struct {
    int32_t delta;                      /**< Value of the Lightness delta value */
    uint8_t tid;                        /**< Transaction ID */
} light_hsl_delta_set_params_t;

/** Message format for the Light HSL hue/saturation move set "message". */
/** There is no official move set message for Light HSL server. */
/** This structure supports internal messages, specifically for server<->app interaction. */
typedef struct {
    int16_t delta;                        /**< Value of the Lightness move value */
    uint8_t tid;                          /**< Transaction ID */
} light_hsl_move_set_params_t;



/** Parameters for the Light HSL Status message. */
typedef struct {
    uint16_t present_lightness;         /**< The present value of the Lightness state */
    uint16_t present_hue;               /**< The present value of the Hue state */
    uint16_t present_saturation;        /**< The present value of the Saturation state */
    uint16_t target_lightness;          /**< The target value of the Lightness state */
    uint16_t target_hue;                /**< The target value of the Hue state */
    uint16_t target_saturation;         /**< The target value of the Saturation state */
    uint32_t remaining_time_ms;         /**< Remaining time value in milliseconds */
} light_hsl_status_params_t;


/** Parameters for the Light HSL Hue Status message. */
typedef struct {
    uint16_t present_hue;               /**< The present value of the Hue state */
    uint16_t target_hue;                /**< The target value of the Hue state (optional) */
    uint32_t remaining_time_ms;         /**< Remaining time value in milliseconds */
} light_hsl_hue_status_params_t;


/** Parameters for the Light HSL Saturation Status message. */
typedef struct {
    uint16_t present_saturation;        /**< The present value of the Saturation state */
    uint16_t target_saturation;         /**< The target value of the Saturation state (optional) */
    uint32_t remaining_time_ms;         /**< Remaining time value in milliseconds */
} light_hsl_saturation_status_params_t;


/** Message format for the HSL Default Status message. */
typedef struct {
    uint16_t lightness;                 /**< The default value of the Lightness state */
    uint16_t hue;                       /**< The default value of the Hue state */
    uint16_t saturation;                /**< The default value of the Saturation state */
} light_hsl_default_status_params_t;


/** Message format for the HSL Range Status message. */
typedef struct {
    uint8_t status_code;                /**< Status code for the requesting message */
    uint16_t hue_range_min;             /**< Value of the Hue range min state */
    uint16_t hue_range_max;             /**< Value of the Hue range max state */
    uint16_t saturation_range_min;      /**< Value of the Hue range min state */
    uint16_t saturation_range_max;      /**< Value of the Hue range max state */
} light_hsl_range_status_params_t;


/** Storage format for model states while booting - the stored values will be read and passed to
 * the model to do any state binding and set the current states based on these stored values. */
typedef struct {
    uint8_t onpowerup;
    uint16_t hue;
    uint16_t default_hue;
    uint16_t saturation;
    uint16_t default_saturation;
    light_hsl_range_set_params_t range;
} light_hsl_saved_values_t;



#endif /* LIGHT_HSL_COMMON_H__ */
