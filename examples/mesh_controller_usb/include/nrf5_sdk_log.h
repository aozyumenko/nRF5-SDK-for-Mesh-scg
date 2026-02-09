#ifndef NRF5_SDK_LOG_H
#define NRF5_SDK_LOG_H

#include "sdk_config.h"


#ifdef NRF_LOG_BACKEND_UART_ENABLED

    #include "nrf_log.h"
    #include "nrf_log_ctrl.h"
    #include "nrf_log_default_backends.h"

    #ifdef __LOG_INIT
        #undef __LOG_INIT
    #endif

    #ifdef __LOG
        #undef __LOG
    #endif

    #define __LOG_INIT(msk, level, callback) \
        do { \
            ret_code_t err_code = NRF_LOG_INIT(NULL); \
            APP_ERROR_CHECK(err_code); \
            NRF_LOG_DEFAULT_BACKENDS_INIT(); \
        } while(0)
    #define __LOG(source, level, ...) \
        NRF_LOG_INFO(__VA_ARGS__);

#else // NRF_LOG_BACKEND_UART_ENABLED

    #ifndef __LOG_INIT
        #define __LOG_INIT(msk, level, callback)
    #endif
    #ifndef __LOG
        #define __LOG(source, level, ...)
    #endif

#endif // NRF_LOG_BACKEND_UART_ENABLED


#endif // NRF5_SDK_LOG_H
