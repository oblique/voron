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
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
NM = $(CROSS_COMPILE)nm

KERNEL_SRCS_C = $(wildcard kernel/*.c)
KERNEL_SRCS_S = $(wildcard kernel/*.S)
KERNEL_SRCS = $(KERNEL_SRCS_C) $(KERNEL_SRCS_S)
KERNEL_OBJS = $(KERNEL_SRCS_C:%.c=%.o) $(KERNEL_SRCS_S:%.S=%.o)
HDRS = $(wildcard include/*.h)

all: uImage usbbootImage kernel.syms

kernel.elf: $(KERNEL_OBJS) kernel/linker.ld
	@echo -e "  LD\t$@"
	@$(CC) $(LDFLAGS) -o $@ $(KERNEL_OBJS) $(LIBGCC)

kernel.bin: kernel.elf
	@$(OBJCOPY) $< -O binary $@

kernel.syms: kernel.elf
	@$(NM) $< > $@

uImage: kernel.bin
	@mkimage -A arm -T kernel -C none -a 0x80000000 -e 0x80000000 -n Voron -d $< $@

usbbootImage: usbboot/usbboot.o usbboot/usbboot.ld usbboot/kernel_image.o
	@echo "Creating $@"
	@echo "Entry Point: 0x82000000"
	@echo "Load Image:  0x80000000"
	@$(CC) -T usbboot/usbboot.ld -nostdlib -nostdinc -nodefaultlibs -nostartfiles \
	-fno-builtin -o __$@ $<
	@$(OBJCOPY) __$@ -O binary $@
	@rm -f __$@
	@echo "Done"

usbboot/kernel_image.o: kernel.bin
	@$(LD) -r -b binary -o $@ $<

%.o: %.c $(HDRS)
	@echo -e "  CC\t$<"
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S $(HDRS)
	@echo -e "  AS\t$<"
	@$(CC) $(ASFLAGS) -c -o $@ $<

clean:
	@rm -f $(KERNEL_OBJS) kernel.elf kernel.bin kernel.syms uImage usbbootImage \
	usbboot/kernel_image.o usbboot/usbboot.o voron.tar.gz

targz:
	@git archive --format=tar.gz --prefix=voron/ -o voron.tar.gz HEAD
	@echo voron.tar.gz created
