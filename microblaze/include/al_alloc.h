/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef _AL_ALLOC_H_
#define _AL_ALLOC_H_

#include <linux/device.h>
#include "al_ioctl.h"

struct al5_dma_buffer {
	size_t size;
	dma_addr_t dma_handle;
	void *cpu_handle;
	size_t offset; /* internal */
};

struct al5_dma_buffer *al5_alloc_dma(struct device *dev, size_t size);
void al5_free_dma(struct device *dev, struct al5_dma_buffer *buf);

#endif /* _AL_ALLOC_H_ */
