ifdef TOPDIR

obj-m += own.o

else

default:
	$(MAKE) -C $(KERNEL_DIR) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) KBUILD_VERBOSE=1 M=$(PWD) modules

distclean:
	rm -f *.ko *.o .*.cmd *.mod.c Module.symvers modules.order

endif
