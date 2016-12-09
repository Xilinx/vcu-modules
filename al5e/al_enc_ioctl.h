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

#define AL_MCU_CONFIG_CHANNEL  _IOWR('q', 2, struct al5_channel_param)
#define AL_MCU_DESTROY_CHANNEL  _IO('q', 4)
#define AL_MCU_ENCODE_ONE_FRM _IOWR('q', 5, struct al5_encode_msg)
#define AL_MCU_WAIT_FOR_STATUS _IOWR('q', 6, struct al5_pic_status)

#define MAIL_TESTS _IO('q', 7)

#define AL_MCU_SET_TIMER_BUFFER _IOWR('q', 16, struct al5_dma_info)

#define AL_MAX_ROWS_TILE       34
#define AL_NUM_CORE            4
#define MAX_SEGMENTS           8

struct al5_hw_rate_ctrl_param {
	__u32 load_tables;
	__u32 load_ctx;
	__u32 store_ctx;

	__u32 prevent_ent_ofw;
	__u32 prevent_fifo_ofw;

	__u32 ld_str_buf_lvl;
	__u32 reserved1;
	__u32 reserved2;
	__u32 buffer_size;
	__u32 buffer_decr;

	__u32 avg_group_size;

	__u16 lcu_per_group;
	__u16 group_per_row;

	__u8 min_qp;
	__u8 max_qp;
	__u8 min_qpsc;
	__u8 QPSC;

	__u32 force_sc;
	__u32 dis_scdetect;
	__u32 clow_limit;
	__u16 cthreshold;

	__u16 limit_res;

	__u16 first_group;
	__u16 last_group;
	__u32 num_groups;
	__u32 target_size;

	__u8 sync_left_mode;
	__u8 sync_right_mode;
	__u8 core_ch_order;

} __aligned (4);

struct al5_rcparam {
	__u32 rcmode;
	__u32 initial_rem_delay;
	__u32 cpbsize;
	__u16 frame_rate;
	__u16 clk_ratio;
	__u32 target_bit_rate;
	__u32 max_bit_rate;
	__s16 initial_qp;
	__s16 min_qp;
	__s16 max_qp;
	__s16 ipdelta;
	__s16 pbdelta;
	__u32 options;
} __aligned (4);

struct al5_gop_param {
	__u32 mode;
	__u32 freq_idr;
	__u32 freq_lt;
	__u32 gdr_mode;
	__u16 gop_length;
	union {
		struct {
			__u8 num_b;
		};
		struct {
			__u8 num_frm;
			__u32 frms;
		};
	};
} __aligned (4);

struct al5_channel_param {
	__u16 width;
	__u16 height;
	__u32 pic_format;
	__u32 color_space;

	__u32 profile;
	__u8 level;
	__u8 tier;

	__u32 hls_param;

	__u32 options;
	__s8 beta_offset;
	__s8 tc_offset;

	__s8 cb_slice_qp_offset;
	__s8 cr_slice_qp_offset;
	__s8 cb_pic_qp_offset;
	__s8 cr_pic_qp_offset;

	__u8 cu_qpdelta_depth;
	__u8 cabac_init_idc;

	__u8 num_core;
	__u16 slice_size;
	__u16 num_slices;

	__u32 l2cache_auto;
	__u32 l2cache_mem_offset;
	__u32 l2cache_mem_size;
	__u16 clip_hrz_range;
	__u16 clip_vrt_range;
	__s16 me_range[2][2];

	__u8 max_cu_size;
	__u8 min_cu_size;
	__u8 max_tu_size;
	__u8 min_tu_size;
	__u8 max_transfo_depth_intra;
	__u8 max_transfo_depth_inter;

	struct al5_hw_rate_ctrl_param hw_rc;
	__u32 entropy_mode;
	__u32 wpmode;

	__u32 max_burst_size;

	struct al5_rcparam rcparam;
	struct al5_gop_param gop_param;

} __aligned (4);

struct al5_pic_param {
	__s16 qpoffset;
	__u8 wpidc;
	__u8 me_mode;
	__u32 type;
	__u32 pic_struct;
	__u32 is_idr;
	__u32 is_ref;
	__u32 scene_change;

	__u32 enc_options;

	__s16 pps_qp;
	__u8 ref_pic_set_idx;

	__u8 num_ref_idx_l0;
	__u8 num_ref_idx_l1;

	__u8 ref_idx_a;
	__u8 ref_idx_b;
	__u8 ref_idx_f;
	__u32 ref_mode_a;
	__u32 ref_mode_b;
	__u32 ref_mode_f;
	__u32 merge_ltl0;
	__u32 merge_ltl1;
	__s32 current_poc;
	__s32 ref_apoc;
	__s32 ref_bpoc;
	__s32 ref_fpoc;
	__s32 ltpic_num;
	__s32 rm_pic_num;
	__u32 rm_lt;
	__s32 coloc_poc;
	__s32 coloc_ref_apoc;
	__s32 coloc_ref_bpoc;

	__u32 no_backward_pred_flag;
	__u32 ref_coloc_l0flag;
	__u32 ref_coloc_ltflag;
	union {
		__u32 non_vcl_hdr_size;
		__u32 hdr_offset;
	};

	__s32 frame_num;
	__s32 last_idr_id;

	__u32 gdr_mode;
	__u16 gdr_pos;
	__u32 user_param;
} __aligned (4);

struct al5_pic_buffer_addrs {
	__u32 src_y;
	__u32 src_uv;
	__u32 ref_ay;
	__u32 ref_auv;
	__u32 ref_amap;
	__u32 ref_by;
	__u32 ref_buv;
	__u32 ref_bmap;
	__u32 rec_y;
	__u32 rec_uv;
	__u32 rec_map;
	__u32 coloc;
	__u32 mv;
	__u32 wpp;
	__u32 map_out;
	__u32 data_out;
	__u32 ep1;
	__u32 ep2;
	__u32 ep3;
	__u32 stream;
	__u32 str_part;
	__u32 max_size;
	__u32 offset;
	__u32 avail_size;
	__u32 map_in;
	__u32 data_in;
	__u32 pitch_src;
	__u32 pitch_rec;

	__u32 streamv;
	__u32 str_partv;
	__u32 wppv;
	__u32 ep2v;

} __aligned (4);

struct al5_pic_status
{
	__u32 skip;
	__u32 initial_removal_delay;
	__u32 dpb_output_delay;
	__u32 size;
	__u32 frm_tag_size;
	__s32 stuffing;
	__s32 filler;
	__s32 num_clmn;
	__s32 num_row;
	__s32 qp;

	__u32 user_param;

	__s32 num_parts;

	__u32 sum_cplx;

	__s32 tile_width[AL_NUM_CORE];
	__s32 tile_height[AL_MAX_ROWS_TILE];

	__u32 error_code;

	__u32 num_group;
} __aligned (4);

struct al5_encode_msg {
	struct al5_pic_param pic_param;
	struct al5_pic_buffer_addrs buffers_addrs;
} __aligned(4);

#endif	/* _AL_ENC_IOCTL_H_ */
