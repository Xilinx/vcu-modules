/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef _AL_DMABUF_H_
#define _AL_DMABUF_H_

#include <linux/device.h>
#include "al_alloc.h"

struct al5_buffer_info {
	u32 bus_address;
	u32 size;
};

void *al5_dmabuf_wrap(struct device *dev, size_t size,
		      struct al5_dma_buffer *buffer);
int al5_create_dmabuf_fd(struct device *dev, size_t size,
			 struct al5_dma_buffer *buffer);

int al5_allocate_dmabuf(struct device *dev, size_t size, s32 *fd);
int al5_dmabuf_get_address(struct device *dev, s32 fd, u32 *bus_address);
int al5_get_dmabuf_info(struct device *dev, s32 fd,
			struct al5_buffer_info *info);

#endif /* _AL_DMABUF_H_ */
