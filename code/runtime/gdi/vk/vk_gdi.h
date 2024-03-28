#ifndef VK_GDI_H
#define VK_GDI_H

#include <engine.h>

#if defined(OS_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(OS_ANDROID)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(OS_OSX)
#define VK_USE_PLATFORM_METAL_EXT
#else
#error "Not Implemented"
#endif

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include "loader/vk_loader.h"
#include "vk_functions.h"
#include "vk_memory.h"
#include "vk_resource.h"
#include "vk_render_pass.h"
#include "vk_texture.h"
#include "vk_swapchain.h"

struct vk_device {
    VkPhysicalDevice                 Device;
    u32                              GraphicsQueueFamilyIndex;
    u32                              PresentQueueFamilyIndex;
    vk_device_extension_support      DeviceInfo;
    VkPhysicalDevice                 PhysicalDevice;
    VkPhysicalDeviceProperties       Properties;
    VkPhysicalDeviceMemoryProperties MemoryProperties;
    string                           Name;
};

struct vk_frame_context {
    VkFence Fence;
};

struct gdi_context {
    arena*                  Arena;
    ak_job_queue*           ResourceQueue;
    gdi*                    GDI;
    vk_device*              PhysicalDevice;
    VkAllocationCallbacks*  VKAllocator;
    VkDevice                Device;
    const vk_device_funcs*  DeviceFuncs;
    VkQueue                 GraphicsQueue;
    VkQueue                 PresentQueue;
    u64                     TotalFramesRendered;
    array<vk_frame_context> Frames;

    vk_resource_manager<vk_render_pass>  RenderPassManager;
    vk_resource_manager<vk_texture>      TextureManager;
    vk_resource_manager<vk_texture_view> TextureViewManager;
    vk_resource_manager<vk_framebuffer>  FramebufferManager;
    vk_resource_manager<vk_swapchain>    SwapchainManager;
};

struct gdi {
    heap*                    MainHeap;
    lock_allocator*          MainAllocator;
    arena*                   Arena;
    gdi_logging_callbacks    LogCallbacks;
    VkAllocationCallbacks    VKAllocator;
    vk_loader*               Loader;
    const vk_global_funcs*   GlobalFuncs;
    VkInstance               Instance;
    const vk_instance_funcs* InstanceFuncs;
    array<vk_device>         Devices;

    #ifdef DEBUG_BUILD
    VkDebugReportCallbackEXT DebugReportCallback;
    VkDebugUtilsMessengerEXT DebugMessenger;
    #endif
};

#include "vk_resource_manager.inl"

vk_frame_context* VK_Get_Current_Frame_Context(gdi_context* Context);

#endif