#ifndef VK_MEMORY_H
#define VK_MEMORY_H

struct vk_allocation {
    VkDeviceMemory Memory;
    VkDeviceSize   Offset;
};

#endif