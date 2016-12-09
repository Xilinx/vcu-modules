/*
 * al_mail.h create a message to send to the vcu
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

#ifndef _MCU_COMMON_H_
#define _MCU_COMMON_H_

#include "al_ioctl.h"

enum al5_mail_uid
{
	AL_MCU_MSG_INIT,
	AL_MCU_MSG_DEINIT,

	AL_MCU_MSG_IP_INT,

	AL_MCU_MSG_CPU_INT,
	AL_MCU_MSG_CPU_RSP,

	AL_MCU_MSG_CREATE_CHANNEL,
	AL_MCU_MSG_DESTROY_CHANNEL,

	AL_MCU_MSG_ENCODE_ONE_FRM,

	AL_MCU_MSG_DECODE_ONE_FRM,
	AL_MCU_MSG_SEARCH_START_CODE,

	AL_MCU_MSG_QUIET_DESTROY_CHANNEL,

	AL_MCU_MSG_GET_RESOURCE,
	AL_MCU_MSG_SET_BUFFER,

	AL_MCU_MSG_SHUTDOWN,

	/* NSFProd */
	AL_MCU_MSG_TRACE,
	AL_MCU_MSG_RANDOM_TEST,
	AL_MCU_MSG_SET_TIMER_BUFFER,
	AL_MCU_MSG_SET_IRQ_TIMER_BUFFER,

};

struct al5_mail {
	u16 msg_uid;
	u16 body_size;
	u8 *body;
};

struct msg_info {
	u32 chan_uid;
	void *priv;
};

struct buffer_msg {
	u32 addr;
	u32 size;
};

struct al5_mail* create_init_msg(struct msg_info *info);
struct al5_mail* create_destroy_channel_msg(struct msg_info *info);
struct al5_mail* create_quiet_destroy_channel_msg(struct msg_info *info);

struct al5_mail *al5_create_msg(u32 mcu_chan_uid, int channel_uid_size, u32 msg_uid, u32 size);
struct al5_mail *al5_create_empty_msg(u32 mcu_chan_uid, u32 msg_uid);

void al5_free_mail(struct al5_mail *m_data);

void al5_get_user_uid(u32 *user_uid, struct al5_mail *mail);
void al5_get_chan_uid(u32 *chan_uid, struct al5_mail *mail);

#endif /* _MCU_COMMON_H_ */
