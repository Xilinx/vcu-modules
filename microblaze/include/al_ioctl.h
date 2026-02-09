/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef _AL_IOCTL_H_
#define _AL_IOCTL_H_

#include <linux/types.h>
#include <linux/ioctl.h>

#define GET_DMA32_FD _IOWR('q', 13, struct al5_dma32_info)
#define GET_DMA32_PHY _IOWR('q', 18, struct al5_dma32_info)

struct al5_dma32_info {
	__s32 fd;
	__u32 size;
	__u32 phy_addr;
};

#define OPAQUE_SIZE 256

struct al5_params {
	__u32 size;
	__u32 opaque[OPAQUE_SIZE];
};

struct al5_channel_status {
	__u32 error_code;
};

#endif /* _AL_IOCTL_H_ */
