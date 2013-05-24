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
    OMAP4BOOT_PATH="${PWD}/boot/omap4boot/out/panda"
    if [ ! -f "${OMAP4BOOT_PATH}/usbboot" -o ! -f "${OMAP4BOOT_PATH}/aboot.bin" ]; then
        git submodule init || exit 1
        git submodule update || exit 1
        cd boot/omap4boot || exit 1
        git reset --hard
        patch -p1 -i ../omap4boot_remove_signature_check.patch
        if [ -z "$TOOLCHAIN" ]; then
            make TOOLCHAIN=arm-none-eabi-
        else
            make TOOLCHAIN="$TOOLCHAIN"
        fi
    fi
fi

sudo "${OMAP4BOOT_PATH}/usbboot" "${OMAP4BOOT_PATH}/aboot.bin" "${BIN}"
