set(target "mesh_controller_usb_${PLATFORM}_${SOFTDEVICE}")

add_executable(${target}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/serial.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/serial_cmd_handler.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/serial_bearer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ad_data_proxy.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ctrl_led.c"
    "${CMAKE_SOURCE_DIR}/mesh/stack/src/mesh_stack.c"
    "${CMAKE_SOURCE_DIR}/examples/common/src/mesh_app_utils.c"
    "${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_gpiote.c"
    ${BLE_SOFTDEVICE_SUPPORT_SOURCE_FILES}
    ${PROV_COMMON_SOURCE_FILES}
    ${ACCESS_SOURCE_FILES}
    ${CONFIG_SERVER_SOURCE_FILES}
    ${HEALTH_SERVER_SOURCE_FILES}
    ${MESH_SERIAL_PROXY_SOURCE_FILES}
    ${WEAK_SOURCE_FILES}
    ${MESH_CORE_SOURCE_FILES}
    ${MESH_BEARER_SOURCE_FILES}
    ${MESH_APP_TIMER_SOURCE_FILES}
    ${${PLATFORM}_SOURCE_FILES}
    ${${nRF5_SDK_VERSION}_SOURCE_FILES}
    # SDK start
    "${SDK_ROOT}/components/libraries/usbd/app_usbd.c"
    "${SDK_ROOT}/components/libraries/usbd/class/cdc/acm/app_usbd_cdc_acm.c"
    "${SDK_ROOT}/components/libraries/usbd/app_usbd_core.c"
    "${SDK_ROOT}/components/libraries/usbd/app_usbd_serial_num.c"
    "${SDK_ROOT}/components/libraries/usbd/app_usbd_string_desc.c"
    "${SDK_ROOT}/components/libraries/atomic_fifo/nrf_atfifo.c"
    "${SDK_ROOT}/components/libraries/atomic/nrf_atomic.c"
    "${SDK_ROOT}/external/utf_converter/utf.c"
    "${SDK_ROOT}/integration/nrfx/legacy/nrf_drv_clock.c"
    "${SDK_ROOT}/integration/nrfx/legacy/nrf_drv_power.c"
    "${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_clock.c"
    "${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_power.c"
    "${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_usbd.c"
    "${SDK_ROOT}/modules/nrfx/soc/nrfx_atomic.c"
    # SDK end
    # SDK logs start
    "${SDK_ROOT}/components/libraries/log/src/nrf_log_backend_serial.c"
    "${SDK_ROOT}/components/libraries/log/src/nrf_log_backend_uart.c"
    "${SDK_ROOT}/components/libraries/log/src/nrf_log_default_backends.c"
    "${SDK_ROOT}/components/libraries/log/src/nrf_log_frontend.c"
    "${SDK_ROOT}/components/libraries/log/src/nrf_log_str_formatter.c"
    "${SDK_ROOT}/components/libraries/memobj/nrf_memobj.c"
    "${SDK_ROOT}/components/libraries/balloc/nrf_balloc.c"
    "${SDK_ROOT}/external/fprintf/nrf_fprintf.c"
    "${SDK_ROOT}/external/fprintf/nrf_fprintf_format.c"
    "${SDK_ROOT}/components/libraries/ringbuf/nrf_ringbuf.c"
    "${SDK_ROOT}/integration/nrfx/legacy/nrf_drv_uart.c"
    "${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_uart.c"
    "${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_uarte.c"
    "${SDK_ROOT}/components/libraries/strerror/nrf_strerror.c"
    "${SDK_ROOT}/modules/nrfx/drivers/src/prs/nrfx_prs.c"
    "${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_gpiote.c"
    # SDK logs end
)

target_include_directories(${target} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${MBTLE_SOURCE_DIR}/examples"
    "${CMAKE_SOURCE_DIR}/examples/common/include"
    ${BLE_SOFTDEVICE_SUPPORT_INCLUDE_DIRS}
    ${CONFIG_SERVER_INCLUDE_DIRS}
    ${HEALTH_SERVER_INCLUDE_DIRS}
    ${MESH_INCLUDE_DIRS}
    ${COMMON_EXAMPLES_INCLUDE_DIRS}
    ${${SOFTDEVICE}_INCLUDE_DIRS}
    ${${PLATFORM}_INCLUDE_DIRS}
    ${${BOARD}_INCLUDE_DIRS}
    ${MESH_SERIAL_PROXY_INCLUDE_DIRS}
    ${${nRF5_SDK_VERSION}_INCLUDE_DIRS}
    # SDK start
    "${SDK_ROOT}/components/libraries/usbd"
    "${SDK_ROOT}/components/libraries/usbd/class/cdc"
    "${SDK_ROOT}/components/libraries/usbd/class/cdc/acm"
    "${SDK_ROOT}/components/libraries/atomic_fifo"
    "${SDK_ROOT}/components/libraries/atomic"
    "${SDK_ROOT}/integration/nrfx/legacy"
    "${SDK_ROOT}/modules/nrfx/drivers/include"
    "${SDK_ROOT}/modules/nrfx/soc"
    # SDK end
    # SDK logs start
    "${SDK_ROOT}/components/libraries/usbd/class/cdc/acm"
    "${SDK_ROOT}/components/libraries/memobj"
    "${SDK_ROOT}/components/libraries/balloc"
    "${SDK_ROOT}/external/fprintf"
    "${SDK_ROOT}/components/libraries/ringbuf"
    "${SDK_ROOT}/external/utf_converter"
    # SDK logs end
)

set_target_link_options(${target}
    ${CMAKE_CURRENT_SOURCE_DIR}/linker/${PLATFORM}_${SOFTDEVICE})

target_compile_options(${target} PUBLIC
    ${${ARCH}_DEFINES})

target_compile_definitions(${target} PUBLIC -DPERSISTENT_STORAGE=0
    ${USER_DEFINITIONS}
    -DUSE_APP_CONFIG
    -DCONFIG_APP_IN_CORE
    ${${PLATFORM}_DEFINES}
    ${${SOFTDEVICE}_DEFINES}
    ${${BOARD}_DEFINES}
    -DSERIAL_SLIP_ENCODING)

target_link_libraries(${target}
    uECC_${PLATFORM}
    rtt_${PLATFORM})

create_hex(${target})
add_flash_target(${target})

get_property(target_include_dirs TARGET ${target} PROPERTY INCLUDE_DIRECTORIES)
add_pc_lint(${target}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
    "${target_include_dirs}"
    "${${PLATFORM}_DEFINES};${${SOFTDEVICE}_DEFINES};${${BOARD}_DEFINES}")
add_ses_project(${target})
