#!/bin/sh
if [ $# -eq 0 ]; then
    BIN="kernel.bin"
else
    BIN=$1
fi
USBBOOT_PATH="${HOME}/embedded/panda/omap4boot/out/panda"
sudo ${USBBOOT_PATH}/usbboot ${USBBOOT_PATH}/aboot.bin $BIN
