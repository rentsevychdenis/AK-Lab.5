ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
ccflags-y := -I$(src)/inc
obj-m := hello1.o hello2.o
else
# normal makefile
KDIR ?= /lib/modules/$(shell uname -r)/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

checkpatch:
	$(KDIR)/scripts/checkpatch.pl --no-tree -f hello1.c hello2.c inc/hello1.h

.PHONY: default clean checkpatch
endif
