#!/bin/sh

sudo /opt/openocd/bin/openocd -s /opt/openocd/share/openocd/scripts -f interface/stlink.cfg -f target/nrf52.cfg -c init
