KDIR ?= /lib/modules/$(shell uname -r)/build
SRC  := $(shell pwd)

obj-m += common/
obj-m += al5e/
obj-m += al5d/
obj-m += dmaproxy/

all:
	$(MAKE) -C $(KDIR) M=$(SRC) O=$(O) modules

modules_install:
	$(MAKE) -C $(KDIR) M=$(SRC) O=$(O) modules_install

clean:
	$(MAKE) -C $(KDIR) M=$(SRC) O=$(O) clean
