/*
 * al_alloc.c dma buffers allocation and dmabuf support
 *
 * Copyright (C) 2016, Sebastien Alaiwan (sebastien.alaiwan@allegrodvt.com)
 * Copyright (C) 2016, Kevin Grandemange (kevin.grandemange@allegrodvt.com)
 * Copyright (C) 2016, Antoine Gruzelle (antoine.gruzelle@allegrodvt.com)
 * Copyright (C) 2016, Allegro DVT (www.allegrodvt.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/dma-buf.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "al_alloc.h"

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Kevin Grandemange");
MODULE_AUTHOR("Sebastien Alaiwan");
MODULE_AUTHOR("Antoine Gruzelle");
MODULE_DESCRIPTION("Allegro Common");

int al5_allocate_dmabuf(struct device *dev, int size, u32 *fd)
{
	struct al5_dma_buffer *buffer;

	buffer = al5_alloc_dma(dev, size);
	if (!buffer) {
		dev_err(dev, "Can't alloc DMA buffer\n");
		return -ENOMEM;
	}

	*fd = al5_create_dmabuf_fd(dev, size, buffer);
	return 0;
}
EXPORT_SYMBOL_GPL(al5_allocate_dmabuf);

int al5_get_dmabuf_info(struct device *dev, u32 fd, struct al5_buffer_info *info)
{
	struct dma_buf *dbuf;
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
	int err = 0;

	dbuf = dma_buf_get(fd);
	if (IS_ERR(dbuf)) {
		return -EINVAL;
	}
	attach = dma_buf_attach(dbuf, dev);
	if (IS_ERR(attach)) {
		err = -EINVAL;
		goto fail_attach;
	}
	sgt = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(sgt)) {
		err = -EINVAL;
		goto fail_map;
	}

	info->bus_address = sg_dma_address(sgt->sgl);
	info->size = dbuf->size;

	dma_buf_unmap_attachment(attach, sgt, DMA_BIDIRECTIONAL);
fail_map:
	dma_buf_detach(dbuf, attach);
fail_attach:
	dma_buf_put(dbuf);
	return err;
}
EXPORT_SYMBOL_GPL(al5_get_dmabuf_info);

int al5_dmabuf_get_address(struct device *dev, u32 fd, u32 *bus_address)
{
	struct dma_buf *dbuf;
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
	int err = 0;

	dbuf = dma_buf_get(fd);
	if (IS_ERR(dbuf)) {
		return -EINVAL;
	}
	attach = dma_buf_attach(dbuf, dev);
	if (IS_ERR(attach)) {
		err = -EINVAL;
		goto fail_attach;
	}
	sgt = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(sgt)) {
		err = -EINVAL;
		goto fail_map;
	}

	*bus_address = sg_dma_address(sgt->sgl);

	dma_buf_unmap_attachment(attach, sgt, DMA_BIDIRECTIONAL);
fail_map:
	dma_buf_detach(dbuf, attach);
fail_attach:
	dma_buf_put(dbuf);
	return err;
}
EXPORT_SYMBOL_GPL(al5_dmabuf_get_address);

struct al5_dma_buffer *al5_alloc_dma(struct device *dev, size_t size)
{
	struct al5_dma_buffer *buf =
		kmalloc(sizeof(struct al5_dma_buffer),
			     GFP_KERNEL);
	if (!buf) {
		return NULL;
	}

	buf->size = size;
	buf->cpu_handle = dma_alloc_coherent(dev, buf->size,
			       	   &buf->dma_handle,
				   GFP_KERNEL | GFP_DMA);

	if (!buf->cpu_handle) {
		kfree(buf);
		return NULL;
	}

	return buf;
}
EXPORT_SYMBOL_GPL(al5_alloc_dma);

void al5_free_dma(struct device *dev, struct al5_dma_buffer *buf)
{
	if (buf)
		dma_free_coherent(dev, buf->size, buf->cpu_handle, buf->dma_handle);
	kfree(buf);
}
EXPORT_SYMBOL_GPL(al5_free_dma);


