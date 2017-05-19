#ifndef __AL_MAIL_PRIVATE__
#define __AL_MAIL_PRIVATE__

struct al5_mail {
	u32 body_offset;
	u16 msg_uid;
	u16 body_size;
	u8 *body;
};

#endif
