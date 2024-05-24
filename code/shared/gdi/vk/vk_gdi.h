#ifndef VK_GDI_H
#define VK_GDI_H

/*
todo: 
-There is a chance for swapchain resizing to fail. This is very complicating
 and difficult to reproduce. This might also just be a vulkan validation error
 issue and in release there is no problem. The problem is that when we delete 
 the swapchain we also release the semaphores with the swapchain and recreate 
 them. Vulkan throws a validation error layer saying that the semaphore cannot
 be used in a submitted buffer, even though the fence that submitted the commands
 have been finished. I suspect this is because the fence only signals when swapchains
 is finished in the command buffers and not when the presentation engine is 
 finished waiting on the swapchain. There is some ambiguity with the spec as 
 shown by this post and the links it has: 
 https://stackoverflow.com/questions/75437792/how-to-synchronize-vulkan-swapchain-presentation-with-sempahore-destruction 
 Ultimately the way the semaphores and queue submissions work needs a rework,
 but since this might need to be changed or updated for async copy/compute, ray tracing,
 sparse resources, or different presentation queue families, we can fix this later.
*/

#include <core/core.h>
#include <math/math.h>
#include <gdi/gdi.h>

namespace modules {
    internal const string Vulkan = String_Lit("Vulkan");
};

#if defined(OS_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(OS_ANDROID)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(OS_OSX)
#define VK_USE_PLATFORM_METAL_EXT
#else
#error "Not Implemented"
#endif

#include <gdi/gdi_shared.h>

struct vk_sampler;
struct vk_texture_view;

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include "loader/vk_loader.h"
#include "vk_functions.h"
#include "vk_memory.h"
#include "vk_resource.h"
#include "vk_thread_context.h"

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
    VkCommandBuffer CopyCmdBuffer;
    VkFence         Fence;
};

struct gdi_context {
    arena*                  Arena;
    gdi*                    GDI;
    vk_device*              PhysicalDevice;
    VkAllocationCallbacks*  VKAllocator;
    VkDevice                Device;
    const vk_device_funcs*  DeviceFuncs;
    gdi_context_info        Info;
    VkQueue                 GraphicsQueue;
    VkQueue                 PresentQueue;
    vk_memory_manager       MemoryManager;
    vk_descriptor_pool      DescriptorPool;
    u64                     TotalFramesRendered;
    array<vk_frame_context> Frames;

    vk_thread_context_manager ThreadContextManager;
    vk_resource_context       ResourceContext;
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

struct vk_pipeline_barrier {
    bool                        InUse;
    array<VkImageMemoryBarrier> ImageMemoryBarriers;
};

enum class vk_pipeline_stage {
    None,
    Present,
    Color_Attachment,
    Depth_Write_Attachment,
    Count
};

static const vk_pipeline_stage G_PipelineStages[] = {
    vk_pipeline_stage::None,
    vk_pipeline_stage::Present,
    vk_pipeline_stage::Color_Attachment,
    vk_pipeline_stage::Depth_Write_Attachment
};

static_assert(Array_Count(G_PipelineStages) == GDI_RESOURCE_STATE_COUNT);

static const VkPipelineStageFlags G_PipelineStageMasks[] = {
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT|VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
};

static_assert(Array_Count(G_PipelineStageMasks) == (u32)vk_pipeline_stage::Count);

static const VkAccessFlags G_VKAccessMasks[] = {
    0,
    0,
    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT|VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
};

static_assert(Array_Count(G_VKAccessMasks) == GDI_RESOURCE_STATE_COUNT);

static const VkImageLayout G_VKImageLayouts[] = {
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
};

static_assert(Array_Count(G_VKImageLayouts) == GDI_RESOURCE_STATE_COUNT);

vk_frame_context* VK_Get_Current_Frame_Context(gdi_context* Context);
vk_cmd_list* VK_Allocate_Cmd_List(gdi_context* Context);

#endif