// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx zynqmp-dma client driver
 *
 * Copyright (C) 2019 Xilinx, Inc.
 *
 * Author: Jeegar Patel <jeegar.patel@xilinx.com>
 *
 * This driver uses the dma engine subsystem's API to perform
 * memory to memory copy operation using zynqmp-dma and share
 * IOCTL so userspace application use them
 */

#include <linux/cdev.h>
#include <linux/dma-buf.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_dma.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/version.h>

#include "dmaproxy.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 13, 0)
MODULE_IMPORT_NS("DMA_BUF");
#elif LINUX_VERSION_CODE > KERNEL_VERSION(5, 15, 0)
MODULE_IMPORT_NS(DMA_BUF);
#endif

static dev_t dmaproxy_dev;
static struct cdev dmaproxy_cdev;
static struct class *dmaproxy_cl;
#define MINOR_CNT 8

/* Upper bound on a single memcpy. The engine moves several hundred MB/s, so
 * this only ever trips when the transfer is genuinely stuck.
 */
#define DMAPROXY_COPY_TIMEOUT_MS 5000

struct dmacopy_done {
	bool done;
	wait_queue_head_t wait;
};

struct dmaproxy_data {
	struct dma_chan *chan;
	/* serialises DMAPROXY_COPY against the per-file transfer state below */
	struct mutex lock;
	struct dma_buf *src_dbuf;
	struct dma_buf_attachment *src_attach;
	struct sg_table *src_sgt;
	struct dma_buf *dst_dbuf;
	struct dma_buf_attachment *dst_attach;
	struct sg_table *dst_sgt;
	dma_addr_t src_buf;
	dma_addr_t dst_buf;
	struct dmacopy_done done;
};

static void dmacopy_callback(void *arg)
{
	struct dmacopy_done *done = arg;

	done->done = true;
	wake_up(&done->wait);
}

static int dmaproxy_open(struct inode *i, struct file *f)
{
	struct dmaproxy_data *dmaproxy_data;
	dma_cap_mask_t mask;
	struct dma_chan *chan;

	dmaproxy_data = kzalloc(sizeof(*dmaproxy_data), GFP_KERNEL);
	if (!dmaproxy_data)
		return -ENOMEM;

	mutex_init(&dmaproxy_data->lock);

	dma_cap_zero(mask);
	dma_cap_set(DMA_MEMCPY, mask);

	chan = dma_request_channel(mask, NULL, NULL);
	if (!chan) {
		pr_err("dmaproxy: No more DMA channel available\n");
		kfree(dmaproxy_data);
		return -EBUSY;
	}

	dmaproxy_data->chan = chan;

	f->private_data = dmaproxy_data;

	return 0;
}

static int dmaproxy_close(struct inode *i, struct file *f)
{
	struct dmaproxy_data *dmaproxy_data = f->private_data;

	dmaengine_terminate_all(dmaproxy_data->chan);
	dma_release_channel(dmaproxy_data->chan);
	dmaproxy_data->chan = NULL;

	kfree(dmaproxy_data);
	return 0;
}

/* Check that [offset, offset + size) lies inside the dma_buf. The engine has
 * no notion of the buffer bounds, so anything not rejected here is programmed
 * into the hardware as-is.
 */
static int dmabuf_check_range(struct dma_buf *dbuf, int offset, size_t size)
{
	if (offset < 0 || size == 0 || size > dbuf->size)
		return -EINVAL;

	if ((size_t)offset > dbuf->size - size)
		return -EINVAL;

	return 0;
}

