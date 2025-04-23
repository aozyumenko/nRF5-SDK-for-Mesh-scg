#!/bin/sh

# 
MBR=/home/scg/src/nRF5/nRF5_SDK_17.0.2_d674dde/components/softdevice/mbr/hex/mbr_nrf52_2.4.1_mbr.hex


sudo /opt/openocd/bin/openocd -s /opt/openocd/share/openocd/scripts -f interface/stlink.cfg -f target/nrf52.cfg -c init -c "reset halt" -c "program $MBR verify"  -c reset -c exit
