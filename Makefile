KDIR ?= /lib/modules/$(shell uname -r)/build

default:
	$(MAKE) -C $(KDIR) M=$(shell pwd)

clean:
	$(MAKE) -C $(KDIR) M=$(shell pwd) clean
