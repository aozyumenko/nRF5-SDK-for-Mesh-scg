#!/bin/sh

SOFTDEVICE=/home/scg/src/nRF5/nRF5-SDK-for-Mesh/bin/softdevice/s140_nrf52_7.2.0_softdevice.hex

while : ; do
    sudo /opt/openocd/bin/openocd -s /opt/openocd/share/openocd/scripts -f interface/stlink.cfg -f target/nrf52.cfg -c init -c "reset halt" -c "program $SOFTDEVICE verify" -c "reset" -c exit
    [[ "$?" == "0" ]] && break
done
