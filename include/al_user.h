/*
 * al_user.h high level function to write messages in the mailbox
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

#ifndef _AL_USER_H_
#define _AL_USER_H_

#include <linux/dma-buf.h>
#include <linux/cdev.h>

#include "al_ioctl.h"
#include "al_mail.h"
#include "al_queue.h"
#include "mcu_interface.h"

enum user_mail {
	AL5_USER_MAIL_INIT,
	AL5_USER_MAIL_CREATE,
	AL5_USER_MAIL_DESTROY,
	AL5_USER_MAIL_STATUS,
	AL5_USER_MAIL_SC,
	AL5_USER_MAIL_DEBUG,

	/* always the last one */
	AL5_USER_MAIL_NUMBER,
};

enum user_operations {
	AL5_USER_INIT,
	AL5_USER_CREATE,
	AL5_USER_DESTROY,
	AL5_USER_XCODE,
	AL5_USER_STATUS,
	AL5_USER_SC,
	AL5_USER_DEBUG,

	/* always the last one */
	AL5_USER_OPS_NUMBER
};

#define BAD_CHAN 0xff
/* User structure
 * There is one User for each opening of the device file descriptor.
 * Being a user allow us to call ioctl's to communicate with mcu or ip
 * In order to perform encoding or decoding operations, creating a channel is needed
 */
struct al5_user {

	u32 uid;
	u32 chan_uid;

	struct mcu_mailbox_interface *mcu;

	struct al5_queue queues[AL5_USER_MAIL_NUMBER];
	struct mutex locks[AL5_USER_OPS_NUMBER];
};

void al5_user_init(struct al5_user *user, u32 uid, struct mcu_mailbox_interface *mcu);
int al5_user_destroy_channel(struct al5_user *user, int quiet);

int al5_create_and_send(struct al5_user *user, struct msg_info *info,
				struct al5_mail *(*create)(struct msg_info *));

int al5_chan_is_created(struct al5_user *user);
void al5_user_deliver(struct al5_user *user, struct al5_mail *mail);


int al5_is_ready(struct al5_user *user, struct al5_mail **mail, int my_uid);
int al5_status_is_ready(struct al5_user *user, struct al5_mail **mail, int my_uid);


#endif /* _AL_USER_H_ */
