/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef __AL_MAIL_PRIVATE__
#define __AL_MAIL_PRIVATE__

struct al5_mail {
	u32 body_offset;
	u16 msg_uid;
	u16 body_size;
	u8 *body;
};

size_t al5_get_mail_alloc_size(u32 content_size);
void al5_mail_init(struct al5_mail *mail, u32 msg_uid, u32 content_size);

#endif
