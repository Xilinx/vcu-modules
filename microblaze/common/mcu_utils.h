/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef _MCU_UTILS_H_
#define _MCU_UTILS_H_

#include <linux/types.h>
#include <linux/io.h>

int memcpy_toio_32(void *pdst, const void *psrc, int size);
int memcpy_fromio_32(void *pdst, void *psrc, int size);

#endif /* _MCU_UTILS_H_ */
