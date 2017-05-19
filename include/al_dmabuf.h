#include <linux/device.h>

struct al5_dma_buffer {
        u32 size;
        dma_addr_t dma_handle;
        void *cpu_handle;
};

int al5_create_dmabuf_fd(struct device *dev, unsigned long size, struct al5_dma_buffer *buffer);
