/*
 * dec_mails_factory.c high level message creation for the mcu
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

#include <linux/device.h>
#include <linux/uaccess.h>
#include "al_mail.h"
#include "dec_mails_factory.h"

/* *msg is filled, *mail is read-only */
void al5d_mail_get_status(struct al5_pic_status *status, struct al5_mail *mail)
{
	memcpy(status, mail->body + 4, sizeof(*status));
}

/* *msg is filled, *mail is read-only */
void al5d_mail_get_sc_status(struct al5_scstatus *scstatus, struct al5_mail *mail)
{
	memcpy(scstatus, mail->body + 4, sizeof(*scstatus));
}

struct al5_mail *
al5d_create_decode_one_frame_msg(struct msg_info *info)
{
	struct al5_decode_msg *msg = (struct al5_decode_msg *)info->priv;
	struct al5_mail *mail;
	int channel_uid_size = 4;
	u32 mcu_chan_uid = info->chan_uid;
	size_t size = sizeof(*msg);

	mail = al5_create_msg(mcu_chan_uid, channel_uid_size,
			      AL_MCU_MSG_DECODE_ONE_FRM, size);
	if (!mail)
		return NULL;

	memcpy(mail->body + channel_uid_size, msg, sizeof(*msg));

	return mail;
}

struct al5_mail *al5d_create_channel_param_msg(struct msg_info *info)
{
	struct al5_channel_param *msg = (struct al5_channel_param *)info->priv;
	struct al5_mail *mail;
	u32 drv_chan_uid = info->chan_uid;
	int channel_uid_size = 4;
	size_t size = channel_uid_size + sizeof(struct al5_channel_param);

	mail = al5_create_msg(drv_chan_uid, channel_uid_size,
			      AL_MCU_MSG_CREATE_CHANNEL, size);
	if (!mail)
		return NULL;

	memcpy(mail->body + channel_uid_size, msg, sizeof(*msg));

	return mail;
}


struct al5_mail *
al5d_create_search_sc_mail(struct msg_info *info)
{
	struct al5_search_sc_msg *msg = (struct al5_search_sc_msg *)info->priv;
	u32 user_uid = info->chan_uid;
	struct al5_mail *mail;
	int user_uid_size = 4;
	size_t size = sizeof(*msg);

	mail = al5_create_msg(user_uid, user_uid_size,
			      AL_MCU_MSG_SEARCH_START_CODE, size);
	if (!mail)
		return NULL;

	memcpy(mail->body + user_uid_size, msg,	sizeof(*msg));

	return mail;
}
