/*
 * al_test.c functions used to test the vcu mailboxes and the vcu firmware
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

#include <linux/random.h>
#include <linux/uaccess.h>

#include "al_test.h"
#include "al_mail.h"
#include "al_alloc.h"

#define TEST_MAILBOX_SIZE (0x400 - 8)
/* headers are 4 bytes, and a msg should be at least 4 bytes below the mailbox size */
#define MAX_MSG_SIZE (TEST_MAILBOX_SIZE - 12)
#define MAIL_TEST_NB 256

struct mail_test_ctx {
	/* mailbox state */
	struct mailbox *cpu_to_mcu;
	size_t mailbox_size;
	u32 old_head;
	u32 old_tail;
	u32 new_head;
	u32 new_tail;

	/* treasure msg state */
	u16 treasure_location;
	u16 test_mail_size;
	u8 treasure;
	size_t mail_size;

	int success_nb;
	int error_nb;
};

unsigned long get_random_bytes_in_range(int min, int max)
{
	unsigned long rand;
	get_random_bytes(&rand, sizeof(unsigned long));
	rand = (rand % (max - min)) + min;
	return rand;
}

struct al5_mail *create_random_msg(u8 drv_chan_uid, u8 treasure)
{
	struct al5_mail *mail;
	u16 treasure_location;
	size_t size;

	size = get_random_bytes_in_range(4, MAX_MSG_SIZE);
	mail = al5_create_msg(drv_chan_uid, 4, AL_MCU_MSG_RANDOM_TEST, size);

	treasure_location = get_random_bytes_in_range(3, mail->body_size);

	memset(mail->body, 0xdd, mail->body_size);

	mail->body[0] = drv_chan_uid;
	mail->body[1] = treasure_location & 0xff;
	mail->body[2] = treasure_location >> 8;
	mail->body[treasure_location] = treasure;

	return mail;
}

struct al5_mail *create_set_timer_buffer_msg(struct msg_info *info)
{
	u32 mcu_chan_uid = info->chan_uid;
	u32 phy_addr = (u32)(uintptr_t)info->priv;
	struct al5_mail *mail;
	int channel_uid_size = 4;

	mail = al5_create_msg(mcu_chan_uid, channel_uid_size,
			AL_MCU_MSG_SET_TIMER_BUFFER, sizeof(u32));

	memcpy(mail->body + channel_uid_size, &phy_addr, sizeof(u32));

	return mail;
}

struct al5_mail *create_set_irq_timer_buffer_msg(struct msg_info *info)
{
	u32 mcu_chan_uid = info->chan_uid;
	u32 phy_addr = (u32)(uintptr_t)info->priv;
	struct al5_mail *mail;
	int channel_uid_size = 4;

	mail = al5_create_msg(mcu_chan_uid, channel_uid_size,
			AL_MCU_MSG_SET_IRQ_TIMER_BUFFER, sizeof(u32));

	memcpy(mail->body + channel_uid_size, &phy_addr, sizeof(u32));

	return mail;
}

void reset_mailbox(struct mailbox *m)
{
	iowrite32(0, m->head);
	iowrite32(0, m->tail);
}

static void reset_all_mailbox(struct mcu_mailbox_interface *mcu)
{
	reset_mailbox(mcu->cpu_to_mcu);
	reset_mailbox(mcu->mcu_to_cpu);
}

void treasure_error(struct mail_test_ctx *ctx, u8 mail_treasure)
{
	pr_err("--- TREASURE ERROR !! ---\n"
		"got treasure %x\n"
		"On rand mail created of mail->body_size 0x%x, treasure 0x%x located in mail->body at 0x%x\n"
		"Before sending mail, head was 0x%x and tail was 0x%x\n"
		"After sending mail, head is 0x%x and tail is 0x%x\n\n",
		(unsigned int)mail_treasure, (unsigned int)ctx->mail_size, (unsigned int)ctx->treasure,
		(unsigned int)ctx->treasure_location,
	       (unsigned int)ctx->old_head, (unsigned int)ctx->old_tail, (unsigned int)ctx->new_head, (unsigned int)ctx->new_tail);
	ctx->error_nb++;
}

int send_msg_trace(struct al5_user *user, struct al5_mail *mail,
		struct mail_test_ctx *ctx)
{
	int err;
	err = al5_mcu_send(user->mcu, mail);
	if (err)
		return err;

