/*
 * al_test.h
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

#ifndef _AL_TEST_H_
#define _AL_TEST_H_

#include "al_mail.h"
#include "al_codec_mails.h"
#include "mcu_interface.h"
#include "al_mailbox.h"

#include "al_codec.h"
#include "al_user.h"

struct al5_mail *create_set_timer_buffer_msg(struct msg_info *info);
struct al5_mail *create_set_irq_timer_buffer_msg(struct msg_info *info);
struct al5_mail *create_random_msg(u8 drv_chan_uid, u8 treasure);

void al5_handle_mcu_random_test(struct al5_group *group,
				struct al5_mail *mail);
int al5_set_timer_buffer(struct al5_codec_desc *codec, struct al5_user *user,
			 unsigned long arg);
int al5_set_irq_timer_buffer(struct al5_codec_desc *codec,
			     struct al5_user *user, unsigned long arg);

int al5_mail_tests(struct al5_user *user, unsigned long arg);

#endif /* _AL_TEST_H_ */