static int dmabuf_get_address(struct dmaproxy_data *dmaproxy_data,
			      dmaproxy_arg_t dmaproxy)
{
	struct dma_device *dma_dev = dmaproxy_data->chan->device;
	dma_addr_t align_mask = (1UL << dma_dev->copy_align) - 1;

	dmaproxy_data->src_dbuf = dma_buf_get(dmaproxy.src_fd);
	if (IS_ERR(dmaproxy_data->src_dbuf)) {
		pr_err("dmaproxy: dma_buf_get failed for src\n");
		return -EINVAL;
	}

	if (dmabuf_check_range(dmaproxy_data->src_dbuf, dmaproxy.src_offset,
			       dmaproxy.size)) {
		pr_err("dmaproxy: src range [%d, +%zu) outside %zu-byte buffer\n",
		       dmaproxy.src_offset, dmaproxy.size,
		       dmaproxy_data->src_dbuf->size);
		goto err_put_src;
	}

	dmaproxy_data->src_attach =
		dma_buf_attach(dmaproxy_data->src_dbuf, dma_dev->dev);
	if (IS_ERR(dmaproxy_data->src_attach)) {
		pr_err("dmaproxy: dma_buf_attach get failed for src\n");
		goto err_put_src;
	}

	dmaproxy_data->src_sgt = dma_buf_map_attachment(
		dmaproxy_data->src_attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(dmaproxy_data->src_sgt)) {
		pr_err("dmaproxy: dma_buf_map get failed for src\n");
		goto err_detach_src;
	}

	/* Only the first scatterlist entry is programmed, so the buffer has to
	 * be contiguous in the engine's address space.
	 */
	if (dmaproxy_data->src_sgt->nents != 1) {
		pr_err("dmaproxy: src buffer is not contiguous (%u entries)\n",
		       dmaproxy_data->src_sgt->nents);
		goto err_unmap_src;
	}

	dmaproxy_data->src_buf = sg_dma_address(dmaproxy_data->src_sgt->sgl);

	if (dmaproxy.dst_fd != dmaproxy.src_fd) {
		dmaproxy_data->dst_dbuf = dma_buf_get(dmaproxy.dst_fd);
		if (IS_ERR(dmaproxy_data->dst_dbuf)) {
			pr_err("dmaproxy: dma_buf_get failed for dst\n");
			goto err_unmap_src;
		}

		if (dmabuf_check_range(dmaproxy_data->dst_dbuf,
				       dmaproxy.dst_offset, dmaproxy.size)) {
			pr_err("dmaproxy: dst range [%d, +%zu) outside %zu-byte buffer\n",
			       dmaproxy.dst_offset, dmaproxy.size,
			       dmaproxy_data->dst_dbuf->size);
			goto err_put_dst;
		}

		dmaproxy_data->dst_attach =
			dma_buf_attach(dmaproxy_data->dst_dbuf, dma_dev->dev);
		if (IS_ERR(dmaproxy_data->dst_attach)) {
			pr_err("dmaproxy: dma_buf_attach get failed for dst\n");
			goto err_put_dst;
		}

		dmaproxy_data->dst_sgt = dma_buf_map_attachment(
			dmaproxy_data->dst_attach, DMA_BIDIRECTIONAL);
		if (IS_ERR(dmaproxy_data->dst_sgt)) {
			pr_err("dmaproxy: dma_buf_map get failed for dst\n");
			goto err_detach_dst;
		}

		if (dmaproxy_data->dst_sgt->nents != 1) {
			pr_err("dmaproxy: dst buffer is not contiguous (%u entries)\n",
			       dmaproxy_data->dst_sgt->nents);
			goto err_unmap_dst;
		}

		dmaproxy_data->dst_buf =
			sg_dma_address(dmaproxy_data->dst_sgt->sgl);
	} else {
		if (dmabuf_check_range(dmaproxy_data->src_dbuf,
				       dmaproxy.dst_offset, dmaproxy.size)) {
			pr_err("dmaproxy: dst range [%d, +%zu) outside %zu-byte buffer\n",
			       dmaproxy.dst_offset, dmaproxy.size,
			       dmaproxy_data->src_dbuf->size);
			goto err_unmap_src;
		}

		dmaproxy_data->dst_buf = dmaproxy_data->src_buf;
	}

	dmaproxy_data->src_buf = dmaproxy_data->src_buf + dmaproxy.src_offset;
	dmaproxy_data->dst_buf = dmaproxy_data->dst_buf + dmaproxy.dst_offset;

	/* Reject a misaligned transfer instead of quietly relocating it: moving
	 * the address would copy from or to the wrong place.
	 */
	if ((dmaproxy_data->src_buf | dmaproxy_data->dst_buf) & align_mask) {
		pr_err("dmaproxy: transfer needs %llu-byte alignment\n",
		       (unsigned long long)align_mask + 1);
		goto err_align;
	}

	return 0;

err_align:
	if (dmaproxy.dst_fd == dmaproxy.src_fd)
		goto err_unmap_src;
err_unmap_dst:
	dma_buf_unmap_attachment(dmaproxy_data->dst_attach,
				 dmaproxy_data->dst_sgt, DMA_BIDIRECTIONAL);
err_detach_dst:
	dma_buf_detach(dmaproxy_data->dst_dbuf, dmaproxy_data->dst_attach);
err_put_dst:
	dma_buf_put(dmaproxy_data->dst_dbuf);
err_unmap_src:
	dma_buf_unmap_attachment(dmaproxy_data->src_attach,
				 dmaproxy_data->src_sgt, DMA_BIDIRECTIONAL);
err_detach_src:
	dma_buf_detach(dmaproxy_data->src_dbuf, dmaproxy_data->src_attach);
err_put_src:
	dma_buf_put(dmaproxy_data->src_dbuf);
	return -EINVAL;
}

