/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef _AL_TRACES_H_
#define _AL_TRACES_H_

#include <linux/printk.h>

#define AL5_DEBUG 0
#if AL5_DEBUG
#define mails_info(format, ...)                 \
	do {                                    \
		pr_info("\e[0;32m");            \
		pr_cont(format, ##__VA_ARGS__); \
		pr_cont("\e[0m\n");             \
	} while (0)
#define ioctl_info(format, ...)                 \
	do {                                    \
		pr_info("\e[0;33m");            \
		pr_cont(format, ##__VA_ARGS__); \
		pr_cont("\e[0m\n");             \
	} while (0)

#define irq_info(format, ...)                   \
	do {                                    \
		pr_info("\e[0;34m");            \
		pr_cont(format, ##__VA_ARGS__); \
		pr_cont("\e[0m\n");             \
	} while (0)

#define setup_info(format, ...) dev_info(codec->device, format, ##__VA_ARGS__)
#else

#define mails_info(format, ...)
#define ioctl_info(format, ...)
#define irq_info(format, ...)
#define setup_info(format, ...)

#endif

#define mcu_info(format, ...)                   \
	do {                                    \
		pr_info("\e[0;31m");            \
		pr_cont(format, ##__VA_ARGS__); \
		pr_cont("\e[0m\n");             \
	} while (0)

#endif
