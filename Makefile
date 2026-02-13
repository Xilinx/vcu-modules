SRC := $(shell pwd)

ifeq ($(O),)
        include $(KERNEL_SRC)/.config
else
        include $(O)/.config
endif

default: modules
modules:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(shell pwd) clean
