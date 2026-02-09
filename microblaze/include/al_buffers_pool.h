/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef __AL_BUFFERS_POOL__
#define __AL_BUFFERS_POOL__

struct al5_buffers_pool {
	int count;
	struct al5_dma_buffer **buffers;
	void **handles;
	int *fds;
};

void al5_bufpool_init(struct al5_buffers_pool *bufpool);
int al5_bufpool_allocate(struct al5_buffers_pool *bufpool,
			 struct device *device, int count, int size);
void al5_bufpool_free(struct al5_buffers_pool *bufpool, struct device *device);
int al5_bufpool_get_id(struct al5_buffers_pool *bufpool, int fd);
int al5_bufpool_reserve_fd(struct al5_buffers_pool *bufpool, int id);

#endif
