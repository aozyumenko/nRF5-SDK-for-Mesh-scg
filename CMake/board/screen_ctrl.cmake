set(screen_ctrl_DEFINES
    -DCUSTOM_BOARD_INC=screen_ctrl_config
    -DCONFIG_GPIO_AS_PINRESET
    -DCONFIG_NFCT_PINS_AS_GPIOS)
set(screen_ctrl_INCLUDE_DIRS
    "${SDK_ROOT}/components/boards"
    "${CMAKE_SOURCE_DIR}/examples/smarthome/screen_ctrl/include")
