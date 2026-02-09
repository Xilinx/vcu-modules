/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef __AL_QUEUE__
#define __AL_QUEUE__

#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>

#include "al_mailbox.h"
#include "al_list.h"

#define WAIT_TIMEOUT_DURATION (HZ * 5)

struct al5_queue {
	wait_queue_head_t queue;
	struct al5_list *list;
	spinlock_t lock;
	int locked;
};

void al5_queue_init(struct al5_queue *q);
struct al5_mail *al5_queue_pop_no_wait(struct al5_queue *q);
struct al5_mail *al5_queue_pop(struct al5_queue *q);
int al5_queue_pop_timeout(struct al5_mail **mail, struct al5_queue *q);
int al5_queue_push(struct al5_queue *q, struct al5_mail *mail);
void al5_queue_unlock(struct al5_queue *q);
void al5_queue_lock(struct al5_queue *q);

#endif
