/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#include "al_vcu.h"
#include "al_codec.h"
#include "al_group.h"
#include "al_constants.h"

irqreturn_t al5_hardirq_handler(int irq, void *data)
{
	struct al5_codec_desc *codec = (struct al5_codec_desc *)data;
	u32 irq_status = al5_readl(AL5_MCU_IRQ_STA);

	if (irq_status == 0)
		return IRQ_NONE;

	al5_writel(irq_status, AL5_MCU_INTERRUPT_CLR);

	/* wait for the interrupt acknowledgment to propagate in the hw */
	al5_readl(AL5_MCU_IRQ_STA);

	return IRQ_WAKE_THREAD;
}
EXPORT_SYMBOL_GPL(al5_hardirq_handler);

irqreturn_t al5_irq_handler(int irq, void *data)
{
	struct al5_codec_desc *codec = (struct al5_codec_desc *)data;

	irq_info("Got irq from Mcu");
	al5_group_read_mails(&codec->users_group);

	return IRQ_HANDLED;
}
EXPORT_SYMBOL_GPL(al5_irq_handler);
