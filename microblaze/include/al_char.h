/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#ifndef _AL_CHAR_H_
#define _AL_CHAR_H_

#include <linux/fs.h>
#include <linux/module.h>

int al5_setup_chrdev_region(int *major, int minor, int nb_devs, char *desc);
int al5_setup_cdev(struct cdev *cdev, const struct file_operations *fops,
		   struct module *owner, int major, int minor);
void al5_clean_up_cdev(struct cdev *cdev);

#endif /* _AL_CHAR_H_ */
