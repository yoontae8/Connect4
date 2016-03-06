CC = arm-linux-gcc
KDIR = /root/work/kernel-2.6.35

obj-m = con_led_timer_dd.o

PWD = $(shell pwd)


all: module

module:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	rm -rf *.ko *.o *.symvers *.order *.mod.c *.cmd
