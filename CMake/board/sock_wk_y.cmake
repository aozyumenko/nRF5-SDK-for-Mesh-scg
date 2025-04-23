set(sock_wk_y_DEFINES
    -DCUSTOM_BOARD_INC=sock_wk_y_config
    -DCONFIG_GPIO_AS_PINRESET
    -DCONFIG_NFCT_PINS_AS_GPIOS)
set(sock_wk_y_INCLUDE_DIRS
    "${SDK_ROOT}/components/boards"
    "${CMAKE_SOURCE_DIR}/examples/smarthome/sock_wk_y/include")