static void dmabuf_cleanup(struct dmaproxy_data *dmaproxy_data,
			   dmaproxy_arg_t dmaproxy)
{
	dma_buf_unmap_attachment(dmaproxy_data->src_attach,
				 dmaproxy_data->src_sgt, DMA_BIDIRECTIONAL);
	dma_buf_detach(dmaproxy_data->src_dbuf, dmaproxy_data->src_attach);
	dma_buf_put(dmaproxy_data->src_dbuf);
	if (dmaproxy.dst_fd != dmaproxy.src_fd) {
		dma_buf_unmap_attachment(dmaproxy_data->dst_attach,
					 dmaproxy_data->dst_sgt,
					 DMA_BIDIRECTIONAL);
		dma_buf_detach(dmaproxy_data->dst_dbuf,
			       dmaproxy_data->dst_attach);
		dma_buf_put(dmaproxy_data->dst_dbuf);
	}
}

static int perform_dma_copy(struct dmaproxy_data *dmaproxy_data,
			    dmaproxy_arg_t dmaproxy)
{
	struct dma_device *dma_dev;
	struct dma_chan *chan;
	struct dma_async_tx_descriptor *tx = NULL;
	enum dma_status status;
	enum dma_ctrl_flags flags;
	dma_cookie_t cookie;
	long timeout;

	chan = dmaproxy_data->chan;
	dma_dev = dmaproxy_data->chan->device;

	/* src_buf/dst_buf are dma addresses produced by dma_buf_map_attachment()
	 * for this engine's device, so they are programmed as-is. They must not
	 * be handed to phys_to_page()/dma_map_page(): a dma address is not a
	 * physical address, the buffers are already mapped, and coherent memory
	 * may have no struct page or linear mapping at all.
	 */
	flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;
	tx = dma_dev->device_prep_dma_memcpy(chan, dmaproxy_data->dst_buf,
					     dmaproxy_data->src_buf,
					     dmaproxy.size, flags);
	if (!tx) {
		pr_err("dmaproxy: device prep for dma memcpy get failed\n");
		return -ECANCELED;
	}

	dmaproxy_data->done.done = false;
	init_waitqueue_head(&dmaproxy_data->done.wait);
	tx->callback = dmacopy_callback;
	tx->callback_param = &dmaproxy_data->done;
	cookie = tx->tx_submit(tx);

	if (dma_submit_error(cookie)) {
		pr_err("dmaproxy: dma submit error\n");
		return -ECANCELED;
	}
	dma_async_issue_pending(chan);

	timeout = wait_event_timeout(dmaproxy_data->done.wait,
				     dmaproxy_data->done.done,
				     msecs_to_jiffies(DMAPROXY_COPY_TIMEOUT_MS));

	/* The engine must be stopped before the caller drops its references to
	 * the buffers, otherwise a late completion writes into freed memory.
	 */
	if (!timeout) {
		pr_err("dmaproxy: timeout error while dma copy\n");
		dmaengine_terminate_sync(chan);
		return -ETIME;
	}

	status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);
	if (status != DMA_COMPLETE) {
		pr_err("dmaproxy: dma copy get failed\n");
		dmaengine_terminate_sync(chan);
		return -ECANCELED;
	}

	return 0;
}

