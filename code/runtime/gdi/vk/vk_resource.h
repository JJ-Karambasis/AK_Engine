#ifndef VK_RESOURCE_H
#define VK_RESOURCE_H

struct vk_resource {
    VkFence LastUsedFence;
    u64     LastUsedFrameIndex;
};

struct vk_resource_manager;
struct vk_resource_manager : public resource_manager {
    gdi_context*                     Context;
    resource_allocate_callback_func* AllocateCallback;
    resource_free_callback_func*     FreeCallback;
};

struct vk_resource_manager_create_info {
    uptr                             ResourceSize;
    u32                              MaxCount;
    resource_allocate_callback_func* AllocateCallback;
    resource_free_callback_func*     FreeCallback;
};

void VK_Resource_Manager_Create(vk_resource_manager* ResourceManager, gdi_context* Context, const vk_resource_manager_create_info& CreateInfo);

#endif