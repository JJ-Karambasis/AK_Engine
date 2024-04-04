#ifndef VK_TEXTURE_H
#define VK_TEXTURE_H

struct vk_texture {
    VkImage       Image;
    vk_allocation Allocation;
    u32           Width;
    u32           Height;
    gdi_format    Format;
};

struct vk_texture_view {
    async_handle<vk_texture> TextureHandle;
    VkImageView              ImageView;
};

internal void VK_Texture_Record_Frame(gdi_context* Context, async_handle<vk_texture> Handle);

bool VK_Create_Texture_View(gdi_context* Context, vk_texture_view* TextureView, const gdi_texture_view_create_info& CreateInfo);
void VK_Delete_Texture_View(gdi_context* Context, vk_texture_view* TextureView);
internal void VK_Texture_View_Record_Frame(gdi_context* Context, async_handle<vk_texture_view> Handle);

#endif