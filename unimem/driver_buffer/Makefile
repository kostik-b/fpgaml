
obj-m := unimem_buffer_alloc.o

KERNELDIR=/lib/modules/$(shell uname -r)/build

PWD:= $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
