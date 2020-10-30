# if signature error
# CONFIG_MODULE_SIG=n

ifneq ($(KERNELRELEASE),)
	MODULE_NAME = oled_driver
	${MODULE_NAME}_module-objs := 	oled_driver.o \
									oled_operation.o
	obj-m := ${MODULE_NAME}_module.o

else
# ------------------------------------------------------- #
# get system verison
KVERS = $(shell uname -r)
# module install name
MODULE_NAME = oled_driver

# module test app name
APP_NAME = app_${MODULE_NAME}

CROSS_COMPILE = arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc

# for aliyun server
KERNEL_DIR = /arm/tinker/debian_kernel
# for home virtual ubuntu 18.04
# KERNEL_DIR = /home/quan/tinker/debian_kernel

# get driver floder path
CUR_DIR = $(shell pwd)

all : modules
modules:
	$(MAKE) -C $(KERNEL_DIR) M=$(CUR_DIR) modules ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
	@echo "---> kernel_modules build finished."

app :
	$(CC) $(APP_NAME).c -o $(APP_NAME)
	@echo "---> app build finished."

cleanall :
	$(MAKE) -C $(KERNEL_DIR) M=$(CUR_DIR) clean
	rm -rvf $(APP_NAME)
	rm -rvf .*.?.sw?p
	rm -rvf .*.?.swp
	@echo "---> all clean."

clean:
	rm -rvf .*.?.sw?p .*.?.swp *.o *.*.o .*.cmd .tmp_versions *.mod.c Module.symvers modules.order
	@echo "---> clean finished."

install:
	@echo "---> install module ${MODULE_NAME}_module"
	$(shell sudo insmod ${MODULE_NAME}_module.ko)
remove:
	@echo "---> remove module ${MODULE_NAME}_module"
	$(shell sudo rmmod ${MODULE_NAME}_module)

.PHONY: modules cleanall clean install remove
endif
#-------------------- END LINE --------------------------
