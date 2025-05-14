#ifndef LIGHT_HSL_UTILS_H__
#define LIGHT_HSL_UTILS_H__
#include <stdint.h>

#include "nordic_common.h"



//#define T32_SCALE_FACTOR (65535LL)

/* macro for performing scaling down */
//#define SCALE_DOWN(_input, _scale_factor)   ((_input)/(_scale_factor))

/* macro for performing scaling up */
//#define SCALE_UP(_input, _scale_factor)     ((_input)*(_scale_factor))



static inline uint16_t light_hsl_utils_param_range_restrict(uint16_t param,
                                                            uint16_t param_min,
                                                            uint16_t param_max)
{
    /* 6.1.4.1.3 Binding with the HSL Hue Range state */
    /* 6.1.4.4.3 Binding with the HSL Saturation Range state */
    return MAX(param_min, MIN(param_max, param));
}


/* convert Light HSL hue/saturation state to Generic Level */
static inline int16_t light_hsl_utils_param_to_generic_level(uint16_t param)
{
    return param - GENERIC_LEVEL_MIN;
}

/* convert Generic Level state to Light HSL hue/saturation */
static inline uint16_t light_hsl_utils_generic_level_to_param(int16_t level)
{
    return level + GENERIC_LEVEL_MIN;
}


/* converts Generic Level state to hue/saturation value */
//static inline uint16_t light_hsl_utils_level_to_value(int16_t level, uint16_t value_min,
//                                                      uint16_t value_max)
//{
//    return value_min + SCALE_DOWN((uint32_t)(level + 32768) * (uint32_t)(value_max - value_min), T32_SCALE_FACTOR);
//}

/* converts hue/saturation value to Generic Level state */
//static inline int16_t light_hsl_utils_value_to_level(uint16_t value, uint32_t value_min,
//                                                     uint32_t value_max)
//{
//    if (value_max == value_min) {
//        return 0;
//    }

//    return ((SCALE_UP((uint64_t)(value - value_min), T32_SCALE_FACTOR) / (value_max - value_min)) - 32768);
//}




#endif /* LIGHT_HSL_UTILS_H__ */
