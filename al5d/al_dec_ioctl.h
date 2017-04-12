/*
 * al_dec_ioctl.h structure used to communicate with the userspace
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

#ifndef _AL_DEC_IOCTL_H_
#define _AL_DEC_IOCTL_H_

#include <linux/types.h>
#include "al_ioctl.h"

#define AL_MCU_CONFIG_CHANNEL  _IOWR('q', 2, struct al5_channel_param)
#define AL_MCU_DESTROY_CHANNEL  _IO('q', 4)
#define AL_MCU_DECODE_ONE_FRM _IOWR('q', 5, struct al5_decode_msg)
#define AL_MCU_WAIT_FOR_STATUS _IOWR('q', 6, struct al5_pic_status)
#define MAIL_TESTS _IO('q', 7)
#define AL_MCU_SEARCH_START_CODE _IOWR('q', 8, struct al5_search_sc_msg)
#define AL_MCU_WAIT_FOR_START_CODE _IOWR('q', 9, struct al5_scstatus)
#define AL_MCU_SET_TIMER_BUFFER _IOWR('q', 16, struct al5_dma_info)
#define AL_MCU_SET_IRQ_TIMER_BUFFER _IOWR('q', 17, struct al5_dma_info)

#define MAX_ROWS_TILE       22
#define MAX_COLUMNS_TILE    20

struct al5_scparam
{
	__u32 AVC;
	__u8 stop_param;
	__u8 stop_cond_idc;
	__u16 max_size;
} __aligned (4);

struct al5_sc_buffer_addrs
{
	__u32 stream;
	__u32 max_size;
	__u32 offset;
	__u32 avail_size;

	__u32 buf_out;
} __aligned (4);

struct al5_sctable
{
	__u32 position;
	__u8 nut;
	__u8 temporal_id;
	__u16 reserved;
} __aligned (4);

struct al5_scstatus
{
	__u16 num_sc;
	__u32 num_bytes;
} __aligned (4);

struct al5_channel_param
{
	__s32 width;
	__s32 height;
	__u32 frame_rate;
	__u32 clk_ratio;
	__u32 max_latency;
	__u8 num_core;
	__u8 ddrwidth;
	__u32 low_lat;
	__u32 parallel_wpp;
	__u32 disable_cache;
	__u32 codec;
} __aligned (4);

struct al5_pict_param
{
	__u8 codec;
	__u8 frm_id;
	__u8 max_transfo_depth_intra;
	__u8 max_transfo_depth_inter;
	__u8 min_tusize;
	__u8 max_tusize;
	__u8 max_tuskip_size;
	__u8 max_cusize;
	__u8 pcm_bit_depth_y;
	__u8 pcm_bit_depth_c;
	__u8 bit_depth_luma;
	__u8 bit_depth_chroma;
	__u8 chroma_qp_offset_depth;
	__u8 qp_off_lst_size;
	__u8 parallel_merge;
	__u8 coloc_pic_id;
	__u8 bit_depth_y;
	__u8 bit_depth_c;
	__u8 max_bit_depth;

	__s8 pic_cb_qp_offset;
	__s8 pic_cr_qp_offset;
	__s8 cb_qp_off_lst[6];
	__s8 cr_qp_off_lst[6];
	__s8 delta_qpcudepth;
	__s8 min_pcmsize;
	__s8 max_pcmsize;
	__s8 min_cusize;

	__u16 pic_width;
	__u16 pic_height;
	__u16 lcu_width;
	__u16 lcu_height;
	__u16 columnwidth[MAX_COLUMNS_TILE];
	__u16 rowheight[MAX_ROWS_TILE];
	__u16 numtilecolumns;
	__u16 numtilerows;
	__u16 numslice;

	__s32 current_poc;

	__u32 option_flags;

	__u32 chroma_mode;
	__u32 ent_mode;

	__s32 frm_num;
	__u32 user_param;

} __aligned (4);

struct al5_pict_buffer_addrs
{
	__u32 comp_data;
	__u32 comp_map;
	__u32 list_ref;
	__u32 stream;
	__u32 stream_size;
	__u32 rec_y;
	__u32 rec_c;
	__u32 pitch;
	__u32 scl;
	__u32 poc;
	__u32 mv;
	__u32 wp;
} __aligned (4);

struct al5_pic_status
{
	__u8 frm_id;

	__u32 num_lcu;
	__u32 num_bytes;
	__u32 num_bins;
	__u32 bandwidth;
	__u32 crc;
	__u32 conceal;
	__u32 bhanged;
} __aligned (4);

struct al5_decode_msg
{
	struct al5_pict_param pict_param;
	struct al5_pict_buffer_addrs pict_buffer_addrs;
	__u32 slice_param_v;
} __aligned (4);

struct al5_search_sc_msg
{
	struct al5_scparam param;
	struct al5_sc_buffer_addrs buffer_addrs;
} __aligned (4);

#endif	/* _AL_DEC_IOCTL_H_ */
