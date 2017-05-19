#ifndef __MCU_INTERFACE_PRIVATE__
#define __MCU_INTERFACE_PRIVATE__

#include <linux/spinlock.h>
#include "mcu_interface.h"

struct mcu_mailbox_interface {
	struct mailbox *mcu_to_cpu;
	struct mailbox *cpu_to_mcu;
	spinlock_t read_lock;
	spinlock_t write_lock;
	void *interrupt_register;

};

#endif
