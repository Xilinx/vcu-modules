/*
 * al_codec.h per device operation. functions to change the state of the vcu
 * and functions to handle channels created by the user.
 *
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

#ifndef _AL_CODEC_H_
#define _AL_CODEC_H_

#include <linux/dma-buf.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/firmware.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>

#include "al_group.h"
#include "mcu_interface.h"
#include "al_alloc.h"
#include "al_user.h"
#include "al_vcu.h"
#include "al_traces.h"

#define al5_writel(val, reg) iowrite32(val, codec->regs + reg)
#define al5_readl(reg) ioread32(codec->regs + reg)

#define al5_dbg(format, ...) \
	dev_dbg(codec->device, format, ## __VA_ARGS__)

#define al5_info(format, ...) \
	dev_info(codec->device, format, ## __VA_ARGS__)

#define al5_err(format, ...) \
	dev_err(codec->device, format, ## __VA_ARGS__)

#define al5_warn_ratelimited(format, ...) \
	dev_warn_ratelimited(codec->device, format, ## __VA_ARGS__)

/* MCU Mailbox */
#define MAILBOX_CMD                     0x7000
#define MAILBOX_STATUS                  0x7800
#define MAILBOX_SIZE                    (0x800 - 0x8)

/* MCU Bootloader */
#define AL5_MCU_INTERRUPT_HANDLER       0x0010
#define MCU_SLEEP_INSTRUCTION           0xba020004 /* Instruction "mbar 16" */

/* MCU Control Regs */
#define AL5_MCU_RESET                   0x9000
#define AL5_MCU_RESET_MODE              0x9004
#define AL5_MCU_STA                     0x9008
#define AL5_MCU_WAKEUP                  0x900c

#define AL5_ICACHE_ADDR_OFFSET_MSB      0x9010
#define AL5_ICACHE_ADDR_OFFSET_LSB      0x9014
#define AL5_DCACHE_ADDR_OFFSET_MSB      0x9018
#define AL5_DCACHE_ADDR_OFFSET_LSB      0x901c

#define AL5_MCU_INTERRUPT               0x9100
#define AL5_MCU_INTERRUPT_MASK          0x9104
#define AL5_MCU_INTERRUPT_CLR           0x9108

#define AXI_ADDR_OFFSET_IP              0x9208

/* MCU Cache */
#define MCU_SUBALLOCATOR_SIZE           (1024 * 1024 * 32)      /* 32 MB */
#define MCU_CACHE_OFFSET                0x80000000
#define AL5_ICACHE_SIZE                 (1024 * 600)            /* 600 KB (for possible extensions) */
#define MCU_SRAM_SIZE                   0x8000                  /* 32 kB */

struct al5_codec_desc {
	struct device *device;
	struct cdev cdev;

	/* Base addr for regs */
	void __iomem *regs;
	unsigned long regs_size;

	/* cache */
	struct al5_dma_buffer *icache, *suballoc_buf;
	dma_addr_t dcache_base_addr;

	struct al5_group users_group;
	int minor;
};

struct al5_filp_data {
	struct al5_codec_desc *codec;
	struct al5_user *user;
};

int al5_codec_set_up(struct al5_codec_desc *codec,
		     struct platform_device *pdev,
		     size_t max_users_nb);

void al5_codec_tear_down(struct al5_codec_desc *codec);

int al5_codec_set_firmware(struct al5_codec_desc *codec, char *fw_file,
			   char *bl_fw_file);

int al5_codec_open(struct inode *inode, struct file *filp);
int al5_codec_release(struct inode *inode, struct file *filp);

long al5_codec_compat_ioctl(struct file *file, unsigned int cmd,
			    unsigned long arg);
#endif /* _AL_CODEC_H_ */
