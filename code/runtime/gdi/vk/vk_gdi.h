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

struct gdi_context {
    vk_device*             PhysicalDevice;
    VkDevice               Device;
    const vk_device_funcs* DeviceFuncs;
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

#endif