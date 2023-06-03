export ARCH ?= arm
export CROSS_COMPILE ?= arm-linux-gnueabihf-
export CC  := $(CROSS_COMPILE)gcc
export CPP := $(CROSS_COMPILE)gcc -E 
export CXX := $(CROSS_COMPILE)g++
export AS  := $(CROSS_COMPILE)as

CFLAGS_OPTIONS  := -g -O1
CFLAGS_DEFINES  := -DLUOS
CFLAGS_INCLUDES := -I kernel/core \
	               -I include \
	               -I include/uapi \
	               -I arch/$(ARCH)/include \
	               -I arch/$(ARCH)/include/uapi

CFLAGS := $(CFLAGS_OPTIONS) $(CFLAGS_DEFINES) $(CFLAGS_INCLUDES)

srcs := \
	$(wildcard kernel/core/*.c) \
	$(wildcard arch/$(ARCH)/*.c) \
	$(wildcard arch/$(ARCH)/kernel/*.c)

objdir:= out
objs := $(addprefix $(objdir)/,$(srcs:.c=.o))
deps := $(objs:.o=.d)
elf  := $(objdir)/luos.elf

.PHONY: default

default : $(objs)

# $(elf) : $(objs)
# 	@echo Making $@...
# 	$(CC) $(CFLAGS) -o $@ $(objs)
# 	@echo Making $@ done.

$(objdir)/%.o : %.c $(objdir)/%.d
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(objdir)/%.d : %.c
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MM $< > $@
	sed -i -e "s/[^ ]\+\.o: /$(subst /,\/,$(@:.d=.o)): /g" $@

clean:
	@-rm -f $(objs) $(deps)

sinclude $(deps)

