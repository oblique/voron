CROSS_COMPILE ?= arm-none-eabi-
MAKEFLAGS += -rR --no-print-directory
INC = -Iinclude
LDFLAGS = -T kernel/linker.ld
CFLAGS =  -march=armv7-r -ggdb -Wall -Wextra -Wformat-security -Wshadow \
	   -Wunreachable-code -Wpointer-arith -O2 -std=gnu99 -nostdlib \
	   -nostdinc -nodefaultlibs -nostartfiles -fno-builtin $(INC)
ASFLAGS = -march=armv7-r -ggdb -nostdlib -nostdinc -nodefaultlibs \
	   -nostartfiles -fno-builtin $(INC)
CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
NM = $(CROSS_COMPILE)nm

objs = kernel/start.o kernel/kmain.o kernel/rs232.o kernel/print.o \
	kernel/debug.o

all: kernel.elf kernel.bin kernel.syms uImage

kernel.elf: $(objs) kernel/linker.ld
	@echo -e '  LD\t'$@
	@$(CC) $(LDFLAGS) -o $@ $(objs)

kernel.bin: kernel.elf
	@$(OBJCOPY) $< -O binary $@

kernel.syms: kernel.elf
	@$(NM) $< > $@

uImage: kernel.bin
	@mkimage -A arm -T kernel -C none -a 0x82000000 -e 0x82000000 -n Voron -d $< $@

%.o: %.c
	@echo -e '  CC\t'$<
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	@echo -e '  AS\t'$<
	@$(CC) $(ASFLAGS) -c -o $@ $<

clean:
	@rm -f $(objs) kernel.elf kernel.bin kernel.syms uImage
