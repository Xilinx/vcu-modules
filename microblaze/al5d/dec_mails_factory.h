/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef _MCU_DEC_H_
#define _MCU_DEC_H_

#include "mcu_interface.h"
#include "al_mail.h"
#include "al_dec_ioctl.h"

void al5d_mail_get_status(struct al5_params *status, struct al5_mail *mail);
void al5d_mail_get_sc_status(struct al5_scstatus *scstatus,
			     struct al5_mail *mail);

struct al5_mail *al5d_create_decode_one_frame_msg(u32 chan_uid,
						  struct al5_decode_msg *msg);
struct al5_mail *al5d_create_channel_param_msg(u32 user_uid,
					       struct al5_params *msg);
struct al5_mail *al5d_create_search_sc_mail(u32 user_uid,
					    struct al5_search_sc_msg *msg);
struct al5_mail *al5d_create_decode_one_slice_msg(u32 chan_uid,
						  struct al5_decode_msg *msg);
struct al5_mail *al5d_get_msg(u32 user_uid, struct al5_params *msg);
struct al5_mail *al5d_set_msg(u32 user_uid, struct al5_params *msg);

#endif /* _MCU_DEC_H_ */
