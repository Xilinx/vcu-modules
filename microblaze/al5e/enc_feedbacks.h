/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef __AL_FEEDBACKS__
#define __AL_FEEDBACKS__

#include <linux/types.h>

struct al5_channel_buffers {
	int32_t int_buffers_count;
	int32_t int_buffers_size;
	int32_t rec_buffers_count;
	int32_t rec_buffers_size;
	int32_t reserved;
};

struct al5e_feedback_channel {
	u32 chan_uid;
	u32 user_uid;
	struct al5_channel_buffers buffers_needed;
	u32 error_code;
};

#endif
