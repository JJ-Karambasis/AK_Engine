#ifndef VK_LOADER_H
#define VK_LOADER_H

#ifdef __cplusplus
#define VKLOADERDEF extern "C"
#else
#define VKLOADERDEF
#endif

#define VK_FUNCTION(name) PFN_##name name

typedef struct {
    bool Enabled;
    VK_FUNCTION(vkDestroySurfaceKHR);
    VK_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR);
    VK_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    VK_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR);
    VK_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR);
} vk_khr_surface;

typedef struct {
    bool Enabled;
    VK_FUNCTION(vkGetPhysicalDeviceFeatures2KHR);
    VK_FUNCTION(vkGetPhysicalDeviceProperties2KHR);
    VK_FUNCTION(vkGetPhysicalDeviceFormatProperties2KHR);
    VK_FUNCTION(vkGetPhysicalDeviceImageFormatProperties2KHR);
    VK_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties2KHR);
    VK_FUNCTION(vkGetPhysicalDeviceMemoryProperties2KHR);
    VK_FUNCTION(vkGetPhysicalDeviceSparseImageFormatProperties2KHR);
} vk_khr_get_physical_device_properties2;

typedef struct {
    bool Enabled;
    VK_FUNCTION(vkCreateDebugReportCallbackEXT);
    VK_FUNCTION(vkDebugReportMessageEXT);
    VK_FUNCTION(vkDestroyDebugReportCallbackEXT);
} vk_ext_debug_report;

typedef struct {
    bool Enabled;
    VK_FUNCTION(vkSetDebugUtilsObjectNameEXT);
    VK_FUNCTION(vkSetDebugUtilsObjectTagEXT);
    VK_FUNCTION(vkQueueBeginDebugUtilsLabelEXT);
    VK_FUNCTION(vkQueueEndDebugUtilsLabelEXT);
    VK_FUNCTION(vkQueueInsertDebugUtilsLabelEXT);
    VK_FUNCTION(vkCmdBeginDebugUtilsLabelEXT);
    VK_FUNCTION(vkCmdEndDebugUtilsLabelEXT);
    VK_FUNCTION(vkCmdInsertDebugUtilsLabelEXT);
    VK_FUNCTION(vkCreateDebugUtilsMessengerEXT);
    VK_FUNCTION(vkDestroyDebugUtilsMessengerEXT);
    VK_FUNCTION(vkSubmitDebugUtilsMessageEXT);
} vk_ext_debug_utils;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
typedef struct {
    bool Enabled;
    VK_FUNCTION(vkCreateWin32SurfaceKHR);
} vk_khr_win32_surface;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
typedef struct {
    bool Enabled;
    VK_FUNCTION(vkCreateAndroidSurfaceKHR);
} vk_khr_android_surface;
#elif defined(VK_USE_PLATFORM_METAL_EXT)
typedef struct {
    bool Enabled;
    VK_FUNCTION(vkCreateMetalSurfaceEXT);
} vk_ext_metal_surface;
#else
#error Not Implemented
#endif

typedef struct {
    bool Enabled;
    VK_FUNCTION(vkCreateSwapchainKHR);
    VK_FUNCTION(vkDestroySwapchainKHR);
    VK_FUNCTION(vkGetSwapchainImagesKHR);
    VK_FUNCTION(vkAcquireNextImageKHR);
    VK_FUNCTION(vkQueuePresentKHR);
} vk_khr_swapchain;

typedef struct {
    VK_FUNCTION(vkEnumerateInstanceExtensionProperties);
    VK_FUNCTION(vkEnumerateInstanceLayerProperties);
    VK_FUNCTION(vkCreateInstance);
} vk_global_funcs;

typedef struct {
    VK_FUNCTION(vkEnumeratePhysicalDevices);
    VK_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
    VK_FUNCTION(vkEnumerateDeviceExtensionProperties);
    VK_FUNCTION(vkCreateDevice);
    VK_FUNCTION(vkGetPhysicalDeviceProperties);
    VK_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
    VK_FUNCTION(vkGetDeviceQueue);
    VK_FUNCTION(vkDestroyInstance);

    vk_khr_surface SurfaceKHR;
    vk_khr_get_physical_device_properties2 GetPhysicalDeviceProperties2KHR;
    vk_ext_debug_utils  DebugUtilsEXT;
    vk_ext_debug_report DebugReportEXT;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    vk_khr_win32_surface Win32SurfaceKHR;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    vk_khr_android_surface AndroidSurfaceKHR;
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    vk_ext_metal_surface MetalSurfaceEXT;
#else
#error Not Implemented
#endif
} vk_instance_funcs;

