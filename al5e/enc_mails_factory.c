/*
 * enc_mails_factory.c high level message creation for the mcu
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

#include <linux/uaccess.h>

#include "enc_mails_factory.h"

u16 get_u16(u8 **data)
{
	u16 result = 0;
	int i = 0;

	for (i = 0; i < 2; ++i)
		result += ((*data)[i] << (i * 8));
	*data += 2;

	return result;
}

u32 get_u32(u8 **data)
{
	u32 result = 0;
	int i = 0;

	for (i = 0; i < 4; ++i)
		result += ((*data)[i] << (i * 8));
	*data += 4;

	return result;
}

s32 get_s32(u8 **data)
{
	return get_u32(data);
}

int get_int(u8 **data)
{
	return get_u32(data);
}

/* *msg is filled, *mail is read-only */
void al5e_mail_get_status(struct al5_pic_status *status, struct al5_mail *mail)
{
	u8 *msg_iterator = mail->body;
	int i;

	msg_iterator += 4; /* skip chan_uid */

	status->skip = get_u32(&msg_iterator);
	status->initial_removal_delay = get_u32(&msg_iterator);
	status->dpb_output_delay = get_u32(&msg_iterator);
	status->size = get_u32(&msg_iterator);
	status->frm_tag_size = get_u32(&msg_iterator);
	status->stuffing = get_s32(&msg_iterator);
	status->filler   = get_s32(&msg_iterator);
	status->num_clmn  = get_s32(&msg_iterator);
	status->num_row  = get_s32(&msg_iterator);
	status->qp  = get_int(&msg_iterator);
	status->user_param = get_u32(&msg_iterator);
	status->num_parts  = get_s32(&msg_iterator);

	status->sum_cplx = get_u32(&msg_iterator);
	for (i = 0; i < AL_NUM_CORE; ++i)
		status->tile_width[i] = get_s32(&msg_iterator);
	for (i = 0; i < AL_MAX_ROWS_TILE; ++i)
		status->tile_height[i] = get_s32(&msg_iterator);
	status->error_code = get_u32(&msg_iterator);
	status->num_group = get_u32(&msg_iterator);
}

struct al5_mail *al5e_create_encode_one_frame_msg(struct msg_info *info)
{
	struct al5_encode_msg *msg = (struct al5_encode_msg *)info->priv;
	u32 mcu_chan_uid = info->chan_uid;
	struct al5_mail *mail;
	int channel_uid_size = 4;
	size_t size = sizeof(struct al5_pic_param)
		    + sizeof(struct al5_pic_buffer_addrs);

	mail = al5_create_msg(mcu_chan_uid, channel_uid_size,
			      AL_MCU_MSG_ENCODE_ONE_FRM, size);

	if (!mail)
		return NULL;

	memcpy(mail->body, &mcu_chan_uid, channel_uid_size);
	memcpy(mail->body + channel_uid_size, msg, sizeof(*msg));

	return mail;
}


struct al5_mail *al5e_create_channel_param_msg(struct msg_info *info)
{
	struct al5_channel_param *msg = (struct al5_channel_param *)info->priv;
	u32 drv_chan_uid = info->chan_uid;
	struct al5_mail *mail;
	int channel_uid_size = 4;
	size_t size = sizeof(struct al5_channel_param);

	mail = al5_create_msg(drv_chan_uid, channel_uid_size,
			      AL_MCU_MSG_CREATE_CHANNEL, size);
	if (!mail)
		return NULL;

	memcpy(mail->body + channel_uid_size, msg, size);

	return mail;
}



