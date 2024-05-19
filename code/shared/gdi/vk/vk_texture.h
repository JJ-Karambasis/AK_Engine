#ifndef VK_TEXTURE_H
#define VK_TEXTURE_H

struct vk_texture : vk_resource_base {
    VkImage       Handle;
    vk_allocation Allocation;
    dim2i         Size;
    gdi_format    Format;
    b16           JustAllocated;
    b16           IsSwapchain;
};

struct vk_texture_view : vk_resource_base {
    VkImageView Handle;
};

struct vk_sampler : vk_resource_base {
    VkSampler Handle;
};

internal bool VK_Create_Sampler(gdi_context* Context, vk_sampler* Sampler, const gdi_sampler_create_info& CreateInfo);
internal void VK_Delete_Sampler(gdi_context* Context, vk_sampler* Sampler);
internal void VK_Sampler_Record_Frame(gdi_context* Context, async_handle<vk_sampler> Handle);

internal bool VK_Create_Texture_View(gdi_context* Context, vk_texture_view* TextureView, const gdi_texture_view_create_info& CreateInfo);
internal void VK_Delete_Texture_View(gdi_context* Context, vk_texture_view* TextureView);
internal void VK_Texture_View_Record_Frame(gdi_context* Context, async_handle<vk_texture_view> Handle);

internal bool VK_Create_Texture(gdi_context* Context, vk_texture* Texture, const gdi_texture_create_info& CreateInfo);
internal void VK_Delete_Texture(gdi_context* Context, vk_texture* Texture);
internal void VK_Texture_Record_Frame(gdi_context* Context, async_handle<vk_texture> Handle);

#endif