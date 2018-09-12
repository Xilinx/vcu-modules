/*
 * al_list.c
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

void al5_list_push(struct al5_list **l, struct al5_mail *mail)
{
	struct al5_list *first_pos = *l;

	if (*l == NULL) {
		*l = kmalloc(sizeof(**l), GFP_KERNEL);
		(*l)->mail = mail;
		(*l)->next = NULL;
		return;
	}
	while ((*l)->next != NULL)
		*l = (*l)->next;
	(*l)->next = kmalloc(sizeof(*((*l)->next)), GFP_KERNEL);
	(*l)->next->mail = mail;
	(*l)->next->next = NULL;
	*l = first_pos;
}

struct al5_mail *al5_list_pop(struct al5_list **l)
{
	if (*l == NULL)
		return NULL;

	struct al5_mail *ret = (*l)->mail;
	struct al5_list *next = (*l)->next;

	kfree(*l);
	*l = next;
	return ret;
}

void al5_list_empty_and_destroy(struct al5_list **l)
{
	while (!al5_list_empty(*l)) {
		struct al5_mail *mail = al5_list_pop(l);

		al5_free_mail(mail);
	}
	kfree(*l);
}
