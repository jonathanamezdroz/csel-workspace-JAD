# Part executed when called from kernel build system:
ifneq ($(KERNELRELEASE),)
obj-m += mymodule.o		## name of the generated module

mymodule-objs := skeleton.o	## list of objects needed for that module

# Part executed when called from standard make in module source directory:
else
include ../../buildroot_path
include ../../kernel_settings
include ../../buildroot_path
PWD := $(shell pwd)
#MODPATH := /rootfs # production mode
all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(CPU) CROSS_COMPILE=$(TOOLS) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(MODPATH) modules_install

endif

