/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef _MCU_COMMON_H_
#define _MCU_COMMON_H_

#include <linux/types.h>

struct al5_mail;

struct al5_mail *al5_mail_create(u32 msg_uid, u32 size);
void al5_mail_write(struct al5_mail *mail, void *content, u32 size);
void al5_mail_write_word(struct al5_mail *mail, u32 word);
void al5_free_mail(struct al5_mail *m_data);
u32 al5_mail_get_uid(struct al5_mail *mail);
u32 al5_mail_get_size(struct al5_mail *mail);
void *al5_mail_get_body(struct al5_mail *mail);
u32 al5_mail_get_word(struct al5_mail *mail, u32 word_offset);

struct al5_mail *al5_mail_create_copy(struct al5_mail *mail);

#endif /* _MCU_COMMON_H_ */
