/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#include <linux/device.h>
#include <linux/string.h>

#include "al_mail.h"
#include "al_mail_private.h"
#include "al_codec_mails.h"
#include "dec_mails_factory.h"

const u32 sender_uid_size = 4;

/* *msg is filled, *mail is read-only */
void al5d_mail_get_status(struct al5_params *status, struct al5_mail *mail)
{
	status->size = mail->body_size - 4;
	memcpy(&status->opaque, mail->body + 4, status->size);
}

/* *msg is filled, *mail is read-only */
void al5d_mail_get_sc_status(struct al5_scstatus *scstatus,
			     struct al5_mail *mail)
{
	memcpy(scstatus, mail->body + 4, sizeof(*scstatus));
}

void write_decode_mail(struct al5_mail *mail, u32 chan_uid,
		       struct al5_decode_msg *msg)
{
	al5_mail_write_word(mail, chan_uid);
	al5_mail_write(mail, msg->params.opaque, msg->params.size);
	al5_mail_write(mail, msg->addresses.opaque, msg->addresses.size);
	al5_mail_write_word(mail, msg->slice_param_v);
}

struct al5_mail *al5d_create_decode_one_slice_msg(u32 chan_uid,
						  struct al5_decode_msg *msg)
{
	int mail_size = 4 + msg->params.size + msg->addresses.size +
			sizeof(msg->slice_param_v);
	struct al5_mail *mail =
		al5_mail_create(AL_MCU_MSG_DECODE_ONE_SLICE, mail_size);

	write_decode_mail(mail, chan_uid, msg);

	return mail;
}

struct al5_mail *al5d_create_decode_one_frame_msg(u32 chan_uid,
						  struct al5_decode_msg *msg)
{
	int mail_size = 4 + msg->params.size + msg->addresses.size +
			sizeof(msg->slice_param_v);
	struct al5_mail *mail =
		al5_mail_create(AL_MCU_MSG_DECODE_ONE_FRM, mail_size);

	write_decode_mail(mail, chan_uid, msg);

	return mail;
}

struct al5_mail *al5d_create_channel_param_msg(u32 user_uid,
					       struct al5_params *msg)
{
	int mail_size = 4 + msg->size;
	struct al5_mail *mail =
		al5_mail_create(AL_MCU_MSG_CREATE_CHANNEL, mail_size);

	al5_mail_write_word(mail, user_uid);
	al5_mail_write(mail, msg->opaque, msg->size);
	return mail;
}

struct al5_mail *al5d_create_search_sc_mail(u32 user_uid,
					    struct al5_search_sc_msg *msg)
{
	int mail_size = 4 + msg->param.size + msg->buffer_addrs.size;
	struct al5_mail *mail =
		al5_mail_create(AL_MCU_MSG_SEARCH_START_CODE, mail_size);

	al5_mail_write_word(mail, user_uid);
	al5_mail_write(mail, msg->param.opaque, msg->param.size);
	al5_mail_write(mail, msg->buffer_addrs.opaque, msg->buffer_addrs.size);

	return mail;
}

struct al5_mail *al5d_get_msg(u32 user_uid, struct al5_params *msg)
{
	int mail_size = 4 + msg->size;
	struct al5_mail *mail = al5_mail_create(AL_MCU_MSG_GET, mail_size);

	al5_mail_write_word(mail, user_uid);
	al5_mail_write(mail, msg->opaque, msg->size);
	return mail;
}

struct al5_mail *al5d_set_msg(u32 user_uid, struct al5_params *msg)
{
	int mail_size = 4 + msg->size;
	struct al5_mail *mail = al5_mail_create(AL_MCU_MSG_SET, mail_size);

	al5_mail_write_word(mail, user_uid);
	al5_mail_write(mail, msg->opaque, msg->size);
	return mail;
}
