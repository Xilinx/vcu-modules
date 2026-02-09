/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#include "al_user.h"
#include "al_dec_ioctl.h"

int al5d_user_create_channel(struct al5_user *user,
			     struct al5_channel_config *msg);
int al5d_user_decode_one_frame(struct al5_user *user,
			       struct al5_decode_msg *msg);
int al5d_user_search_start_code(struct al5_user *user,
				struct al5_search_sc_msg *msg);
int al5d_user_wait_for_status(struct al5_user *user, struct al5_params *msg);
int al5d_user_wait_for_start_code(struct al5_user *user,
				  struct al5_scstatus *msg);
int al5d_user_decode_one_slice(struct al5_user *user,
			       struct al5_decode_msg *msg);
int al5d_user_get(struct al5_user *user, struct al5_params *msg);
int al5d_user_set(struct al5_user *user, struct al5_params *msg);
