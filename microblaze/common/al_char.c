/******************************************************************************
*
* Copyright (C) 2026 Allegro DVT.  All rights reserved.
*
******************************************************************************/

#include "al_char.h"
#include "al_codec.h"

int al5_setup_chrdev_region(int *major, int base_minor, int nb_devs, char *desc)
{
	dev_t dev = 0;
	int err;

	if (*major == 0) {
		err = alloc_chrdev_region(&dev, base_minor, nb_devs, desc);
		*major = MAJOR(dev);

		if (err) {
			pr_alert("Allegro codec: can't get major %d\n", *major);
			return err;
		}
	}
	return 0;
}
EXPORT_SYMBOL_GPL(al5_setup_chrdev_region);

int al5_setup_cdev(struct cdev *cdev, const struct file_operations *fops,
		   struct module *owner, int major, int minor)
{
	int err, devno = MKDEV(major, minor);

	cdev_init(cdev, fops);
	cdev->owner = owner;
	err = cdev_add(cdev, devno, 1);
	if (err)
		return err;

	return 0;
}
EXPORT_SYMBOL_GPL(al5_setup_cdev);

void al5_clean_up_cdev(struct cdev *cdev)
{
	cdev_del(cdev);
}
EXPORT_SYMBOL_GPL(al5_clean_up_cdev);
