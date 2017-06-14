#include "al_alloc_ioctl.h"
#include "al_alloc.h"

#include <linux/uaccess.h>

int al5_ioctl_get_dma_fd(struct device *dev, unsigned long arg)
{
	struct al5_dma_info info;
	int err;

	if (copy_from_user(&info, (struct al5_dma_info *)arg, sizeof(info)))
		return -EFAULT;

	err = al5_allocate_dmabuf(dev, info.size, &info.fd);
	if (err)
		return err;

	err = al5_dmabuf_get_address(dev, info.fd, &info.phy_addr);
	if (err)
		return err;

	if (copy_to_user((void *)arg, &info, sizeof(info)))
		return -EFAULT;

	return 0;
}
EXPORT_SYMBOL_GPL(al5_ioctl_get_dma_fd);

int al5_ioctl_get_dmabuf_dma_addr(struct device *dev, unsigned long arg)
{
	struct al5_dma_info info;
	int err;

	if (copy_from_user(&info, (struct al5_dma_info *)arg, sizeof(info)))
		return -EFAULT;

	err = al5_dmabuf_get_address(dev, info.fd, &info.phy_addr);
	if (err)
		return err;

	if (copy_to_user((void *)arg, &info, sizeof(info)))
		return -EFAULT;

	return 0;
}
EXPORT_SYMBOL_GPL(al5_ioctl_get_dmabuf_dma_addr);
