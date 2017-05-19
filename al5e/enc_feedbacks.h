#ifndef __AL_FEEDBACKS__
#define __AL_FEEDBACKS__

#include <linux/types.h>

struct al5_channel_buffers {
	__u32 int_buffers_count;
	__u32 int_buffers_size;
	__u32 rec_buffers_count;
	__u32 rec_buffers_size;
	__u32 reserved;
};

struct al5e_feedback_channel
{
	u32 chan_uid;
	u32 user_uid;
	u32 options;
	u32 num_core;
	u32 pps_param;
	struct al5_channel_buffers buffers_needed;
};

#endif
