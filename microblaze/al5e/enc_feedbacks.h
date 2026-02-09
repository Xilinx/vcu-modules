/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef __AL_FEEDBACKS__
#define __AL_FEEDBACKS__

#include <linux/types.h>

struct al5_channel_buffers {
	__s32 int_buffers_count;
	__s32 int_buffers_size;
	__s32 rec_buffers_count;
	__s32 rec_buffers_size;
	__s32 reserved;
};

struct al5e_feedback_channel {
	u32 chan_uid;
	u32 user_uid;
	struct al5_channel_buffers buffers_needed;
	u32 error_code;
};

#endif
