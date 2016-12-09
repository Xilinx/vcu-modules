/*
 * al_user.c high level function to write messages in the mailbox
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

#include "al_user.h"
#include "al_alloc.h"

static int mail_to_queue(int mail_uid)
{
	switch (mail_uid) {
	case AL_MCU_MSG_INIT:
		return AL5_USER_MAIL_INIT;
	case AL_MCU_MSG_CREATE_CHANNEL:
		return AL5_USER_MAIL_CREATE;
	case AL_MCU_MSG_DESTROY_CHANNEL:
		return AL5_USER_MAIL_DESTROY;
	case AL_MCU_MSG_ENCODE_ONE_FRM:
	case AL_MCU_MSG_DECODE_ONE_FRM:
		return AL5_USER_MAIL_STATUS;
	case AL_MCU_MSG_SEARCH_START_CODE:
		return AL5_USER_MAIL_SC;
	default:
		return AL5_USER_MAIL_DEBUG;
	}
}

static int send_msg(struct mcu_mailbox_interface *mcu, struct al5_mail *mail)
{
	int err = al5_mcu_send(mcu, mail);

	if (err) {
		pr_err("message sent is too big\n");
		return err;
	}
	al5_signal_mcu(mcu);

	return 0;
}

int al5_create_and_send(struct al5_user *user, struct msg_info *info,
		    struct al5_mail *(*create)(struct msg_info *))
{
	struct al5_mail *mail = NULL;
	int err;

	mail = create(info);
	if (!mail)
		return -ENOMEM;
	err = send_msg(user->mcu, mail);
	al5_free_mail(mail);
	if (err)
		return err;

	return 0;
}
EXPORT_SYMBOL_GPL(al5_create_and_send);

void al5_user_deliver(struct al5_user *user, struct al5_mail *mail)
{
	struct al5_queue* queue = &user->queues[mail_to_queue(mail->msg_uid)];
	al5_queue_push(queue, mail);
}

static void user_queues_unlock(struct al5_user *user)
{
	al5_queue_unlock(&user->queues[AL5_USER_MAIL_STATUS]);
	al5_queue_unlock(&user->queues[AL5_USER_MAIL_SC]);
	al5_queue_unlock(&user->queues[AL5_USER_MAIL_CREATE]);
}

static void user_queues_lock(struct al5_user *user)
{
	al5_queue_lock(&user->queues[AL5_USER_MAIL_STATUS]);
	al5_queue_lock(&user->queues[AL5_USER_MAIL_SC]);
	al5_queue_lock(&user->queues[AL5_USER_MAIL_CREATE]);
}

void al5_user_init(struct al5_user *user, u32 uid, struct mcu_mailbox_interface *mcu)
{
	int i;
	for (i = 0; i < AL5_USER_OPS_NUMBER; ++i) {
		mutex_init(&user->locks[i]);
	}
	for (i = 0; i < AL5_USER_MAIL_NUMBER; ++i) {
		al5_queue_init(&user->queues[i]);
	}
	user->uid = uid;
	user->mcu = mcu;
	user->chan_uid = BAD_CHAN;
}
EXPORT_SYMBOL_GPL(al5_user_init);


int al5_user_destroy_channel(struct al5_user *user, int quiet)
{
	int err = 0;
	int i, j;
	struct msg_info info;
	struct al5_mail *mail;

	if (!al5_chan_is_created(user))
		return -EINVAL;

	user_queues_unlock(user);
	for (i = 0; i < AL5_USER_OPS_NUMBER; ++i) {
		err = mutex_lock_killable(&user->locks[i]);
		if (err == -EINTR) {
			for (j = 0; j < i; ++j) {
				mutex_unlock(&user->locks[j]);
			}

			goto relock_user_queues;
		}

	}

	info.chan_uid = user->chan_uid;
	if (quiet) {
		al5_create_and_send(user, &info, create_quiet_destroy_channel_msg);
		if (err)
			goto unlock_mutexes;
	} else {
		err = al5_create_and_send(user, &info, create_destroy_channel_msg);
		if (err)
			goto unlock_mutexes;

		mail = al5_queue_pop(&user->queues[AL5_USER_MAIL_DESTROY]);
		if (mail == NULL)
			goto unlock_mutexes;

		al5_free_mail(mail);
	}
	user->chan_uid = BAD_CHAN;

unlock_mutexes:
	for (i = 0; i < AL5_USER_OPS_NUMBER; ++i) {
		mutex_unlock(&user->locks[i]);
	}
relock_user_queues:
	user_queues_lock(user);
	return err;

}
EXPORT_SYMBOL_GPL(al5_user_destroy_channel);

int al5_chan_is_created(struct al5_user *user)
{
	return (user->chan_uid != BAD_CHAN);
}
EXPORT_SYMBOL_GPL(al5_chan_is_created);
