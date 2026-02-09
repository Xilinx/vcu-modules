/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#include "al_list.h"
#include <linux/slab.h>

void al5_list_init(struct al5_list **l)
{
	*l = NULL;
}

int al5_list_empty(const struct al5_list *l)
{
	return l == NULL;
}

/* return -ENOMEM if we failed to create the list object, 0 otherwise */
int al5_list_push(struct al5_list **l, struct al5_mail *mail)
{
	struct al5_list *first_pos = *l;

	if (*l == NULL) {
		*l = kmalloc(sizeof(**l), GFP_NOWAIT);
		if (*l == NULL)
			return -ENOMEM;
		(*l)->mail = mail;
		(*l)->next = NULL;
		return 0;
	}
	while ((*l)->next != NULL)
		*l = (*l)->next;
	(*l)->next = kmalloc(sizeof(*((*l)->next)), GFP_NOWAIT);
	if ((*l)->next == NULL)
		return -ENOMEM;
	(*l)->next->mail = mail;
	(*l)->next->next = NULL;
	*l = first_pos;

	return 0;
}

struct al5_mail *al5_list_pop(struct al5_list **l)
{
	struct al5_mail *mail;
	struct al5_list *next;

	if (*l == NULL)
		return NULL;

	mail = (*l)->mail;
	next = (*l)->next;

	kfree(*l);
	*l = next;
	return mail;
}

void al5_list_empty_and_destroy(struct al5_list **l)
{
	while (!al5_list_empty(*l)) {
		struct al5_mail *mail = al5_list_pop(l);

		al5_free_mail(mail);
	}
	kfree(*l);
}
