/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef _AL_ENC_IOCTL_H_
#define _AL_ENC_IOCTL_H_

#include <linux/types.h>
#include <linux/ioctl.h>
#include "al_ioctl.h"

#define AL_MCU_CONFIG_CHANNEL _IOWR('q', 2, struct al5_config_channel)
#define AL_MCU_DESTROY_CHANNEL _IO('q', 4)
#define AL_MCU_ENCODE_ONE_FRM _IOWR('q', 5, struct al5_encode_msg)
#define AL_MCU_WAIT_FOR_STATUS _IOWR('q', 6, struct al5_params)
#define AL_MCU_PUT_STREAM_BUFFER _IOWR('q', 22, struct al5_buffer)
#define AL_MCU_GET_REC_PICTURE _IOWR('q', 23, struct al5_reconstructed_info)
#define AL_MCU_RELEASE_REC_PICTURE _IOWR('q', 24, __s32)
#define AL_MCU_GET _IOWR('q', 27, struct al5_params)
#define AL_MCU_SET _IOWR('q', 28, struct al5_params)

struct al5_reconstructed_info {
	__s32 fd;
	__u32 pic_struct;
	__u32 poc;
	__u32 width;
	__u32 height;
};

struct al5_config_channel {
	struct al5_params param;
	struct al5_channel_status status;
	__s32 rc_plugin_fd;
};

struct al5_encode_msg {
	struct al5_params params;
	struct al5_params addresses;
};

struct al5_stream_buffer {
	__u64 stream_buffer_ptr;
	__s32 handle;
	__u32 offset;
	__u32 size;
};

struct al5_buffer {
	struct al5_stream_buffer stream_buffer;
	__s32 external_mv_handle;
};

#endif /* _AL_ENC_IOCTL_H_ */
