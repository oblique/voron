Voron is an experimental ARM based operating system for PandaBoard.

## Features (until now)

* Uniprocessor support only (for now)
* Interrupt handling
* Memory management
* Simple kernel heap allocator (I will implement SLAB in the future)
* Round-robin scheduler
* Kernel threading
* Locking primitives
* Sleep/wakeup mechanism for processes


## Compile

If you are not using an ARM architecture processor then you will need
an ARM architecture toolchain, you can use `arm-none-eabi` from
Sourcery CodeBench Lite. You will also need `uboot-mkimage`
package. To compile it, run

    make

If you have another toolchain, for example `arm-unknown-eabi`, run

    make CROSS_COMPILE=arm-unknown-eabi-

If you are using an ARM architecture processor, run

    make CROSS_COMPILE=


## Boot via SD card

First you must create a Fat32 or Fat16 partition in your SD card and
mark it as boot. Then run (use `CROSS_COMPILE` if needed)

    make bootloader

After this you must copy `uImage`, `boot/boot.scr`,
`boot/u-boot-linaro-stable/u-boot.bin` and `boot/u-boot-linaro-stable/MLO`
in your SD card.


## Boot via USB cable

Plug a USB cable at your PandaBoard and run

    ./scripts/panda_usbboot.sh


## Output

I only use RS-232 Serial port for text output, so you will need `minicom`.
