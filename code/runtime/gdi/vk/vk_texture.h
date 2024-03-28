#ifndef VK_TEXTURE_H
#define VK_TEXTURE_H

struct vk_texture : public vk_resource {
    VkImage       Image;
    vk_allocation Allocation;
    u32           Width;
    u32           Height;
    gdi_format    Format;
};

struct vk_texture_view : public vk_resource {
    gdi_handle<gdi_texture> TextureHandle;
    VkImageView             ImageView;
};

bool VK_Create_Texture_View(gdi_context* Context, vk_texture_view* TextureView, const gdi_texture_view_create_info& CreateInfo);
void VK_Delete_Texture_View(gdi_context* Context, vk_texture_view* TextureView);

#endif