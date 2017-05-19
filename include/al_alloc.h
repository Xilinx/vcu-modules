/*
 * al_alloc.h dma buffers allocation and dmabuf support
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

#ifndef _AL_ALLOC_H_
#define _AL_ALLOC_H_

#include <linux/device.h>
#include "al_ioctl.h"
#include "al_dmabuf.h"

struct al5_buffer_info {
	u32 bus_address;
	u32 size;
};

int al5_allocate_dmabuf(struct device *dev, int size, u32 *fd);
int al5_dmabuf_get_address(struct device *dev, u32 fd, u32 *bus_address);
int al5_get_dmabuf_info(struct device *dev, u32 fd, struct al5_buffer_info *info);

struct al5_dma_buffer *al5_alloc_dma(struct device *dev, size_t size);
void al5_free_dma(struct device *dev, struct al5_dma_buffer *buf);

#endif /* _AL_ALLOC_H_ */
