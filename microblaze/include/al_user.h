/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef _AL_USER_H_
#define _AL_USER_H_

#include <linux/mutex.h>
#include <linux/device.h>

#include "al_ioctl.h"
#include "al_mail.h"
#include "al_queue.h"
#include "mcu_interface.h"
#include "al_buffers_pool.h"

enum user_mail {
	AL5_USER_MAIL_INIT,
	AL5_USER_MAIL_CREATE,
	AL5_USER_MAIL_DESTROY,
	AL5_USER_MAIL_STATUS,
	AL5_USER_MAIL_SC,
	AL5_USER_MAIL_DEBUG,
	AL5_USER_MAIL_REC,
	AL5_USER_MAIL_GET,
	AL5_USER_MAIL_SET,

	/* always the last one */
	AL5_USER_MAIL_NUMBER,
};

enum user_operations {
	AL5_USER_INIT,
	AL5_USER_CHANNEL,
	AL5_USER_STATUS,
	AL5_USER_SC,

	/* always the last one */
	AL5_USER_OPS_NUMBER
};

#define NO_CHECKPOINT -1
#define CHECKPOINT_ALLOCATE_BUFFERS 1
#define CHECKPOINT_SEND_INTERMEDIATE_BUFFERS 2
#define CHECKPOINT_SEND_REFERENCE_BUFFERS 3
#define CHECKPOINT_CREATED 4
#define CHECKPOINT_DESTROYED 5
#define BAD_CHAN 0xff

/* User structure
 * There is one User for each opening of the device file descriptor.
 * Being a user allow us to call ioctl's to communicate with mcu or ip
 * In order to perform encoding or decoding operations,
 * creating a channel is needed
 */

struct al5_user {
	/* the user id is used to identify the mail that are exchanged between the user
	   and the mcu before the creation of a channel */
	int uid;
	/* The chan_uid is used to identify the channel that should receive the mails
	   in the mcu and in the driver. In the driver it is used to select which user
	   should receive the current mail. */
	int chan_uid;

	int checkpoint;

	struct mcu_mailbox_interface *mcu;

	struct al5_queue queues[AL5_USER_MAIL_NUMBER];
	struct mutex locks[AL5_USER_OPS_NUMBER];

	struct device *device;
	struct al5_buffers_pool int_buffers;
	struct al5_buffers_pool rec_buffers;

	bool non_block;
};

void al5_user_init(struct al5_user *user, int uid,
		   struct mcu_mailbox_interface *mcu, struct device *device);
int al5_user_destroy_channel(struct al5_user *user, int quiet);
void al5_user_destroy_channel_resources(struct al5_user *user);
void al5_user_remove_residual_messages(struct al5_user *user);

int al5_check_and_send(struct al5_user *user, struct al5_mail *mail);

int al5_chan_is_created(struct al5_user *user);

int al5_have_checkpoint(struct al5_user *user);

int al5_user_deliver(struct al5_user *user, struct al5_mail *mail);

int al5_is_ready(struct al5_user *user, struct al5_mail **mail, int my_uid);

int al5_status_is_ready(struct al5_user *user, struct al5_mail **mail,
			int my_uid);

struct al5_mail *al5_user_get_mail(struct al5_user *user, u32 mail_uid);

#endif /* _AL_USER_H_ */
