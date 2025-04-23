set(light_rgbw_1_DEFINES
    -DCUSTOM_BOARD_INC=light_rgbw_1_config
    -DCONFIG_GPIO_AS_PINRESET)
set(light_rgbw_1_INCLUDE_DIRS
    "${SDK_ROOT}/components/boards"
    "${CMAKE_SOURCE_DIR}/examples/smarthome/light_rgbw_1/include")
