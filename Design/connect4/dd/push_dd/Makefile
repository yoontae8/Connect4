obj-m = con_push_dd.o

KDIR = /root/work/kernel-2.6.35
PWD = $(shell pwd)
CC = arm-linux-gcc

all: module

module:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.ko *.o *.mod.* *.symvers *.order
