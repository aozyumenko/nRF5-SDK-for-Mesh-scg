#!/bin/sh

source ./board

ELF=${BOARD}_nrf52840_xxAA_s140_7.2.0.elf
PROGRAM=/home/scg/src/nRF5/nRF5-SDK-for-Mesh/build/examples/smarthome/${BOARD}/${ELF}

if [ ! -f $PROGRAM ]; then
    echo "Program ${ELF} not found."
    exit -1
fi


/opt/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-gdb -ex "target remote localhost:3333" ${PROGRAM}
