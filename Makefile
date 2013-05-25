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

all: uImage kernel.syms

kernel.elf: $(KERNEL_OBJS) kernel/linker.ld
	@echo -e "  LD\t$@"
	@$(CC) $(LDFLAGS) -o $@ $(KERNEL_OBJS) $(LIBGCC)

kernel.bin: kernel.elf
	@$(OBJCOPY) $< -O binary $@

kernel.syms: kernel.elf
	@$(NM) $< > $@

uImage: kernel.bin
	@mkimage -A arm -T kernel -C none -a 0x82000000 -e 0x82000000 -n Voron -d $< $@

%.o: %.c $(HDRS)
	@echo -e "  CC\t$<"
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S $(HDRS)
	@echo -e "  AS\t$<"
	@$(CC) $(ASFLAGS) -c -o $@ $<

bootloader:
	[ -d boot/u-boot-linaro-stable ] || (cd boot && \
	    git clone git://git.linaro.org/boot/u-boot-linaro-stable.git)
	make -C boot/u-boot-linaro-stable CROSS_COMPILE=$(CROSS_COMPILE) omap4_panda_config
	make -C boot/u-boot-linaro-stable CROSS_COMPILE=$(CROSS_COMPILE)
	cd boot && mkimage -A arm -T script -C none -a 0 -e 0 -n "Panda SD Boot" -d boot_sd.conf boot.scr

clean:
	@rm -f $(KERNEL_OBJS) kernel.elf kernel.bin kernel.syms uImage voron.tar.gz

targz:
	@git archive --format=tar.gz --prefix=voron/ -o voron.tar.gz HEAD
	@echo voron.tar.gz created
