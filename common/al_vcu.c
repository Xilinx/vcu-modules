/*
 * al_vcu.c handle communication with mcu via interrupts
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

#include <linux/io.h>
#include <linux/uaccess.h>

#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/firmware.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/slab.h>

#include "al_vcu.h"
#include "al_test.h"

#include "al_mail.h"



static int handle_wrong_channel(struct al5_codec_desc *codec, u32 chan_uid)
{
	int err;
	struct al5_mail *mail;
	struct msg_info info = {
		.chan_uid = chan_uid,
		.priv = NULL,
	};
	/* nobody is waiting for us : this mcu_channel_id doesn't exist for driver */
	al5_err("Got a msg from mcu_channel %d, but no drv channel is waiting for it\n",
		    chan_uid);
	/* Send destroy_channel mail to mcu */
	mail = create_quiet_destroy_channel_msg(&info);
	if (!mail) {
		return -ENOMEM;
	}
	al5_err("Destroying orphan mcu channel\n");
	err = al5_mcu_send(&codec->mcu, mail);
	al5_signal_mcu(&codec->mcu);
	al5_free_mail(mail);
	if (err)
		return err;

	return 0;
}



#define MAX_MAILBOX_MSG (MAILBOX_SIZE / 4)
static void read_mails(struct al5_codec_desc *codec) {
	struct al5_user *user;
	struct al5_mail *mail;
	u32 user_uid, chan_uid;
	int err;
	unsigned nb_msg = 0;

	while (!al5_mcu_is_empty(&codec->mcu) && nb_msg++ < MAX_MAILBOX_MSG) {
		mail = al5_mcu_recv(&codec->mcu);
		if (mail == NULL) {
			al5_err("Mail lost\n");
			continue;
		}
		al5_print_inmail_info(mail);

		switch (mail->msg_uid) {

		/* NSFProd */
		case AL_MCU_MSG_RANDOM_TEST:
			al5_handle_mcu_random_test(codec, mail);
			continue;
		/* NSFProd */
		case AL_MCU_MSG_TRACE:
			al5_print_mcu_trace(codec, mail);
			continue;
		case AL_MCU_MSG_CREATE_CHANNEL:
			al5_get_user_uid(&user_uid, mail);
			user = al5_get_user_from_uid(codec, user_uid);
			break;
		case AL_MCU_MSG_INIT:
		case AL_MCU_MSG_SEARCH_START_CODE:
			al5_get_chan_uid(&user_uid, mail);
			user = al5_get_user_from_uid(codec, user_uid);
			break;
		default:
			al5_get_chan_uid(&chan_uid, mail);
			user = al5_get_user_from_chan_uid(codec, chan_uid);
			break;
		}

		if (user == NULL) {
			al5_err("Mail %d received related to inexistant user", mail->msg_uid);
			if (mail->msg_uid == AL_MCU_MSG_DESTROY_CHANNEL)
				continue;
			err = handle_wrong_channel(codec, chan_uid);
			if (err)
				al5_err("Couldn't destroy orphan mcu channel\n");
			continue;
		}
		al5_user_deliver(user, mail);
	}
}

static void ack_interrupt(struct al5_codec_desc *codec)
{
	al5_writel(0x1, AL5_MCU_INTERRUPT_CLR);
}

irqreturn_t al5_hardirq_handler(int irq, void *data)
{
	struct al5_codec_desc *codec = (struct al5_codec_desc *)data;
	ack_interrupt(codec);
	return IRQ_WAKE_THREAD;
}
EXPORT_SYMBOL_GPL(al5_hardirq_handler);

irqreturn_t al5_irq_handler(int irq, void *data)
{
	struct al5_codec_desc *codec = (struct al5_codec_desc *)data;

	spin_lock(&codec->lock);
	irq_info("Got irq from Mcu");
	read_mails(codec);
	spin_unlock(&codec->lock);

	return IRQ_HANDLED;
}
EXPORT_SYMBOL_GPL(al5_irq_handler);