	ctx->new_head = ioread32(ctx->cpu_to_mcu->head);
	ctx->new_tail = ioread32(ctx->cpu_to_mcu->tail);

	al5_signal_mcu(user->mcu);

	al5_free_mail(mail);

	return 0;
}

int send_and_verify(struct al5_user *user,
		struct mail_test_ctx *ctx, int iteration)
{
	struct al5_mail *mail, *feedback;
	u8 mail_treasure;
	int err;

	mail = create_random_msg(user->uid, ctx->treasure);
	if (!mail)
		return -ENOMEM;

	/* get information about the msg we are going to send */
	ctx->old_head = ioread32(ctx->cpu_to_mcu->head);
	ctx->old_tail = ioread32(ctx->cpu_to_mcu->tail);
	ctx->treasure_location = (u16)((mail->body[2] << 8) | (mail->body[1] & 0xff));
	ctx->mail_size = mail->body_size;

	pr_alert("[%d] mail size: %x\n", iteration, (unsigned int)ctx->mail_size);

	err = send_msg_trace(user, mail, ctx);
	if (err)
		return err;

	/* check if the message was correctly written to the mailbox */
	mail_treasure = ioread8(ctx->cpu_to_mcu->data
			+ (ctx->old_tail + ctx->treasure_location + 4)
			% ctx->mailbox_size);

	pr_alert("[%d] Wrote treasure %x to mailbox, wanted: %x\n",
			iteration, mail_treasure, ctx->treasure);

	feedback = al5_queue_pop(&user->queues[AL5_USER_MAIL_DEBUG]);
	if (feedback == NULL)
		return -EINTR;

	/* check if the mcu read the mail correctly and sent back
	 * the correct treasure */
	mail_treasure = feedback->body[1];
	if (mail_treasure == ctx->treasure)
		ctx->success_nb++;
	else {
		treasure_error(ctx, mail_treasure);
		al5_free_mail(feedback);
		return -1;
	}

	al5_free_mail(mail);
	al5_free_mail(feedback);

	return 0;
}

void init_test_ctx(struct mail_test_ctx *ctx, struct mailbox *m)
{
	memset(ctx, 0, sizeof(*ctx));

	ctx->cpu_to_mcu = m;
	ctx->mailbox_size = m->size;

	ctx->success_nb = 0;
	ctx->error_nb = 0;
}

int al5_mail_tests(struct al5_user *user, unsigned long arg)
{
	struct mail_test_ctx ctx;
	int i;
	int err = 0;

	reset_all_mailbox(user->mcu);
	init_test_ctx(&ctx, user->mcu->cpu_to_mcu);

	for (i = 0; i < MAIL_TEST_NB; ++i) {
		ctx.treasure = i;
		err = send_and_verify(user, &ctx, i);
		if (err)
			break;
	}

	pr_info("--- End of random mails test ---\n"
		 " Successes : %i\n"
		 " Errors : %i\n"
		 " Mails lost or not sent : %i\n",
		 ctx.success_nb, ctx.error_nb,
		 MAIL_TEST_NB - ctx.success_nb - ctx.error_nb);

	err = copy_to_user((void *)arg, &ctx.error_nb, sizeof(u32));
	if (err)
		return err;

	if (ctx.error_nb != 0 || ctx.success_nb != 256)
		return -EINVAL;

	return 0;
}
EXPORT_SYMBOL_GPL(al5_mail_tests);

void al5_handle_mcu_random_test(struct al5_codec_desc *codec,
				  struct al5_mail *mail)
{
	u32 user_uid = mail->body[0];
	struct al5_user *user;

	if (user_uid >= codec->max_users_nb) {
		al5_err("Got user %d, expected less than %ld\n",
			    user_uid, (unsigned long)codec->max_users_nb);
		return;
	}

	user = codec->users[user_uid];
	if (user == NULL) {
		al5_err("No channel left for random tests\n");
		return;
	}

	al5_user_deliver(user, mail);
}
EXPORT_SYMBOL_GPL(al5_handle_mcu_random_test);

