obj-m = con_fnd_dd.o

CC = arm-linux-gcc
KDIR = /root/work/kernel-2.6.35

PWD = $(shell pwd)

all: module

module:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.ko *.o
	rm -rf *.symvers *.order *mod.c
