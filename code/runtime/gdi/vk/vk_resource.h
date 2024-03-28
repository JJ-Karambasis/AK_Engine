#ifndef VK_RESOURCE_H
#define VK_RESOURCE_H

struct vk_resource {
    VkFence LastUsedFence;
    u64     LastUsedFrameIndex;
};

void VK_Resource_Record_Frame(gdi_context* Context, vk_resource* Resource);
bool VK_Resource_Should_Queue(gdi_context* Context, const vk_resource* Resource);

template <typename type>
struct vk_resource_manager {
    gdi_context*      Context;
    async_pool<type>  Pool;
    async_queue<type> DeleteQueue;
};

#endif