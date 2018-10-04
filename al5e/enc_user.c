/*
 * enc_user.c high level communication with the vcu and the firmware
 * channel creation, frame encoding
 *
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
#include <linux/types.h>
#include <linux/string.h>
#include <linux/mutex.h>

#include "enc_user.h"
#include "enc_mails_factory.h"
#include "enc_feedbacks.h"
#include "al_dmabuf.h"
#include "al_buffers_pool.h"

#define CHECKPOINT_ALLOCATE_BUFFERS 1
#define CHECKPOINT_SEND_INTERMEDIATE_BUFFERS 2
#define CHECKPOINT_SEND_REFERENCE_BUFFERS 3

static void update_chan_param(struct al5_channel_status *status,
			      struct al5e_feedback_channel *message)
{
	status->options = message->options;
	status->num_core = (u8)message->num_core;
	status->pps_param = message->pps_param;
	status->error_code = message->error_code;
}

static int check_and_affect_chan_uid(struct al5_user *user, u32 chan_uid)
{
	if (chan_uid == BAD_CHAN)
		return -EINVAL;

	user->chan_uid = chan_uid;
	return 0;
}

static struct al5_mail *create_mail_from_bufpool(u32 msg_uid, u32 chan_uid,
						 struct al5_buffers_pool bufpool)
{
	struct al5_mail *mail;
	int i;
	u32 size_per_buffer = 3 * sizeof(u32);
	u32 content_size = bufpool.count * size_per_buffer;

	mail = al5_mail_create(msg_uid, 4 + content_size);
	al5_mail_write_word(mail, chan_uid);
	for (i = 0; i < bufpool.count; i++) {
		struct al5_dma_buffer *buffer = bufpool.buffers[i];
		u32 mcu_vaddr = al5_mcu_get_virtual_address(
			(u32)(uintptr_t)buffer->dma_handle);
		/* ip and mcu only support a 2G dma zone */
		al5_mail_write_word(mail, (u32)(uintptr_t)buffer->dma_handle);
		al5_mail_write_word(mail, mcu_vaddr);
		al5_mail_write_word(mail, buffer->size);
	}

	return mail;
}

static int allocate_channel_buffers(struct al5_user *user,
				    struct al5_channel_buffers buffers)
{
	int err;
	unsigned long pagesize = (1 << PAGE_SHIFT);

	buffers.rec_buffers_size += pagesize -
				    (buffers.rec_buffers_size % pagesize);
	err = al5_bufpool_allocate(&user->int_buffers, user->device,
				   buffers.int_buffers_count,
				   buffers.int_buffers_size);
	if (err)
		return err;
	err = al5_bufpool_allocate(&user->rec_buffers, user->device,
				   buffers.rec_buffers_count,
				   buffers.rec_buffers_size);
	if (err)
		return err;

	return 0;
}

static int send_intermediate_buffers(struct al5_user *user)
{
	struct al5_mail *mail;

	mail = create_mail_from_bufpool(AL_MCU_MSG_PUSH_BUFFER_INTERMEDIATE,
					user->chan_uid, user->int_buffers);
	return al5_check_and_send(user, mail);
}

static int send_reference_buffers(struct al5_user *user)
{
	struct al5_mail *mail;

	mail = create_mail_from_bufpool(AL_MCU_MSG_PUSH_BUFFER_REFERENCE,
					user->chan_uid, user->rec_buffers);
	return al5_check_and_send(user, mail);
}

static int try_to_create_channel(struct al5_user *user,
				 struct al5_params *param,
				 struct al5_channel_status *status,
				 struct al5e_feedback_channel *fb_message)
{
	struct al5_mail *feedback;
	int err =  al5_check_and_send(user, al5e_create_channel_param_msg(user->uid,
									  param));

	if (err)
		return err;

	err = al5_queue_pop_timeout(&feedback,
				    &user->queues[AL5_USER_MAIL_CREATE]);
	if (err)
		return err;

