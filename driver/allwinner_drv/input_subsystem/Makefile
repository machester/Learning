# ------------------------------------------------------- #
# Author : 
# Data :
# Version
# ------------------------------------------------------- #
# get system verison
KVERS = $(shell uname -r)
# get driver floder path
CURDIR := $(shell pwd)

# Kernel modules
obj-m += usr_platform_drv.o
obj-m += usr_platform_dev.o
# module install name
MODULE_DRV = usr_platform_drv
MODULE_DEV = usr_platform_dev

# Specify flags for the module compilation.
#EXTRA_CFLAGS=-g -O0

temp_drv += ${MODULE_DRV}.ko 
temp_dev += ${MODULE_DEV}.ko 

all : kernel_modules

kernel_modules:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules
	@echo "---> build finished."
clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
	rm -rf .*.?.sw?p 
	rm -rf .*.?.swp 
	@echo "---> clean trash finished."
clean_trash:
	@echo "---> clean swap file."
	rm -rf .*.?.sw?p 
	rm -rf .*.?.swp
	@echo "---> clean trash file."
	rm -rf *.o
	rm -rf *.*.o
	rm -rf .*.cmd
	rm -rf .tmp_versions
	rm -rf *odul*.*
	rm -rf .cache.mk
install:
	@echo "---> install module $(temp_drv)"
	$(shell sudo insmod $(temp_drv))
	@echo "---> install module $(temp_dev)"
	$(shell sudo insmod $(temp_dev))
remove:
	@echo "---> remove module $(temp_drv)"
	$(shell sudo rmmod $(temp_drv))
	@echo "---> remove module $(temp_dev)"
	$(shell sudo rmmod $(temp_dev))
	
