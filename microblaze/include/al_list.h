/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef __AL_LIST__
#define __AL_LIST__

#include "al_mailbox.h"

struct al5_list {
	struct al5_mail *mail;
	struct al5_list *next;
};

void al5_list_init(struct al5_list **l);
int al5_list_empty(const struct al5_list *l);
int al5_list_push(struct al5_list **l, struct al5_mail *mail);
struct al5_mail *al5_list_pop(struct al5_list **l);
void al5_list_empty_and_destroy(struct al5_list **l);

#endif
