/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef L2_PREFETCH_H
#define L2_PREFETCH_H

#include <linux/types.h>

u32 get_l2_size_in_bits(void *);
u32 get_l2_color_bitdepth(void *);
u32 get_num_cores(void *);
u32 get_core_frequency(void *);

#endif // L2_PREFETCH_H
