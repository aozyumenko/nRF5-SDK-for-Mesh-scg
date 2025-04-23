set(thermostat_brt100_DEFINES
    -DCUSTOM_BOARD_INC=thermostat_brt100_config
    -DCONFIG_GPIO_AS_PINRESET
    -DCONFIG_NFCT_PINS_AS_GPIOS)
set(thermostat_brt100_INCLUDE_DIRS
    "${SDK_ROOT}/components/boards"
    "${CMAKE_SOURCE_DIR}/examples/smarthome/thermostat_brt100/include")
