/*
 * al_mailbox.c mailbox mechanisms. r/w messages to/from the mcu
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
#include <linux/io.h>

#include "al_mailbox.h"
#include "mcu_utils.h"

const int header_size = 4;

static int not_enough_space_in_mailbox(size_t mailbox_size,
				       size_t used_size,
				       size_t data_size)
{
	/* Head & Tail can only be multiples of 4,
	 * and they can only be equals if mailbox is empty
	 */
	return data_size + 4 > mailbox_size - used_size;
}

static int mailbox_is_wrapping(size_t mailbox_size,
			       unsigned int tail_value,
			       size_t data_size)
{
	return tail_value + data_size > mailbox_size;
}
/* Require mailbox_is_wrapping */
static void write_in_mailbox_until_wrap(struct mailbox *box,
					size_t *memcpy_size,
					unsigned int *tail,
					u8 **data)
{
	size_t space_left_before_wrapping = box->size - *tail;

	memcpy_toio_32(box->data + *tail, *data, space_left_before_wrapping);
	*tail = 0;
	*memcpy_size -= space_left_before_wrapping;
	*data += space_left_before_wrapping;
}

/* Require mailbox_is_wrapping */
static void read_in_mailbox_until_wrap(struct mailbox *box,
				       size_t *memcpy_size,
				       unsigned int *head,
				       u8 **data)
{

	size_t space_left_before_wrapping = box->size - *head;

	memcpy_fromio_32(*data, box->data + *head,  space_left_before_wrapping);
	*head = 0;
	*memcpy_size -= space_left_before_wrapping;
	*data += space_left_before_wrapping;
}

void al5_mailbox_init(struct mailbox *box, void* base, size_t data_size)
{
	box->size = data_size;
	box->head = base;
	box->tail = base + 4;
	box->data = base + 8;
	iowrite32(0, box->head);
	iowrite32(0, box->tail);
}
EXPORT_SYMBOL_GPL(al5_mailbox_init);

static void unserialize_header(u8 *header, u16 *msg_uid, u16 *body_size)
{
	*body_size = header[0] + (header[1] << 8);
	*msg_uid = header[2] + (header[3] << 8);
}

static void serialize_header(u8 *header, u16 msg_uid, u16 body_size)
{
	header[0] = body_size & 0x00ff;
	header[1] = body_size >> 8;
	header[2] = msg_uid & 0x00ff;
	header[3] = msg_uid >> 8;
}

static void pull_tail(struct mailbox *box)
{
	box->local_tail = ioread32(box->tail);
}

static void push_tail(struct mailbox *box)
{
	iowrite32(box->local_tail, box->tail);
}

/* Assume there is enough place in mailbox */
static void write_data(struct mailbox *box, u8 *in_data, size_t size)
{
	u32 tail_value = box->local_tail;
	if (mailbox_is_wrapping(box->size, tail_value, size))
		write_in_mailbox_until_wrap(box,
					    &size,
					    &tail_value,
					    &in_data);
	memcpy_toio_32(box->data + tail_value, in_data, size);

	tail_value = ((tail_value + size + 3) / 4 * 4) % box->size;
	box->local_tail = tail_value;
}

static void read_data(struct mailbox *box, u8 *out_data, size_t size)
{
	u32 head_value = ioread32(box->head);
	u8 *data_iterator = out_data;
	if (mailbox_is_wrapping(box->size, head_value, size))
		read_in_mailbox_until_wrap(box, &size, &head_value, &data_iterator);
	memcpy_fromio_32(data_iterator, box->data + head_value, size);

	head_value = ((head_value + size + 3) / 4 * 4) % box->size;
	iowrite32(head_value, box->head);
}

int al5_mailbox_write(struct mailbox *box, struct al5_mail *mail)
{
	u8 header[header_size];
	size_t mail_size = mail->body_size + header_size;
	unsigned int head_value = ioread32(box->head);
	unsigned int tail_value = ioread32(box->tail);
	size_t mailbox_size = box->size;
	size_t used_size =
		(tail_value >= head_value) ? (tail_value - head_value)
		: (mailbox_size + tail_value - head_value);

	if (not_enough_space_in_mailbox(mailbox_size, used_size, mail_size))
		return -EAGAIN;

	serialize_header(header, mail->msg_uid, mail->body_size);

	pull_tail(box);
	write_data(box, header, header_size);
	write_data(box, mail->body, mail->body_size);
	push_tail(box);

	return 0;
}
EXPORT_SYMBOL_GPL(al5_mailbox_write);

void al5_mailbox_read(struct mailbox *box, struct al5_mail *mail)
{
	u8 header[header_size];
	read_data(box, header, header_size);
	unserialize_header(header, &mail->msg_uid, &mail->body_size);

	mail->body = kmalloc(mail->body_size, GFP_KERNEL);

	read_data(box, mail->body, mail->body_size);
}
EXPORT_SYMBOL_GPL(al5_mailbox_read);

