# Allegro Linux Drivers

## Summary

This repository contains the source code of al5r drivers.

The al5r driver is used by the soft\_codec repository and other control software
to access an hw ip with direct registers access. It implements a read/write
registers interface and make it possible to handle interrupts in userspace.

## Building

To build drivers you need a linux kernel build tree. Give location of
kernel build tree using KERNEL_SRC environment variable.
It you are cross-compiling then define ARCH and CROSS\_COMPILE environment
variable.
The `KERNEL_SRC` directory must contain a `.config` and should be prepared with `make modules_prepare`.

```
$ ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- KERNEL_SRC=linux-headers-dir make
```

Compiled kernel modules are located in al5r/al5r.ko

## Using on a board

Copy al5r.ko on your board and insert it.

```
$ insmod al5r.ko
```

## Device Tree Bindings

You can change configure some of the behavior of driver using device tree
bindings.
See the device-tree-bindings.txt file for more information about available
bindings.
