/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#include "mcu_utils.h"
/* only 32 bits access are available on the apb */
int memcpy_toio_32(void *pdst, const void *psrc, int size)
{
	const __u32 *src = psrc;
	__u32 *dst = pdst;
	int i;

	for (i = 0; i < size / 4; ++i)
		iowrite32(src[i], dst + i);
	return 0;
}

int memcpy_fromio_32(void *pdst, void *psrc, int size)
{
	__u32 *src = psrc;
	__u32 *dst = pdst;
	int i;

	for (i = 0; i < size / 4; ++i)
		dst[i] = ioread32(src + i);
	return 0;
}
