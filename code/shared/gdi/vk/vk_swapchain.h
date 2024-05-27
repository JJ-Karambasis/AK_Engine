#ifndef VK_SWAPCHAIN_H
#define VK_SWAPCHAIN_H

struct vk_semaphore;

struct vk_swapchain : vk_resource_base {
    gdi_format                     Format;
    gdi_texture_usage_flags        UsageFlags;
    VkSurfaceKHR                   Surface;
    VkSwapchainKHR                 Handle;
    array<gdi_handle<gdi_texture>> Textures;
    dim2i                          Size;
    s32                            TextureIndex;
    vk_semaphore*                  AcquireSemaphore;
};

internal string VK_Get_Surface_Extension_Name();
internal bool VK_Is_Surface_Extension(string ExtensionName);
internal void VK_Set_Surface_Extension(vk_instance_extension_support* InstanceExtensions);
internal bool VK_Has_Surface_Extension(const vk_instance_extension_support* InstanceExtensions);
internal VkSurfaceKHR VK_Create_Surface(gdi* GDI, const gdi_window_data* WindowData);
internal VkSurfaceKHR VK_Create_Temp_Surface(gdi* GDI);

internal bool VK_Create_Swapchain(gdi_context* Context, vk_swapchain* Swapchain, const gdi_swapchain_create_info& CreateInfo);
internal bool VK_Create_Swapchain_Textures(gdi_context* Context, vk_swapchain* Swapchain);
internal void VK_Delete_Swapchain(gdi_context* Context, vk_swapchain* Swapchain);
internal void VK_Delete_Swapchain_Textures(gdi_context* Context, vk_swapchain* Swapchain);
internal void VK_Delete_Swapchain_Full(gdi_context* Context, vk_swapchain* Swapchain);
internal void VK_Swapchain_Record_Frame(gdi_context* Context, async_handle<vk_swapchain> Handle);

#endif