typedef struct {
    VK_FUNCTION(vkCreateCommandPool);
    VK_FUNCTION(vkAllocateCommandBuffers);
    VK_FUNCTION(vkCreateFence);
    VK_FUNCTION(vkCreateSemaphore);
    VK_FUNCTION(vkResetFences);
    VK_FUNCTION(vkResetCommandPool);
    VK_FUNCTION(vkBeginCommandBuffer);
    VK_FUNCTION(vkEndCommandBuffer);
    VK_FUNCTION(vkQueueSubmit);
    VK_FUNCTION(vkGetFenceStatus);
    VK_FUNCTION(vkWaitForFences);
    VK_FUNCTION(vkCreateRenderPass);
    VK_FUNCTION(vkCreateImageView);
    VK_FUNCTION(vkCreateFramebuffer);
    VK_FUNCTION(vkCmdPipelineBarrier);
    VK_FUNCTION(vkCmdBeginRenderPass);
    VK_FUNCTION(vkCmdEndRenderPass);
    VK_FUNCTION(vkCreateBuffer);
    VK_FUNCTION(vkGetBufferMemoryRequirements);
    VK_FUNCTION(vkAllocateMemory);
    VK_FUNCTION(vkBindBufferMemory);
    VK_FUNCTION(vkMapMemory);
    VK_FUNCTION(vkCmdCopyBuffer);
    VK_FUNCTION(vkCreatePipelineLayout);
    VK_FUNCTION(vkCreateShaderModule);
    VK_FUNCTION(vkCreateGraphicsPipelines);
    VK_FUNCTION(vkCmdSetViewport);
    VK_FUNCTION(vkCmdSetScissor);
    VK_FUNCTION(vkCmdBindPipeline);
    VK_FUNCTION(vkCmdBindVertexBuffers);
    VK_FUNCTION(vkCmdBindIndexBuffer);
    VK_FUNCTION(vkCmdDrawIndexed);
    VK_FUNCTION(vkCreateDescriptorSetLayout);
    VK_FUNCTION(vkCreateDescriptorPool);
    VK_FUNCTION(vkAllocateDescriptorSets);
    VK_FUNCTION(vkUpdateDescriptorSets);
    VK_FUNCTION(vkCmdBindDescriptorSets);
    VK_FUNCTION(vkCreateImage);
    VK_FUNCTION(vkGetImageMemoryRequirements);
    VK_FUNCTION(vkBindImageMemory);
    VK_FUNCTION(vkDestroyShaderModule);
    VK_FUNCTION(vkCreateSampler);
    VK_FUNCTION(vkCmdCopyBufferToImage);
    VK_FUNCTION(vkCmdDraw);
    VK_FUNCTION(vkDeviceWaitIdle);
    VK_FUNCTION(vkResetDescriptorPool);
    VK_FUNCTION(vkDestroySampler);
    VK_FUNCTION(vkDestroyImageView);
    VK_FUNCTION(vkFreeMemory);
    VK_FUNCTION(vkDestroyImage);
    VK_FUNCTION(vkDestroyFramebuffer);
    VK_FUNCTION(vkDestroyDevice);
    VK_FUNCTION(vkDestroyRenderPass);
    VK_FUNCTION(vkDestroyCommandPool);
    VK_FUNCTION(vkDestroySemaphore);
    VK_FUNCTION(vkDestroyPipelineLayout);
    VK_FUNCTION(vkDestroyPipeline);
    VK_FUNCTION(vkDestroyBuffer);
    VK_FUNCTION(vkUnmapMemory);
    VK_FUNCTION(vkDestroyDescriptorSetLayout);
    VK_FUNCTION(vkDestroyDescriptorPool);
    VK_FUNCTION(vkFreeDescriptorSets);
    VK_FUNCTION(vkDestroyFence);

    vk_khr_swapchain SwapchainKHR;
}  vk_device_funcs;

typedef struct {
    bool SurfaceKHR;
    bool GetPhysicalDeviceProperties2KHR;
    bool DebugUtilsEXT;
    bool DebugReportEXT;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    bool Win32SurfaceKHR;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    bool AndroidSurfaceKHR;
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    bool MetalSurfaceEXT;
#else
    #error Not Implemented
#endif
} vk_instance_extension_support;

typedef struct {
    bool SwapchainKHR;
} vk_device_extension_support;

typedef struct vk_loader vk_loader;

#define VK_LOADER_LOAD_GLOBAL_FUNCS_DEFINE(name) const vk_global_funcs* name(vk_loader* _Loader)
#define VK_LOADER_LOAD_INSTANCE_FUNCS_DEFINE(name) const vk_instance_funcs* name(vk_loader* _Loader, VkInstance Instance, vk_instance_extension_support* InstanceInfo)
#define VK_LOADER_LOAD_DEVICE_FUNCS_DEFINE(name) const vk_device_funcs* name(vk_loader* _Loader, VkInstance Instance, VkDevice Device, vk_device_extension_support* DeviceInfo)

typedef VK_LOADER_LOAD_GLOBAL_FUNCS_DEFINE(vk_loader_load_global_funcs);
typedef VK_LOADER_LOAD_INSTANCE_FUNCS_DEFINE(vk_loader_load_instance_funcs);
typedef VK_LOADER_LOAD_DEVICE_FUNCS_DEFINE(vk_loader_load_device_funcs);

typedef struct {
    vk_loader_load_global_funcs*   Load_Global_Funcs;
    vk_loader_load_instance_funcs* Load_Instance_Funcs;
    vk_loader_load_device_funcs*   Load_Device_Funcs;
} vk_loader_vtable;

struct vk_loader {
    vk_loader_vtable* VTable;
};

#define VK_Loader_Load_Global_Funcs(Loader) (Loader)->VTable->Load_Global_Funcs(Loader)
#define VK_Loader_Load_Instance_Funcs(Loader, ...) (Loader)->VTable->Load_Instance_Funcs(Loader, __VA_ARGS__)
#define VK_Loader_Load_Device_Funcs(Loader, ...) (Loader)->VTable->Load_Device_Funcs(Loader, __VA_ARGS__)

VKLOADERDEF vk_loader* VK_Get_Loader();

#endif