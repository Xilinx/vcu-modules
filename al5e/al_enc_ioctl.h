/*
 * al_enc_ioctl.h structure used to communicate with the userspace
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

#ifndef _AL_ENC_IOCTL_H_
#define _AL_ENC_IOCTL_H_
#include "al_ioctl.h"
#include <linux/types.h>

#define AL_MCU_CONFIG_CHANNEL  _IOWR('q', 2, struct al5_config_channel)
#define AL_MCU_DESTROY_CHANNEL  _IO('q', 4)
#define AL_MCU_ENCODE_ONE_FRM _IOWR('q', 5, struct al5_encode_msg)
#define AL_MCU_WAIT_FOR_STATUS _IOWR('q', 6, struct al5_params)

#define MAIL_TESTS _IO('q', 7)
#define AL_MCU_SET_TIMER_BUFFER _IOWR('q', 16, struct al5_dma_info)
#define AL_MCU_PUT_STREAM_BUFFER _IOWR('q', 22, struct al5_buffer)

#define AL_MCU_GET_REC_PICTURE _IOWR('q', 23, struct al5_reconstructed_info)
#define AL_MCU_RELEASE_REC_PICTURE _IOWR('q', 24, __u32)


#define AL_MAX_ROWS_TILE       22
#define AL_NUM_CORE            4

struct al5_reconstructed_info {
	__u32 fd;
	__u32 pic_struct;
	__u32 poc;
};

struct al5_params {
	__u32 size;
	__u32 opaque_params[128];
};

struct al5_channel_status {
	__u32 options;
	__u8 num_core;
	__u32 pps_param;
	__u32 error_code;
};

struct al5_config_channel {
	struct al5_params param;
	struct al5_channel_status status;
};

struct al5_enc_info {
	__u32 enc_options;
	__s16 pps_qp;
	__u64 user_param;
	__u64 src_handle;
};

struct al5_enc_pic_buf_addrs {
	__u32 src_y;
	__u32 src_uv;
	__u32 pitch_src;

	__u32 ep2;
	__u32 ep2v;
};

struct al5_enc_request_info {
	__u32 req_options;
	__u32 scene_change_delay;
};

struct al5_encode_msg {
	__u32 represents_end_of_stream;
	__u32 padding;
	struct al5_enc_info enc_info;
	struct al5_enc_request_info req_info;
	struct al5_enc_pic_buf_addrs buffers_addrs;
};

struct al5_buffer {
	__u64 stream_buffer_ptr;
	__u32 handle;
	__u32 offset;
	__u32 size;
};

#endif  /* _AL_ENC_IOCTL_H_ */
