/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#include <linux/slab.h>

#include "al_dmabuf.h"
#include <linux/dma-buf.h>
#include "al_buffers_pool.h"

void al5_bufpool_init(struct al5_buffers_pool *bufpool)
{
	memset(bufpool, 0, sizeof(*bufpool));
}

int al5_bufpool_allocate(struct al5_buffers_pool *bufpool,
			 struct device *device, int count, int size)
{
	int i;

	bufpool->count = 0;
	bufpool->buffers =
		kcalloc(count, sizeof(struct al5_dma_buffer *), GFP_KERNEL);
	if (!bufpool->buffers)
		goto fail_buffers;

	bufpool->handles = kcalloc(count, sizeof(void *), GFP_KERNEL);
	if (!bufpool->handles)
		goto fail_handles;

	for (i = 0; i < count; i++) {
		bufpool->buffers[i] = al5_alloc_dma(device, size);
		if (bufpool->buffers[i] == NULL)
			goto fail_dma_allocation;
		bufpool->handles[i] =
			al5_dmabuf_wrap(device, size, bufpool->buffers[i]);
		if (IS_ERR(bufpool->handles[i])) {
			al5_free_dma(device, bufpool->buffers[i]);
			goto fail_dma_allocation;
		}
		++bufpool->count;
	}

	return 0;

fail_dma_allocation:
	al5_bufpool_free(bufpool, device);
	return -ENOMEM;

fail_handles:
	kfree(bufpool->buffers);
fail_buffers:
	return -ENOMEM;
}
EXPORT_SYMBOL_GPL(al5_bufpool_allocate);

void al5_bufpool_free(struct al5_buffers_pool *bufpool, struct device *device)
{
	int i;

	for (i = 0; i < bufpool->count; ++i)
		dma_buf_put(bufpool->handles[i]);

	kfree(bufpool->buffers);
	kfree(bufpool->handles);
	bufpool->buffers = NULL;
	bufpool->handles = NULL;
	memset(bufpool, 0, sizeof(*bufpool));
}
EXPORT_SYMBOL_GPL(al5_bufpool_free);

int al5_bufpool_get_id(struct al5_buffers_pool *bufpool, int fd)
{
	u32 i = 0;
	struct dma_buf *handle = dma_buf_get(fd);

	if (IS_ERR(handle))
		return -1;

	for (i = 0; i < bufpool->count; ++i) {
		if (handle == bufpool->handles[i]) {
			dma_buf_put(handle);
			return i;
		}
	}
	dma_buf_put(handle);
	return -1;
}
EXPORT_SYMBOL_GPL(al5_bufpool_get_id);

int al5_bufpool_reserve_fd(struct al5_buffers_pool *bufpool, int id)
{
	struct dma_buf *dbuf = bufpool->handles[id];
	int fd = dma_buf_fd(dbuf, O_RDWR);

	dma_buf_get(fd);
	return fd;
}
EXPORT_SYMBOL_GPL(al5_bufpool_reserve_fd);
