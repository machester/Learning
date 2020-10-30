# get system verison
KVERS = $(shell uname -r)
# get DRIVER_NAME floder path
CURDIR := $(shell pwd)
# module install name
DRIVER_NAME = customer_home_key_input_trigger_ok
# Kernel modules
obj-m += ${DRIVER_NAME}.o

# Specify flags for the module compilation.
#EXTRA_CFLAGS=-g -O0
CC = gcc -Wall
APPLICATION_NAME = app_${DRIVER_NAME}

DRV_MODULE_NAME += ${DRIVER_NAME}.ko
# DEV_MODULE_NAME += ${MODULE_DEV_NAME}.ko


all: module
module:
	@echo "---> current path : ${CURDIR}"
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules
	@echo "---> build finished."
app:
	@echo "---> make application ${APPLICATION_NAME}"
	$(CC) -o ${APPLICATION_NAME} ${APPLICATION_NAME}.c -lpthread
cleanall:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
	rm -rvf .*.?.sw?p  .*.?.swp 
	rm -rvf ${APPLICATION_NAME}
	@echo "---> all clean"
clean:
	@echo "---> clean trash file."
	rm -rvf .*.?.sw?p .*.?.swp *.o *.*.o .*.cmd .tmp_versions *.mod.c Module.symvers modules.order
install:
	@echo "---> install module ${DRV_MODULE_NAME}"
	$(shell sudo insmod ${DRV_MODULE_NAME})
remove:
	@echo "---> remove module ${DRIVER_NAME}"
	$(shell sudo rmmod ${DRIVER_NAME})
# info:
# 	@echo "---> get module info ${DRV_MODULE_NAME}"
# 	$(shell sudo modinfo ${DRV_MODULE_NAME})
.PHONY: module app cleanall clean install remove info
# ------------------------------- END LINE -----------------------------------------