	*fb_message =
		*(struct al5e_feedback_channel *)al5_mail_get_body(feedback);
	al5_free_mail(feedback);
	update_chan_param(status, fb_message);

	err = check_and_affect_chan_uid(user, fb_message->chan_uid);
	if (err) {
		dev_err(user->device,
			"VCU: unavailable resources or wrong configuration");
		return err;
	}

	return 0;
}

static int channel_is_fully_created(struct al5_user *user)
{
	return al5_chan_is_created(user) && !al5_have_checkpoint(user);
}

int al5e_user_create_channel(struct al5_user *user,
			     struct al5_params *param,
			     struct al5_channel_status *status)
{
	struct al5e_feedback_channel fb_message = { 0 };
	int err = mutex_lock_killable(&user->locks[AL5_USER_CREATE]);

	if (err == -EINTR)
		return err;


	if (channel_is_fully_created(user)) {
		err = -EPERM;
		dev_err(user->device, "Channel already created!");
		goto fail;
	}

	if (!al5_have_checkpoint(user)) {
		err = try_to_create_channel(user, param, status, &fb_message);
		if (err) {
			dev_warn_ratelimited(user->device, "Failed on create channel");
			goto fail;
		}
		user->checkpoint = CHECKPOINT_ALLOCATE_BUFFERS;
	}

	if (user->checkpoint == CHECKPOINT_ALLOCATE_BUFFERS) {
		err = allocate_channel_buffers(user, fb_message.buffers_needed);
		if (err) {
			dev_warn_ratelimited(user->device,
					     "Failed internal buffers allocation, channel wasn't created");
			goto fail_allocate;
		}
		user->checkpoint = CHECKPOINT_SEND_INTERMEDIATE_BUFFERS;
	}

	if (user->checkpoint == CHECKPOINT_SEND_INTERMEDIATE_BUFFERS) {
		err = send_intermediate_buffers(user);
		if (err) {
			dev_warn_ratelimited(user->device,
					     "Failed to send intermediate buffers, channel wasn't created");
			goto fail;
		}
		user->checkpoint = CHECKPOINT_SEND_REFERENCE_BUFFERS;
	}

	if (user->checkpoint == CHECKPOINT_SEND_REFERENCE_BUFFERS) {
		err = send_reference_buffers(user);
		if (err) {
			dev_warn_ratelimited(user->device,
					     "Failed to send reference buffers, channel wasn't created");
			goto fail;
		}
		user->checkpoint = NO_CHECKPOINT;
	}

	goto unlock;

fail_allocate:
	user->checkpoint = NO_CHECKPOINT;
	mutex_unlock(&user->locks[AL5_USER_CREATE]);
	al5_user_destroy_channel(user, 0);
	return err;
fail:
unlock:
	mutex_unlock(&user->locks[AL5_USER_CREATE]);
	return err;
}
EXPORT_SYMBOL_GPL(al5e_user_create_channel);

int al5e_user_encode_one_frame(struct al5_user *user,
			       struct al5_encode_msg *msg)
{
	int err = mutex_lock_killable(&user->locks[AL5_USER_XCODE]);

	if (err == -EINTR)
		return err;

	if (!al5_chan_is_created(user)) {
		dev_err(user->device, "Cannot encode frame until channel is configured");
		err = -EPERM;
		goto unlock;
	}

	err = al5_check_and_send(user,
				 al5e_create_encode_one_frame_msg(user->chan_uid,
								  msg));

unlock:
	mutex_unlock(&user->locks[AL5_USER_XCODE]);
	return err;
}
EXPORT_SYMBOL_GPL(al5e_user_encode_one_frame);

int al5e_user_wait_for_status(struct al5_user *user,
			      struct al5_params *msg)
{
	struct al5_mail *feedback;
	int err = 0;

	if (!mutex_trylock(&user->locks[AL5_USER_STATUS]))
		return -EINTR;

