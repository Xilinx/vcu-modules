/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef _MCU_ENC_H_
#define _MCU_ENC_H_

#include "al_mail.h"
#include "al_enc_ioctl.h"
#include "al_codec_mails.h"

void al5e_mail_get_status(struct al5_params *status, struct al5_mail *mail);
struct al5_mail *al5e_create_encode_one_frame_msg(u32 chan_uid,
						  struct al5_encode_msg *msg);
struct al5_mail *al5e_create_channel_param_msg(u32 user_uid,
					       struct al5_params *msg,
					       u32 mcu_rc_plugin_addr,
					       u32 mcu_rc_plugin_size);
struct al5_mail *al5e_get_msg(u32 user_uid, struct al5_params *msg);
struct al5_mail *al5e_set_msg(u32 user_uid, struct al5_params *msg);
#endif /* _MCU_ENC_H_ */
