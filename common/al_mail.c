/*
 * al_mail.c create a message to send to the vcu
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

#include <linux/slab.h>

#include "al_mail.h"
#include "al_ioctl.h"

struct al5_mail *create_init_msg(struct msg_info *info)

{
	struct buffer_msg *msg = (struct buffer_msg *) info->priv;
	struct al5_mail *mail;
	int channel_uid_size = 4;

	mail = al5_create_msg(info->chan_uid, channel_uid_size,
			AL_MCU_MSG_INIT, sizeof(*msg));
	if (!mail)
		return NULL;

	memcpy(mail->body + channel_uid_size, msg, sizeof(*msg));

	return mail;
}

struct al5_mail *create_destroy_channel_msg(struct msg_info *info)

{
	return al5_create_empty_msg(info->chan_uid, AL_MCU_MSG_DESTROY_CHANNEL);
}

struct al5_mail *create_quiet_destroy_channel_msg(struct msg_info *info)
{
	return al5_create_empty_msg(info->chan_uid, AL_MCU_MSG_QUIET_DESTROY_CHANNEL);
}

struct al5_mail *al5_create_msg(u32 mcu_chan_uid, int channel_uid_size, u32 msg_uid, u32 size)
{
	struct al5_mail *mail;

	mail = kmalloc(sizeof(struct al5_mail), GFP_KERNEL);
	if (!mail) {
		return NULL;
	}
	mail->msg_uid = msg_uid;
	mail->body_size = channel_uid_size + size;
	mail->body = kmalloc(mail->body_size, GFP_KERNEL);
	if (!mail->body) {
		kfree(mail);
		return NULL;
	}

	memcpy(mail->body, &mcu_chan_uid, channel_uid_size);

	return mail;
}
EXPORT_SYMBOL_GPL(al5_create_msg);

struct al5_mail *al5_create_empty_msg(u32 mcu_chan_uid, u32 msg_uid)
{
	return al5_create_msg(mcu_chan_uid, 4, msg_uid, 0);
}
EXPORT_SYMBOL_GPL(al5_create_empty_msg);

void al5_free_mail(struct al5_mail *mail)
{
	if (mail != NULL) {
		kfree(mail->body);
		kfree(mail);
	}
}
EXPORT_SYMBOL_GPL(al5_free_mail);

void al5_get_user_uid(u32 *user_uid, struct al5_mail *mail)
{
	memcpy(user_uid, mail->body + 4, 4);
}
EXPORT_SYMBOL_GPL(al5_get_user_uid);

void al5_get_chan_uid(u32 *chan_uid, struct al5_mail *mail)
{
	memcpy(chan_uid, mail->body, 4);
}
EXPORT_SYMBOL_GPL(al5_get_chan_uid);

