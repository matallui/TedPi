ARMGNU ?= arm-none-eabi
CC = $(ARMGNU)-gcc
AS := $(shell $(CC) -print-prog-name=as)
LD := $(shell $(CC) -print-prog-name=ld)
OBJCOPY := $(shell $(CC) -print-prog-name=objcopy)
OBJDUMP := $(shell $(CC) -print-prog-name=objdump)
NM := $(shell $(CC) -print-prog-name=nm)

RPI=RPI0

ifeq ($(RPI), RPI2)
CPU = cortex-a7
ARCH = armv7-a
FPU = neon-vfpv4
LDLIBS = -Llib/armv7-ar -lgcc
else
CPU = arm1176jzf-s
ARCH = armv6zk
FPU = vfp
LDLIBS = -Llib/armv6-m -lgcc
endif

ASOPTS = --warn -mfpu=$(FPU) -mfloat-abi=hard -march=$(ARCH) #-mcpu=cortex-a7 -g 
CCOPTS = -Wall -O2 -mfpu=$(FPU) -mfloat-abi=hard -march=$(ARCH) -mtune=$(CPU) -D$(RPI) -fno-builtin -nostdlib #-nostartfiles -g
#LDLIBS = -Llib -lgcc
LDSCRIPT = rpi.x
LDOPTS = -T $(LDSCRIPT) #--verbose

COBJS = kernel.o \
		cstartup.o \
		clib.o \
		gpio.o \
		led.o \
		uart.o \
		twi.o \
		irq.o \
		systimer.o \
		cmd.o \
		cli.o \
		drivers/ds1337.o

ASOBJS = start.o

all: kernel.img

clean:
	-rm -rf $(COBJS) $(ASOBJS) kernel.elf kernel.img *.nm *.asm debug/

debug: kernel.elf
	mkdir -p debug
	$(OBJDUMP) -d kernel.elf > debug/kernel.asm
	$(NM) kernel.elf > debug/kernel.nm

%.o: %.s
	$(AS) $(ASOPTS) -o $@ $<

%.o: %.c
	$(CC) $(CCOPTS) -c -o $@ $<

#kernel.elf: $(LD_SCRIPT) $(ASOBJS) $(COBJS)
#	$(CC) $(CCOPT) -Wl,-T,$(LD_SCRIPT) -o kernel.elf $(ASOBJS) $(COBJS)

kernel.elf: $(LDSCRIPT) $(ASOBJS) $(COBJS)
	$(LD) $(LDOPTS) -o kernel.elf $(COBJS) $(ASOBJS) $(LDLIBS)

kernel.img: kernel.elf
	$(OBJCOPY) kernel.elf -O binary kernel.img

