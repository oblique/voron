#!/bin/sh

if [ $# -gt 0 ]; then
    BIN=$(realpath "$1")
fi

[ $(basename ${PWD}) = "scripts" ] && cd ..

if [ -z "$BIN" ]; then
    BIN="${PWD}/kernel.bin"
fi

if [ ! -f "$BIN" ]; then
    echo "run \`make' first."
    exit 1
fi

if [ -z "$OMAP4BOOT_PATH" ]; then
    if [ ! -d "boot/omap4boot" ]; then
	cd boot
	git clone git://github.com/swetland/omap4boot.git
	cd omap4boot
	patch -p1 -i ../omap4boot_remove_signature_check.patch
	cd ../..
    fi

    OMAP4BOOT_PATH="${PWD}/boot/omap4boot/out/panda"

    if [ ! -f "${OMAP4BOOT_PATH}/usbboot" -o ! -f "${OMAP4BOOT_PATH}/aboot.bin" ]; then
        cd boot/omap4boot
        if [ -z "$TOOLCHAIN" ]; then
            make TOOLCHAIN=arm-none-eabi-
        else
            make TOOLCHAIN="$TOOLCHAIN"
        fi
    fi
fi

sudo "${OMAP4BOOT_PATH}/usbboot" "${OMAP4BOOT_PATH}/aboot.bin" "${BIN}"
