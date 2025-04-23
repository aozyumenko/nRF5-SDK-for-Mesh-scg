set(dds238_2_DEFINES
    -DCUSTOM_BOARD_INC=dds238_2_config
    -DCONFIG_GPIO_AS_PINRESET
    -DCONFIG_NFCT_PINS_AS_GPIOS)
set(dds238_2_INCLUDE_DIRS
    "${SDK_ROOT}/components/boards"
    "${CMAKE_SOURCE_DIR}/examples/smarthome/dds238_2/include")
