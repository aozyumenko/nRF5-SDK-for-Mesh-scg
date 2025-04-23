#!/bin/sh

source ./board

FW=${BOARD}_nrf52840_xxAA_s140_7.2.0.hex
PROGRAM=/home/scg/src/nRF5/nRF5-SDK-for-Mesh/build/examples/smarthome/${BOARD}/${FW}

if [ ! -f $PROGRAM ]; then
    echo "Firmware file ${FW} not found."
    exit -1
fi

while : ; do
    sudo /opt/openocd/bin/openocd -s /opt/openocd/share/openocd/scripts -f interface/stlink.cfg -f target/nrf52.cfg -c init -c "reset halt" -c "program $PROGRAM verify"  -c reset -c exit
    [[ "$?" == "0" ]] && break
done