	if (!al5_chan_is_created(user)) {
		err = -EPERM;
		goto unlock;
	}

	feedback = al5_queue_pop(&user->queues[AL5_USER_MAIL_STATUS]);
	if (feedback)
		al5e_mail_get_status(msg, feedback);
	else
		err = -EINTR;
	al5_free_mail(feedback);

unlock:
	mutex_unlock(&user->locks[AL5_USER_STATUS]);
	return err;
}

int al5e_user_put_stream_buffer(struct al5_user *user,
				struct al5_buffer *buffer)
{
	int error;
	struct al5_buffer_info buffer_info;
	struct al5_mail *mail;
	u32 mcu_vaddr;

	if (!al5_chan_is_created(user))
		return -EPERM;

	error = al5_get_dmabuf_info(user->device, buffer->handle, &buffer_info);
	if (error)
		return error;

	if (buffer->size > buffer_info.size)
		return -EFAULT;

	mail = al5_mail_create(AL_MCU_MSG_PUT_STREAM_BUFFER, 28);
	al5_mail_write_word(mail, user->chan_uid);
	al5_mail_write_word(mail, buffer_info.bus_address);
	mcu_vaddr = al5_mcu_get_virtual_address(buffer_info.bus_address);
	al5_mail_write_word(mail, mcu_vaddr);
	al5_mail_write_word(mail, buffer->size);
	al5_mail_write_word(mail, buffer->offset);
	al5_mail_write(mail, &buffer->stream_buffer_ptr, 8);

	return al5_check_and_send(user, mail);
}

static int get_user_rec_buffer(struct al5_user *user, int id)
{
	if (id > user->rec_buffers.count || id < 0)
		return -1;
	return al5_bufpool_reserve_fd(&user->rec_buffers, id);
}

int al5e_user_get_rec(struct al5_user *user, struct al5_reconstructed_info *msg)
{
	int err = 0;
	struct al5_mail *feedback;
	struct al5_mail *mail;

	if (!mutex_trylock(&user->locks[AL5_USER_REC]))
		return -EINTR;

	if (!al5_chan_is_created(user)) {
		err = -EPERM;
		goto unlock;
	}

	mail = al5_create_empty_mail(user->chan_uid,
				     AL_MCU_MSG_GET_RECONSTRUCTED_PICTURE);

	err = al5_check_and_send(user, mail);
	if (err)
		goto unlock;

	feedback = al5_queue_pop(&user->queues[AL5_USER_MAIL_REC]);
	if (!feedback) {
		err = -EINTR;
		goto unlock;
	}

	msg->fd = get_user_rec_buffer(user, al5_mail_get_word(feedback, 1));
	if (msg->fd == -1) {
		err = -EINVAL;
		goto no_rec_buf;
	}
	msg->pic_struct = al5_mail_get_word(feedback, 2);
	msg->poc = al5_mail_get_word(feedback, 3);

no_rec_buf:
	al5_free_mail(feedback);
unlock:
	mutex_unlock(&user->locks[AL5_USER_REC]);
	return err;
}

int al5e_user_release_rec(struct al5_user *user, u32 fd)
{
	u32 uid;
	int id;
	struct al5_mail *mail;
	int err = 0;

	if (!mutex_trylock(&user->locks[AL5_USER_REC]))
		return -EINTR;

	if (!al5_chan_is_created(user)) {
		err = -EPERM;
		goto unlock;
	}

	id = al5_bufpool_get_id(&user->rec_buffers, fd);
	if (id < 0) {
		err = -EINVAL;
		goto unlock;
	}
	uid = id;
	mail = al5_create_classic_mail(user->chan_uid,
				       AL_MCU_MSG_RELEASE_RECONSTRUCTED_PICTURE,
				       &uid, sizeof(uid));

	err = al5_check_and_send(user, mail);
unlock:
	mutex_unlock(&user->locks[AL5_USER_REC]);
	return err;
}

