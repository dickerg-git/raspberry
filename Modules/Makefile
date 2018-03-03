#
# make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
#

obj-m += Achar_mod.o

KDIR = /home/rogerd/cdev/kernel/raspberry3/linux


all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules


clean:
	rm -rf *.o *.ko *.mod.* *.symvers *.order

