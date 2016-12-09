/*
 * al_queue.c
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

#include "al_queue.h"

void al5_queue_init(struct al5_queue *q)
{
	init_waitqueue_head(&q->queue);
	spin_lock_init(&q->lock);
	al5_list_init(&q->list);
	q->locked = true;
}
EXPORT_SYMBOL_GPL(al5_queue_init);

struct al5_mail * al5_queue_pop_timeout(struct al5_queue *q)
{
	struct al5_mail * mail;
	unsigned long flags;

	wait_event_interruptible_timeout(q->queue,(!al5_list_empty(q->list)) || !q->locked, WAIT_TIMEOUT_DURATION);
	spin_lock_irqsave(&q->lock, flags);
	mail = al5_list_pop(&q->list);
	spin_unlock_irqrestore(&q->lock, flags);

	return mail;
}
EXPORT_SYMBOL_GPL(al5_queue_pop_timeout);

struct al5_mail * al5_queue_pop(struct al5_queue *q)
{
	struct al5_mail * mail;
	unsigned long flags;

	wait_event_interruptible(q->queue,(!al5_list_empty(q->list)) || !q->locked);
	spin_lock_irqsave(&q->lock, flags);
	mail = al5_list_pop(&q->list);
	spin_unlock_irqrestore(&q->lock, flags);

	return mail;
}
EXPORT_SYMBOL_GPL(al5_queue_pop);

void al5_queue_push(struct al5_queue *q, struct al5_mail *mail)
{
	unsigned long flags;

	spin_lock_irqsave(&q->lock, flags);
	al5_list_push(&q->list, mail);
	spin_unlock_irqrestore(&q->lock, flags);

	wake_up_interruptible(&q->queue);
}
EXPORT_SYMBOL_GPL(al5_queue_push);

void al5_queue_unlock(struct al5_queue *q)
{
	q->locked = false;
	wake_up_interruptible(&q->queue);
}
EXPORT_SYMBOL_GPL(al5_queue_unlock);

void al5_queue_lock(struct al5_queue *q)
{
	q->locked = true;
}
EXPORT_SYMBOL_GPL(al5_queue_lock);

