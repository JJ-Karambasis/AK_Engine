#ifndef VK_SWAPCHAIN_H
#define VK_SWAPCHAIN_H

internal string VK_Get_Surface_Extension_Name();
internal bool VK_Is_Surface_Extension(string ExtensionName);
internal void VK_Set_Surface_Extension(vk_instance_extension_support* InstanceExtensions);
internal bool VK_Has_Surface_Extension(const vk_instance_extension_support* InstanceExtensions);
internal VkSurfaceKHR VK_Create_Surface(gdi* GDI, gdi_window_data* WindowData);
internal VkSurfaceKHR VK_Create_Temp_Surface(gdi* GDI);

#endif