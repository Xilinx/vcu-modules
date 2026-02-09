/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef _MCU_MAILBOX_H_
#define _MCU_MAILBOX_H_

#include <linux/types.h>

#include "al_mail.h"

struct mailbox {
	size_t size; /* In bytes */
	u8 *data;
	void *tail;
	void *head;
	u16 local_tail;
};

void al5_mailbox_init(struct mailbox *box, void *base, size_t data_size);
int al5_mailbox_write(struct mailbox *box, struct al5_mail *mail);
bool al5_mailbox_read(struct mailbox *box, struct al5_mail *mail,
		      size_t mail_size);

#endif /* _MCU_MAILBOX_H_ */
