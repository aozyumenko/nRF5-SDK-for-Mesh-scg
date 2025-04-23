#!/bin/sh

source ./board

make clean
cmake -G "Unix Makefiles" -DSDK_ROOT=/home/scg/src/nRF5/nRF5_SDK_17.0.2_d674dde -DPLATFORM=nrf52840_xxAA -DBOARD=$BOARD ..
