CROSS_COMPILE ?= arm-none-eabi-
MAKEFLAGS += -rR --no-print-directory
INC = -Iinclude
LDFLAGS = -T kernel/linker.ld -nostdlib -nostdinc -nodefaultlibs -nostartfiles \
	   -fno-builtin
CFLAGS =  -march=armv7-r -ggdb -Wall -Wextra -Wformat-security -Wshadow \
	   -Wunreachable-code -Wpointer-arith -O2 -std=gnu99 -nostdlib \
	   -nostdinc -nodefaultlibs -nostartfiles -fno-builtin $(INC)
ASFLAGS = -march=armv7-r -ggdb -nostdlib -nostdinc -nodefaultlibs \
	   -nostartfiles -fno-builtin $(INC)
LIBGCC = -L $(shell dirname $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)) -lgcc
CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
NM = $(CROSS_COMPILE)nm

objs = kernel/start.o kernel/kmain.o kernel/rs232.o kernel/print.o \
	kernel/debug.o kernel/interrupts.o

all: kernel.elf kernel.bin kernel.syms uImage usbbootImage

kernel.elf: $(objs) kernel/linker.ld
	@echo -e '  LD\t'$@
	@$(CC) $(LDFLAGS) -o $@ $(objs) $(LIBGCC)

kernel.bin: kernel.elf
	@$(OBJCOPY) $< -O binary $@

kernel.syms: kernel.elf
	@$(NM) $< > $@

uImage: kernel.bin
	@mkimage -A arm -T kernel -C none -a 0x80008000 -e 0x80008000 -n Voron -d $< $@

usbbootImage.elf: usbboot/usbboot.o usbboot/usbboot.ld usbboot/kernel_image.ld
	@echo -e '  LD\t'$@
	@$(CC) -T usbboot/usbboot.ld -nostdlib -nostdinc -nodefaultlibs -nostartfiles \
	-fno-builtin -o $@ $<

usbbootImage: usbbootImage.elf
	@echo "Create usbbootImage"
	@echo -e "Entry Point: 0x82000000"
	@$(OBJCOPY) $< -O binary $@

usbboot/kernel_image.ld: kernel.bin
	@hexdump -v -e '"BYTE(0x" 1/1 "%02X" ")\n"' $< > $@

%.o: %.c
	@echo -e '  CC\t'$<
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	@echo -e '  AS\t'$<
	@$(CC) $(ASFLAGS) -c -o $@ $<

clean:
	@rm -f $(objs) kernel.elf kernel.bin kernel.syms uImage usbbootImage \
	usbbootImage.elf usbboot/kernel_image.ld usbboot/usbboot.o
