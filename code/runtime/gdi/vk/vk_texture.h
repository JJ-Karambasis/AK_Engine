#ifndef VK_TEXTURE_H
#define VK_TEXTURE_H

struct vk_texture : public vk_resource {
    gdi_texture   ID;
    VkImage       Image;
    vk_allocation Allocation;
};

internal vk_texture* VK_Texture_Manager_Allocate_Sync(resource_manager* ResourceManager);
internal RESOURCE_ALLOCATE_CALLBACK_DEFINE(VK_Texture_Allocate_Callback);
internal RESOURCE_FREE_CALLBACK_DEFINE(VK_Texture_Free_Callback);

#endif