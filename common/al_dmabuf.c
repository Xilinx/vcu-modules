/*
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


#include "al_dmabuf.h"

#include <linux/uaccess.h>
#include <linux/dma-buf.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Kevin Grandemange");
MODULE_AUTHOR("Sebastien Alaiwan");
MODULE_AUTHOR("Antoine Gruzelle");
MODULE_DESCRIPTION("Allegro Common");

struct al5_dmabuf_priv {
	struct al5_dma_buffer *buffer;

	/* DMABUF related */
	struct device *dev;
	struct sg_table *sgt_base;
	enum dma_data_direction dma_dir;

};

struct al5_dmabuf_attachment {
	struct sg_table sgt;
	enum dma_data_direction dma_dir;
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
/* device argument was removed */
static int al5_dmabuf_attach(struct dma_buf *dbuf, struct dma_buf_attachment *dbuf_attach)
#else
static int al5_dmabuf_attach(struct dma_buf *dbuf, struct device* dev, struct dma_buf_attachment *dbuf_attach)
#endif
{
	struct al5_dmabuf_priv *dinfo = dbuf->priv;

	struct al5_dmabuf_attachment *attach;

	struct scatterlist *rd, *wr;
	struct sg_table *sgt;
	int ret, i;

	attach = kzalloc(sizeof(*attach), GFP_KERNEL);
	if (!attach)
		return -ENOMEM;

	sgt = &attach->sgt;

	ret = sg_alloc_table(sgt, dinfo->sgt_base->orig_nents, GFP_KERNEL);
	if (ret) {
		kfree(attach);
		return -ENOMEM;
	}

	rd = dinfo->sgt_base->sgl;
	wr = sgt->sgl;

	for (i = 0; i < sgt->orig_nents; ++i) {
		sg_set_page(wr, sg_page(rd), rd->length, rd->offset);
		rd = sg_next(rd);
		wr = sg_next(wr);
	}

	attach->dma_dir = DMA_NONE;

	dbuf_attach->priv = attach;

	return 0;
}

static void al5_dmabuf_detach(struct dma_buf *dbuf,
			      struct dma_buf_attachment *db_attach)
{
	struct al5_dmabuf_attachment *attach = db_attach->priv;
	struct sg_table *sgt;

	if (!attach)
		return;

	sgt = &attach->sgt;

	/* release the scatterlist cache */
	if (attach->dma_dir != DMA_NONE)
		dma_unmap_sg(db_attach->dev, sgt->sgl, sgt->orig_nents,
			     attach->dma_dir);

	sg_free_table(sgt);
	kfree(attach);
	db_attach->priv = NULL;
}

static struct sg_table *al5_dmabuf_map(struct dma_buf_attachment *db_attach,
				       enum dma_data_direction dma_dir)
{
	struct al5_dmabuf_attachment *attach = db_attach->priv;
	struct sg_table *sgt;
	struct mutex *lock = &db_attach->dmabuf->lock;

	mutex_lock(lock);

	sgt = &attach->sgt;

	if (attach->dma_dir == dma_dir) {
		mutex_unlock(lock);
		return sgt;
	}

	if (attach->dma_dir != DMA_NONE) {
		dma_unmap_sg(db_attach->dev, sgt->sgl, sgt->orig_nents,
			     attach->dma_dir);
		attach->dma_dir = DMA_NONE;
	}

	sgt->nents = dma_map_sg(db_attach->dev, sgt->sgl, sgt->orig_nents,
				dma_dir);

	if (!sgt->nents) {
		pr_err("failed to map scatterlist\n");
		mutex_unlock(lock);
		return ERR_PTR(-EIO);
	}

	attach->dma_dir = dma_dir;

	mutex_unlock(lock);