static long dmaproxy_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
	dmaproxy_arg_t dmaproxy;
	struct dmaproxy_data *dmaproxy_data = f->private_data;
	int ret;

	switch (cmd) {
	case DMAPROXY_COPY: {
		if (copy_from_user(&dmaproxy, (dmaproxy_arg_t *)arg,
				   sizeof(dmaproxy_arg_t))) {
			pr_err("dmaproxy: copy from user failed for DMAPROXY_COPY\n");
			return -EFAULT;
		}

		/* The transfer state lives in dmaproxy_data and the channel
		 * takes one descriptor at a time, so threads sharing this file
		 * must not overlap.
		 */
		mutex_lock(&dmaproxy_data->lock);

		if (dmabuf_get_address(dmaproxy_data, dmaproxy)) {
			mutex_unlock(&dmaproxy_data->lock);
			return -EINVAL;
		}

		ret = perform_dma_copy(dmaproxy_data, dmaproxy);

		dmabuf_cleanup(dmaproxy_data, dmaproxy);
		mutex_unlock(&dmaproxy_data->lock);
		break;
	}

	default:
		return -EINVAL;
	}

	return ret;
}

static const struct file_operations dmaproxy_fops = { .owner = THIS_MODULE,
						      .open = dmaproxy_open,
						      .release = dmaproxy_close,
						      .unlocked_ioctl =
							      dmaproxy_ioctl };

static int __init dma_proxy_init(void)
{
	int ret;
	struct device *dmaproxy_device;

	ret = alloc_chrdev_region(&dmaproxy_dev, 0, MINOR_CNT, "dmaproxy");
	if (ret < 0)
		return ret;

	cdev_init(&dmaproxy_cdev, &dmaproxy_fops);
	ret = cdev_add(&dmaproxy_cdev, dmaproxy_dev, MINOR_CNT);
	if (ret < 0) {
		unregister_chrdev_region(dmaproxy_dev, MINOR_CNT);
		return ret;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
	dmaproxy_cl = class_create("char");
#else
	dmaproxy_cl = class_create(THIS_MODULE, "char");
#endif
	if (IS_ERR(dmaproxy_cl)) {
		cdev_del(&dmaproxy_cdev);
		unregister_chrdev_region(dmaproxy_dev, MINOR_CNT);
		return PTR_ERR(dmaproxy_cl);
	}

	dmaproxy_device = device_create(dmaproxy_cl, NULL, dmaproxy_dev, NULL,
					"dmaproxy");
	if (IS_ERR(dmaproxy_device)) {
		class_destroy(dmaproxy_cl);
		cdev_del(&dmaproxy_cdev);
		unregister_chrdev_region(dmaproxy_dev, MINOR_CNT);
		return PTR_ERR(dmaproxy_device);
	}

	return 0;
}

static void __exit dma_proxy_exit(void)
{
	device_destroy(dmaproxy_cl, dmaproxy_dev);
	class_destroy(dmaproxy_cl);
	cdev_del(&dmaproxy_cdev);
	unregister_chrdev_region(dmaproxy_dev, MINOR_CNT);
}

module_init(dma_proxy_init);
module_exit(dma_proxy_exit);

MODULE_AUTHOR("Jeegar Patel <jeegar.patel@xilinx.com>");
MODULE_DESCRIPTION("Xilinx's zynqmp-dma Client Driver");
MODULE_LICENSE("GPL v2");
