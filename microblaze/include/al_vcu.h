/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef _AL_VCU_H_
#define _AL_VCU_H_

#include <linux/interrupt.h>

#define AL5_NR_DEVS 4
#define MAX_USERS_NB 256

extern int max_users_nb;

irqreturn_t al5_irq_handler(int irq, void *data);
irqreturn_t al5_hardirq_handler(int irq, void *data);

#endif /* _AL_VCU_H_ */