	return sgt;
}

static void al5_dmabuf_unmap(struct dma_buf_attachment *at,
			     struct sg_table *sg, enum dma_data_direction dir)
{
}

static int al5_dmabuf_mmap(struct dma_buf *buf, struct vm_area_struct *vma)
{
	struct al5_dmabuf_priv *dinfo = buf->priv;
	unsigned long start = vma->vm_start;
	unsigned long vsize = vma->vm_end - start;
	struct al5_dma_buffer *buffer = dinfo->buffer;
	int ret;

	if (!dinfo) {
		pr_err("No buffer to map\n");
		return -EINVAL;
	}

	vma->vm_pgoff = 0;

	ret = dma_mmap_coherent(dinfo->dev, vma, buffer->cpu_handle,
				buffer->dma_handle, vsize);

	if (ret < 0) {
		pr_err("Remapping memory failed, error: %d\n", ret);
		return ret;
	}

	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;

	return 0;
}

static void al5_dmabuf_release(struct dma_buf *buf)
{
	struct al5_dmabuf_priv *dinfo = buf->priv;
	struct al5_dma_buffer *buffer = dinfo->buffer;

	if (dinfo->sgt_base) {
		sg_free_table(dinfo->sgt_base);
		kfree(dinfo->sgt_base);
	}


	dma_free_coherent(dinfo->dev, buffer->size, buffer->cpu_handle,
			  buffer->dma_handle);

	put_device(dinfo->dev);
	kzfree(buffer);
	kfree(dinfo);
}

static void *al5_dmabuf_kmap(struct dma_buf *dmabuf, unsigned long page_num)
{
	struct al5_dmabuf_priv *dinfo = dmabuf->priv;
	void *vaddr = dinfo->buffer->cpu_handle;

	return vaddr + page_num * PAGE_SIZE;
}

static void *al5_dmabuf_vmap(struct dma_buf *dbuf)
{
	struct al5_dmabuf_priv *dinfo = dbuf->priv;
	void *vaddr = dinfo->buffer->cpu_handle;

	return vaddr;
}

static const struct dma_buf_ops al5_dmabuf_ops = {
	.attach		= al5_dmabuf_attach,
	.detach		= al5_dmabuf_detach,
	.map_dma_buf	= al5_dmabuf_map,
	.unmap_dma_buf	= al5_dmabuf_unmap,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
/* the map_atomic interface was removed after 4.19 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
	.map_atomic	= al5_dmabuf_kmap,
#endif
	.map		= al5_dmabuf_kmap,
#else
	.kmap_atomic	= al5_dmabuf_kmap,
	.kmap		= al5_dmabuf_kmap,
#endif
	.vmap		= al5_dmabuf_vmap,
	.mmap		= al5_dmabuf_mmap,
	.release	= al5_dmabuf_release,
};

static void define_export_info(struct dma_buf_export_info *exp_info,
			       int size,
			       void *priv)
{
	exp_info->owner = THIS_MODULE;
	exp_info->exp_name = KBUILD_MODNAME;
	exp_info->ops = &al5_dmabuf_ops;
	exp_info->flags = O_RDWR;
	exp_info->resv = NULL;
	exp_info->size = size;
	exp_info->priv = priv;
}

static struct sg_table *al5_get_base_sgt(struct al5_dmabuf_priv *dinfo)
{
	int ret;
	struct sg_table *sgt;
	struct al5_dma_buffer *buf = dinfo->buffer;
	struct device *dev = dinfo->dev;

	sgt = kzalloc(sizeof(*sgt), GFP_KERNEL);
	if (!sgt)
		return NULL;

	ret = dma_get_sgtable(dev, sgt, buf->cpu_handle, buf->dma_handle,
			      buf->size);
	if (ret < 0) {
		kfree(sgt);
		return NULL;
	}

	return sgt;

}

static struct dma_buf *al5_get_dmabuf(void *dma_info_priv)
{
	struct dma_buf *dbuf;
	struct dma_buf_export_info exp_info;

	struct al5_dmabuf_priv *dinfo = dma_info_priv;

	struct al5_dma_buffer *buf = dinfo->buffer;

	define_export_info(&exp_info,
			   buf->size,
			   (void *)dinfo);

	if (!dinfo->sgt_base)
		dinfo->sgt_base = al5_get_base_sgt(dinfo);

	if (WARN_ON(!dinfo->sgt_base))
		return NULL;

	dbuf = dma_buf_export(&exp_info);
	if (IS_ERR(buf)) {
		pr_err("couldn't export dma buf\n");
		return NULL;
	}

	return dbuf;
}

void *al5_dmabuf_wrap(struct device *dev, unsigned long size,
		      struct al5_dma_buffer *buffer)
{
	struct al5_dmabuf_priv *dinfo;
	struct dma_buf *dbuf;

	dinfo = kzalloc(sizeof(*dinfo), GFP_KERNEL);
	if (!dinfo)
		return ERR_PTR(-ENOMEM);

	dinfo->dev = get_device(dev);
	dinfo->buffer = buffer;
	dinfo->dma_dir = DMA_BIDIRECTIONAL;
	dinfo->sgt_base = al5_get_base_sgt(dinfo);

	dbuf = al5_get_dmabuf(dinfo);
	if (IS_ERR_OR_NULL(dbuf))
		return ERR_PTR(-EINVAL);

	return dbuf;
}
EXPORT_SYMBOL_GPL(al5_dmabuf_wrap);

int al5_create_dmabuf_fd(struct device *dev, unsigned long size,
			 struct al5_dma_buffer *buffer)
{
	struct dma_buf *dbuf = al5_dmabuf_wrap(dev, size, buffer);

	if (IS_ERR(dbuf))
		return PTR_ERR(dbuf);
	return dma_buf_fd(dbuf, O_RDWR);
}
EXPORT_SYMBOL_GPL(al5_create_dmabuf_fd);

int al5_allocate_dmabuf(struct device *dev, int size, u32 *fd)
{
	struct al5_dma_buffer *buffer;

	buffer = al5_alloc_dma(dev, size);
	if (!buffer) {
		dev_err(dev, "Can't alloc DMA buffer of size %d", size);
		return -ENOMEM;
	}

	*fd = al5_create_dmabuf_fd(dev, size, buffer);
	return 0;
}
EXPORT_SYMBOL_GPL(al5_allocate_dmabuf);

int al5_get_dmabuf_info(struct device *dev, u32 fd,
			struct al5_buffer_info *info)
{
	struct dma_buf *dbuf;
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
	int err = 0;

	dbuf = dma_buf_get(fd);
	if (IS_ERR(dbuf))
		return -EINVAL;
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
	if (IS_ERR(dbuf))
		return -EINVAL;
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

