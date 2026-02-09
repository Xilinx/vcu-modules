/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#include <linux/slab.h>
#include <linux/string.h>

#include "al_mail.h"
#include "al_mail_private.h"

static size_t get_mail_struct_size(void)
{
	const size_t mail_size = roundup(sizeof(struct al5_mail), 4);

	return mail_size;
}

/* body should be aligned 32 bits */
size_t al5_get_mail_alloc_size(u32 content_size)
{
	return get_mail_struct_size() + content_size;
}

void al5_mail_init(struct al5_mail *mail, u32 msg_uid, u32 content_size)
{
	mail->body_offset = 0;
	mail->msg_uid = msg_uid;
	mail->body_size = content_size;
	mail->body = (u8 *)mail + get_mail_struct_size();
}

struct al5_mail *al5_mail_create(u32 msg_uid, u32 content_size)
{
	struct al5_mail *mail =
		kmalloc(al5_get_mail_alloc_size(content_size), GFP_KERNEL);

	if (!mail)
		return NULL;

	al5_mail_init(mail, msg_uid, content_size);

	return mail;
}
EXPORT_SYMBOL_GPL(al5_mail_create);

void al5_mail_write(struct al5_mail *mail, void *content, u32 size)
{
	memcpy(mail->body + mail->body_offset, content, size);
	mail->body_offset += size;
}
EXPORT_SYMBOL_GPL(al5_mail_write);

void al5_mail_write_word(struct al5_mail *mail, u32 word)
{
	al5_mail_write(mail, &word, 4);
}
EXPORT_SYMBOL_GPL(al5_mail_write_word);

void al5_free_mail(struct al5_mail *mail)
{
	if (mail != NULL)
		kfree(mail);
}
EXPORT_SYMBOL_GPL(al5_free_mail);

u32 al5_mail_get_uid(struct al5_mail *mail)
{
	return mail->msg_uid;
}
EXPORT_SYMBOL_GPL(al5_mail_get_uid);

u32 al5_mail_get_size(struct al5_mail *mail)
{
	return mail->body_size;
}
EXPORT_SYMBOL_GPL(al5_mail_get_size);

void *al5_mail_get_body(struct al5_mail *mail)
{
	return mail->body;
}
EXPORT_SYMBOL_GPL(al5_mail_get_body);

u32 al5_mail_get_word(struct al5_mail *mail, u32 word_offset)
{
	return ((u32 *)(mail->body))[word_offset];
}
EXPORT_SYMBOL_GPL(al5_mail_get_word);

struct al5_mail *al5_mail_create_copy(struct al5_mail *mail)
{
	struct al5_mail *copy = al5_mail_create(mail->msg_uid, mail->body_size);

	al5_mail_write(copy, mail->body, mail->body_size);

	return copy;
}
EXPORT_SYMBOL_GPL(al5_mail_create_copy);
