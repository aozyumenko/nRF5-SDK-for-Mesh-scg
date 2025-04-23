set(sock_wwk_dey_DEFINES
    -DCUSTOM_BOARD_INC=sock_wwk_dey_config
    -DCONFIG_GPIO_AS_PINRESET
    -DCONFIG_NFCT_PINS_AS_GPIOS)
set(sock_wwk_dey_INCLUDE_DIRS
    "${SDK_ROOT}/components/boards"
    "${CMAKE_SOURCE_DIR}/examples/smarthome/sock_wwk_dey/include")
