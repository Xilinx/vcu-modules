/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#include "al_user.h"
#include "al_enc_ioctl.h"

int al5e_user_create_channel(struct al5_user *user,
			     struct al5_config_channel *msg);
int al5e_user_encode_one_frame(struct al5_user *user,
			       struct al5_encode_msg *msg);
int al5e_user_wait_for_status(struct al5_user *user, struct al5_params *msg);
int al5e_user_put_stream_buffer(struct al5_user *user,
				struct al5_buffer *buffer);

int al5e_user_release_rec(struct al5_user *user, s32 fd);
int al5e_user_get_rec(struct al5_user *user,
		      struct al5_reconstructed_info *msg);

int al5e_user_get(struct al5_user *user, struct al5_params *msg);
int al5e_user_set(struct al5_user *user, struct al5_params *msg);
