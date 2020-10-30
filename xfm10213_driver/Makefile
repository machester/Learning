# get system verison
KVERS = $(shell uname -r)
# get driver floder path
CURDIR := $(shell pwd)
# module install name
MODULE_NAME = xfm10213
# Kernel modules
obj-m += ${MODULE_NAME}.o
# Specify flags for the module compilation.
#EXTRA_CFLAGS=-g -O0
CC = gcc -Wall
APPLICATION_NAME = app_${MODULE_NAME}

driver += ${MODULE_NAME}.ko


all : kernel_modules
kernel_modules:
	@echo "---> current path : ${CURDIR}"
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules
	@echo "---> build finished."
app:
	@echo "---> make application ${APPLICATION_NAME}"
	$(CC) -o ${APPLICATION_NAME} ${APPLICATION_NAME}.c -lpthread
cleanall:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
	rm -rvf .*.?.sw?p 
	rm -rvf .*.?.swp 
	rm -rvf ${APPLICATION_NAME}
	@echo "---> all clean"
clean:
	@echo "---> clean trash file."
	rm -rvf .*.?.sw?p .*.?.swp *.o *.*.o .*.cmd .tmp_versions *.mod.c Module.symvers modules.order
install:
	@echo "---> install module $(device)"
	$(shell sudo insmod ${device})
	@echo "---> install module $(driver)"
	$(shell sudo insmod ${driver})
remove:
	@echo "---> remove module $(MODULE_NAME)"
	$(shell sudo rmmod $(MODULE_NAME))
.PHONY: kernel_modules app cleanall clean install remove
# ------------------------------- END LINE -----------------------------------------