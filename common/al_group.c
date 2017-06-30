/*
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
#include <linux/printk.h>
#include <linux/string.h>

#include "al_group.h"
#include "al_mail.h"
#include "al_codec_mails.h"

void al5_group_init(struct al5_group *group, struct mcu_mailbox_interface *mcu,
		    int max_users_nb, struct device *device)
{
	group->users = kcalloc(max_users_nb, sizeof(void *), GFP_KERNEL);
	group->max_users_nb = max_users_nb;
	spin_lock_init(&group->lock);
	group->mcu = mcu;
	group->device = device;
}

void al5_group_deinit(struct al5_group *group)
{
	kfree(group->users);
}

int al5_group_bind_user(struct al5_group *group, struct al5_user *user)
{
	unsigned long flags = 0;
	int uid = -1;
	int err = 0;
	int i;

	spin_lock_irqsave(&group->lock, flags);
	for (i = 0; i < group->max_users_nb; ++i) {
		if (group->users[i] == NULL) {
			uid = i;
			break;
		}
	}
	if (uid == -1) {
		pr_err("Max user allocation reached\n");
		err = -EAGAIN;
		goto unlock;
	}

	group->users[uid] = user;
	al5_user_init(user, uid, group->mcu, group->device);

unlock:
	spin_unlock_irqrestore(&group->lock, flags);
	return err;

}
EXPORT_SYMBOL_GPL(al5_group_bind_user);

void al5_group_unbind_user(struct al5_group *group, struct al5_user *user)
{
	unsigned long flags = 0;

	spin_lock_irqsave(&group->lock, flags);
	group->users[user->uid] = NULL;
	spin_unlock_irqrestore(&group->lock, flags);
}
EXPORT_SYMBOL_GPL(al5_group_unbind_user);

struct al5_user *al5_group_user_from_uid(struct al5_group *group, int user_uid)
{
	struct al5_user *user;

	if (user_uid >= group->max_users_nb) {
		pr_err("Received user id %d, expected less than %ld\n",
		       user_uid, (unsigned long)group->max_users_nb);
		user = NULL;
	} else
		user = group->users[user_uid];

	return user;
}
EXPORT_SYMBOL_GPL(al5_group_user_from_uid);

static bool hasChannelId(struct al5_user *user, int channel_id)
{
	return user != NULL && user->chan_uid == channel_id;
}

struct al5_user *al5_group_user_from_chan_uid(struct al5_group *group,
					      int chan_uid)
{
	int i;

	for (i = 0; i < group->max_users_nb; ++i)
		if (hasChannelId(group->users[i], chan_uid))
			return group->users[i];

	pr_err("Cannot find channel from chan_uid %d received\n", chan_uid);

	return NULL;
}
EXPORT_SYMBOL_GPL(al5_group_user_from_chan_uid);

static void destroy_orphan_channel(struct al5_group *group, int chan_uid)
{
	int err;
	struct al5_mail *mail;

	mail = create_quiet_destroy_channel_msg(chan_uid);
	if (!mail)
		goto fail;

	err = al5_mcu_send(group->mcu, mail);
	if (err)
		goto fail;

	al5_signal_mcu(group->mcu);
	al5_free_mail(mail);
	return;

fail:
	pr_err("Couldn't destroy orphan mcu channel\n");
}

void print_cannot_retrieve_user(u32 mail_uid)
{
	pr_err("Mail of uid %d received related to inexistant user\n",
	       mail_uid);
}

bool should_destroy_channel_on_bad_feedback(u32 mail_uid)
{
	switch (mail_uid) {
	case AL_MCU_MSG_INIT:
	case AL_MCU_MSG_SEARCH_START_CODE:
	case AL_MCU_MSG_DESTROY_CHANNEL:
		return false;
	default:
		return true;
	}
}

struct al5_user *retrieve_user(struct al5_group *group, struct al5_mail *mail)
{
	struct al5_user *user;
	int user_uid, chan_uid = BAD_CHAN;

	switch (al5_mail_get_uid(mail)) {

	case AL_MCU_MSG_CREATE_CHANNEL:
		user_uid = al5_mail_get_word(mail, 1);
		user = al5_group_user_from_uid(group, user_uid);
		break;
	case AL_MCU_MSG_INIT:
	case AL_MCU_MSG_SEARCH_START_CODE:
		user_uid = al5_mail_get_word(mail, 0);
		user = al5_group_user_from_uid(group, user_uid);
		break;
	default:
		chan_uid = al5_mail_get_word(mail, 0);
		user = al5_group_user_from_chan_uid(group, chan_uid);
		break;
	}
	return user;
}

int try_to_deliver_to_user(struct al5_group *group, struct al5_mail *mail)
{
	struct al5_user *user = retrieve_user(group, mail);

	if (user == NULL) {
		print_cannot_retrieve_user(al5_mail_get_uid(mail));
		return -1;
	}

	al5_user_deliver(user, mail);
	return 0;
}

void handle_mail(struct al5_group *group, struct al5_mail *mail)
{
	int error;
	u32 mail_uid = al5_mail_get_uid(mail);

	if (mail_uid == AL_MCU_MSG_TRACE) {
		al5_print_mcu_trace(mail);
		return;
	}

	error = try_to_deliver_to_user(group, mail);
	if (error && should_destroy_channel_on_bad_feedback(mail_uid))
		destroy_orphan_channel(group, al5_mail_get_chan_uid(mail));
}

void read_mail(struct al5_group *group)
{
	struct al5_mail *mail = al5_mcu_recv(group->mcu);

	handle_mail(group, mail);
}

#define MAX_MAILBOX_MSG 100
void al5_group_read_mails(struct al5_group *group)
{
	unsigned nb_msg = 0;

	while (!al5_mcu_is_empty(group->mcu) && nb_msg++ < MAX_MAILBOX_MSG)
		read_mail(group);
}
