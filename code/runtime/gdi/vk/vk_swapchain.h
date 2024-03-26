#ifndef VK_SWAPCHAIN_H
#define VK_SWAPCHAIN_H

struct vk_swapchain : public vk_resource {
    gdi_format              Format;
    gdi_texture_usage_flags UsageFlags;
    VkSurfaceKHR            Surface;
    VkSwapchainKHR          Swapchain;
    array<gdi_texture>      Textures;
};

struct vk_swapchain_delete_info {
    bool PartialDelete;
};

internal string VK_Get_Surface_Extension_Name();
internal bool VK_Is_Surface_Extension(string ExtensionName);
internal void VK_Set_Surface_Extension(vk_instance_extension_support* InstanceExtensions);
internal bool VK_Has_Surface_Extension(const vk_instance_extension_support* InstanceExtensions);
internal VkSurfaceKHR VK_Create_Surface(gdi* GDI, const gdi_window_data* WindowData);
internal VkSurfaceKHR VK_Create_Temp_Surface(gdi* GDI);

internal RESOURCE_ALLOCATE_CALLBACK_DEFINE(VK_Swapchain_Allocate_Callback);
internal RESOURCE_FREE_CALLBACK_DEFINE(VK_Swapchain_Free_Callback);

#endif