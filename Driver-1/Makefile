obj-m += driver1.o  

ifdef ARCH
	KDIR = /home/pratham/temp 
else
	KDIR = /lib/modules/$(shell uname -r)/build
endif

all:
	make -C $(KDIR) M=$(shell pwd) modules
clean:
	make -C $(KDIR) M=$(shell pwd) clean
