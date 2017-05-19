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
#include <linux/string.h>

#include "al_mail.h"
#include "al_mail_private.h"

struct al5_mail *al5_mail_create(u32 msg_uid, u32 size)
{
	struct al5_mail *mail = kmalloc(sizeof(*mail), GFP_KERNEL);
	if (!mail) {
		return NULL;
	}
	mail->body_offset = 0;
	mail->msg_uid = msg_uid;
	mail->body_size = size;
	mail->body = kmalloc(size, GFP_KERNEL);
	if (!mail->body) {
		kfree(mail);
		return NULL;
	}

	return mail;
}
EXPORT_SYMBOL_GPL(al5_mail_create);

void al5_mail_write(struct al5_mail *mail, void* content, u32 size)
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
	if (mail != NULL) {
		kfree(mail->body);
		kfree(mail);
	}
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

void * al5_mail_get_body(struct al5_mail *mail)
{
	return mail->body;
}
EXPORT_SYMBOL_GPL(al5_mail_get_body);

u32 al5_mail_get_word(struct al5_mail *mail, u32 word_offset)
{
	return ((u32*)(mail->body))[word_offset];
}
EXPORT_SYMBOL_GPL(al5_mail_get_word);

struct al5_mail *al5_mail_create_copy(struct al5_mail * mail)
{
	struct al5_mail * copy = al5_mail_create(mail->msg_uid, mail->body_size);
	al5_mail_write(copy, mail->body, mail->body_size);

	return copy;
}
EXPORT_SYMBOL_GPL(al5_mail_create_copy);
