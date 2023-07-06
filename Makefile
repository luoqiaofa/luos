export ARCH ?= arm
export CROSS_COMPILE ?= arm-linux-gnueabihf-
export CC  := $(CROSS_COMPILE)gcc
export LD  := $(CROSS_COMPILE)ld
export CPP := $(CROSS_COMPILE)gcc -E 
export CXX := $(CROSS_COMPILE)g++
export AS  := $(CROSS_COMPILE)as
export AR  := $(CROSS_COMPILE)ar
export OBJCOPY  := $(CROSS_COMPILE)objcopy
export OBJDUMP  := $(CROSS_COMPILE)objdump

export CONFIG_SOC_IMX6UL=y

BUILD_DATE := $(shell date "+%Y-%m-%d")
BUILD_TIME := $(shell date "+%H:%M:%S")
BUILD_MACROS := -DBUILD_DATE=$(BUILD_DATE) -DBUILD_TIME=$(BUILD_TIME)
ifeq ($(strip $(CONFIG_SOC_IMX6UL)), y)
BUILD_MACROS += -DCONFIG_SOC_IMX6UL=1
endif

CFLAGS_OPTIONS  := -g -O1 -Wall -Wa,-mimplicit-it=thumb -nostdlib -fno-builtin
CFLAGS_DEFINES  := -DLUOS $(BUILD_MACROS)
INCLUDES := kernel/core \
	        include \
	        include/uapi \
	        arch/$(ARCH)/include \
	        arch/$(ARCH)/include/uapi

ifeq ($(strip $(CONFIG_SOC_IMX6UL)), y)
INCLUDES += arch/arm/cpu/armv7
INCLUDES += arch/arm/mach-imx6/imx6ul
INCLUDES += arch/arm/mach-imx6/bsp/int
INCLUDES += arch/arm/mach-imx6/bsp/clk
INCLUDES += arch/arm/mach-imx6/bsp/gpio
INCLUDES += arch/arm/mach-imx6/bsp/uart
INCLUDES += arch/arm/mach-imx6/bsp/led
INCLUDES += arch/arm/mach-imx6/bsp/epittimer
INCLUDES += lib/symbolParse
endif

CFLAGS_INCLUDES := $(addprefix -I,$(INCLUDES))

CFLAGS := $(CFLAGS_OPTIONS) $(CFLAGS_DEFINES) $(CFLAGS_INCLUDES)

LD_FLAGS := -lgcc -L/opt/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf/lib/gcc/arm-none-linux-gnueabihf/9.2.1

srcs := \
	$(wildcard init/*.c) \
	$(wildcard kernel/core/*.c) \
	$(wildcard arch/$(ARCH)/*.c)
ifeq ($(strip $(CONFIG_SOC_IMX6UL)), y)
	srcs += $(wildcard arch/arm/cpu/armv7/*.S)
	srcs += $(wildcard arch/arm/cpu/armv7/*.c)
	srcs += $(wildcard arch/arm/mach-imx6/*.c)
	srcs += $(wildcard arch/arm/mach-imx6/bsp/int/*.c)
	srcs += $(wildcard arch/arm/mach-imx6/bsp/clk/*.c)
	srcs += $(wildcard arch/arm/mach-imx6/bsp/gpio/*.c)
	srcs += $(wildcard arch/arm/mach-imx6/bsp/uart/*.c)
	srcs += $(wildcard arch/arm/mach-imx6/bsp/led/*.c)
	srcs += $(wildcard arch/arm/mach-imx6/bsp/epittimer/*.c)
	srcs += $(wildcard lib/stdio/*.c)
	srcs += $(wildcard lib/symbolParse/*.c)
else
    srcs += $(wildcard arch/$(ARCH)/kernel/*.c)
endif

O ?= out
objdir := $(O)
cfiles := $(filter %.c,$(srcs))
asms   := $(filter %.S,$(srcs))
# $(info cfiles=$(cfiles))
objs   := $(addprefix $(objdir)/,$(cfiles:.c=.o))
objs   += $(addprefix $(objdir)/,$(asms:.S=.o))
objs   += $(objdir)/version.o
# $(info objs=$(objs))
deps   := $(objs:.o=.d)
elf    := $(objdir)/luos.elf

.PHONY: default prepare

default : $(elf)

$(elf) : $(objs) $(objdir)/linkSyms.o
	@echo Making $@...
	@$(LD) -T arch/arm/mach-imx6/imx6ul.lds -o $(elf) $(objs) $(objdir)/linkSyms.o $(LD_FLAGS)
	$(OBJCOPY) -O binary -S $(elf) $(objdir)/luos.bin
	$(OBJDUMP) -Dszt $@ > $(objdir)/luos.S
	@echo Making $@ done.

prepare:
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	@python3 scripts/gen-version.py > $(objdir)/version.c 

$(objdir)/version.c : prepare

$(objdir)/version.o : $(objdir)/version.c
	@$(CC) $(CFLAGS) -c -o $@ $<

$(objdir)/linkSyms.o : $(objs) 
	@$(AR) cr $(objdir)/tmp.a $(objs)
	@sh ./gen_linkSyms.sh $(objdir)/tmp.a > $(objdir)/linkSyms.c
	@$(CC) $(CFLAGS) -c -o $@ $(objdir)/linkSyms.c
	@-rm -f $(objdir)/tmp.a 
	@-rm -f $(objdir)/version.c

$(objdir)/%.o : %.c $(objdir)/%.d
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(objdir)/%.o : %.S $(objdir)/%.d
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(objdir)/%.d : %.S
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MM $< > $@
	@sed -i -e "s/[^ ]\+\.o: /$(subst /,\/,$(@:.d=.o)): /g" $@

$(objdir)/%.d : %.c
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MM $< > $@
	@sed -i -e "s/[^ ]\+\.o: /$(subst /,\/,$(@:.d=.o)): /g" $@

clean:
	@-rm -f $(objs) $(deps)

sinclude $(deps)