#if AL5_DEBUG
static char* string_from_uid(AL_EMcuMessage msg_uid)
{
	switch (msg_uid)
	{
		case AL_MCU_MSG_INIT:
			return "AL_MCU_MSG_INIT";
		case AL_MCU_MSG_DEINIT:
			return "AL_MCU_MSG_DEINIT";
		case AL_MCU_MSG_CREATE_CHANNEL:
			return "AL_MCU_MSG_CREATE_CHANNEL";
		case AL_MCU_MSG_DESTROY_CHANNEL:
			return "AL_MCU_MSG_DESTROY_CHANNEL";
		case AL_MCU_MSG_ENCODE_ONE_FRM:
			return "AL_MCU_MSG_ENCODE_ONE_FRM";
		case AL_MCU_MSG_DECODE_ONE_FRM:
			return "AL_MCU_MSG_DECODE_ONE_FRM";
		case AL_MCU_MSG_SEARCH_START_CODE:
			return "AL_MCU_MSG_SEARCH_START_CODE";
		case AL_MCU_MSG_QUIET_DESTROY_CHANNEL:
			return "AL_MCU_MSG_QUIET_DESTROY_CHANNEL";
		case AL_MCU_MSG_GET_RESOURCE:
			return "AL_MCU_MSG_GET_RESOURCE";
		case AL_MCU_MSG_SET_BUFFER:
			return "AL_MCU_MSG_SET_BUFFER";
		case AL_MCU_MSG_SHUTDOWN:
			return "AL_MCU_MSG_SHUTDOWN";
		case AL_MCU_MSG_TRACE:
			return "AL_MCU_MSG_TRACE";
		case AL_MCU_MSG_SET_TIMER_BUFFER:
			return "AL_MCU_MSG_SET_TIMER_BUFFER";
		case AL_MCU_MSG_SET_IRQ_TIMER_BUFFER:
			return "AL_MCU_MSG_SET_IRQ_TIMER_BUFFER";
		default:
			return "unrepertoried";
	}
}
#endif

void al5_print_inmail_info(struct al5_mail *mail)
{
	mails_info("Received %s related to id %u", string_from_uid(mail->msg_uid), mail->body[0]);
}
EXPORT_SYMBOL_GPL(al5_print_inmail_info);

void al5_print_outmail_info(struct al5_mail *mail)
{
	mails_info("Sent %s related to id %u", string_from_uid(mail->msg_uid), mail->body[0]);
}
EXPORT_SYMBOL_GPL(al5_print_outmail_info);

void al5_print_mcu_trace(struct al5_codec_desc *codec,
			    struct al5_mail *mail)
{
	mcu_info("Mcu trace: %s", mail->body);
	al5_free_mail(mail);
}
EXPORT_SYMBOL_GPL(al5_print_mcu_trace);

/* User must have chan field filled */
int al5_set_timer_buffer(struct al5_codec_desc *codec, struct al5_user *user, unsigned long arg)
{
	struct msg_info msg_info;
	struct al5_dma_info info;
	u32 mcu_phy_addr;
	int err;

	if (copy_from_user(&info, (struct al5_dma_info *)arg, sizeof(info))) {
		return -EFAULT;
	}


	err = __get_dma_fd(codec->device, (void *)&info);
	if (err)
		return err;

	mcu_phy_addr = info.phy_addr + MCU_CACHE_OFFSET;

	msg_info.chan_uid = user->chan_uid;
	msg_info.priv = (void *)(uintptr_t)mcu_phy_addr;

	err = al5_create_and_send(user, &msg_info, create_set_timer_buffer_msg);
	if (err)
		return err;

	if (copy_to_user((void *)arg, &info, sizeof(info)))
		return -EFAULT;

	return 0;

}
EXPORT_SYMBOL_GPL(al5_set_timer_buffer);

int al5_set_irq_timer_buffer(struct al5_codec_desc *codec, struct al5_user *user, unsigned long arg)
{
	struct msg_info msg_info;
	struct al5_dma_info info;
	u32 mcu_phy_addr;
	int err;

	if (copy_from_user(&info, (struct al5_dma_info *)arg, sizeof(info))) {
		return -EFAULT;
	}


	err = __get_dma_fd(codec->device, (void *)&info);
	if (err)
		return err;

	mcu_phy_addr = info.phy_addr + MCU_CACHE_OFFSET;

	msg_info.chan_uid = user->chan_uid;
	msg_info.priv = (void *)(uintptr_t)mcu_phy_addr;

	err = al5_create_and_send(user, &msg_info, create_set_irq_timer_buffer_msg);
	if (err)
		return err;

	if (copy_to_user((void *)arg, &info, sizeof(info)))
		return -EFAULT;

	return 0;

}
EXPORT_SYMBOL_GPL(al5_set_irq_timer_buffer);

