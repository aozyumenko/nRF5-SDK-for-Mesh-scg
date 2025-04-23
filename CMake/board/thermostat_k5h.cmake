set(thermostat_k5h_DEFINES
    -DCUSTOM_BOARD_INC=thermostat_k5h_config
    -DCONFIG_GPIO_AS_PINRESET
    -DCONFIG_NFCT_PINS_AS_GPIOS)
set(thermostat_k5h_INCLUDE_DIRS
    "${SDK_ROOT}/components/boards"
    "${CMAKE_SOURCE_DIR}/examples/smarthome/thermostat_k5h/include")
