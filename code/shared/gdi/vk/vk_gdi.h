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

#include <gdi/gdi_shared.h>

struct vk_sampler;
struct vk_texture_view;

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include "loader/vk_loader.h"
#include "vk_functions.h"
#include "vk_memory.h"
#include "vk_pipeline.h"
#include "vk_bind_groups.h"
#include "vk_render_pass.h"
#include "vk_texture.h"
#include "vk_buffer.h"
#include "vk_swapchain.h"

template <typename type>
struct vk_delete_list_entry {
    u64  LastUsedFrameIndex;
    type Resource;
};

template <typename type>
struct vk_delete_list {
    array<vk_delete_list_entry<type>> List;

    vk_delete_list_entry<type>* begin() { return List.begin(); }
    vk_delete_list_entry<type>* end() { return List.end(); }
};

#define VK_UPLOAD_BUFFER_MINIMUM_BLOCK_SIZE MB(4)
struct vk_upload_buffer_block {
    VkBuffer                Buffer;
    vk_allocation           Allocation;
    buffer                  Data;
    uptr                    Used;
    vk_upload_buffer_block* Next;
};

struct vk_upload_buffer {
    arena*                  Arena;
    VkDevice                Device;
    VkAllocationCallbacks*  VKAllocator;
    vk_memory_manager*      MemoryManager;
    vk_upload_buffer_block* First;
    vk_upload_buffer_block* Last;
    vk_upload_buffer_block* Current;
};

struct vk_copy_upload_to_buffer {
    vk_upload               Upload;
    async_handle<vk_buffer> Buffer;
    VkDeviceSize            Offset;
};

struct vk_copy_upload_to_texture {
    vk_upload                Upload;
    async_handle<vk_texture> Texture;
    u32                      Width;
    u32                      Height;
};

struct vk_copy_context {
    ak_rw_lock                       RWLock;
    u32                              CurrentListIndex;
    array<vk_copy_upload_to_buffer>  CopyUploadToBufferList[2];
    array<vk_copy_upload_to_texture> CopyUploadToTextureList[2];
};

struct vk_delete_context {
    ak_rw_lock                           RWLock;
    u32                                  CurrentListIndex;
    vk_delete_list<vk_pipeline>          PipelineList[2];
    vk_delete_list<vk_bind_group>        BindGroupList[2];
    vk_delete_list<vk_bind_group_layout> BindGroupLayoutList[2];
    vk_delete_list<vk_framebuffer>       FramebufferList[2];
    vk_delete_list<vk_render_pass>       RenderPassList[2];
    vk_delete_list<vk_sampler>           SamplerList[2];
    vk_delete_list<vk_texture_view>      TextureViewList[2];
    vk_delete_list<vk_texture>           TextureList[2];
    vk_delete_list<vk_buffer>            BufferList[2];
    vk_delete_list<vk_swapchain>         SwapchainList[2];
};

struct vk_thread_context {
    vk_delete_context             DeleteContext;
    vk_copy_context               CopyContext;
    fixed_array<vk_upload_buffer> UploadBuffers; //One upload buffer per frame per thread
    vk_thread_context*            Next;
};

struct vk_resource_context {
    async_pool<vk_pipeline>          Pipelines;
    async_pool<vk_bind_group>        BindGroups;
    async_pool<vk_bind_group_layout> BindGroupLayouts;
    async_pool<vk_framebuffer>       Framebuffers;
    async_pool<vk_render_pass>       RenderPasses;
    async_pool<vk_sampler>           Samplers;
    async_pool<vk_texture_view>      TextureViews;
    async_pool<vk_texture>           Textures;
    async_pool<vk_buffer>            Buffers;
    async_pool<vk_swapchain>         Swapchains;

    //TODO: These probably should be atomic u8 but ak atomic 
    //doesn't support those (yet)
    ak_atomic_u32* PipelinesInUse;
    ak_atomic_u32* BindGroupsInUse;
    ak_atomic_u32* BindGroupLayoutsInUse;
    ak_atomic_u32* FramebuffersInUse;
    ak_atomic_u32* RenderPassesInUse;
    ak_atomic_u32* SamplersInUse;
    ak_atomic_u32* TextureViewsInUse;
    ak_atomic_u32* TexturesInUse;
    ak_atomic_u32* BuffersInUse;
    ak_atomic_u32* SwapchainsInUse;

    u64* PipelineLastFrameIndices;
    u64* BindGroupLastFrameIndices;
    u64* BindGroupLayoutLastFrameIndices;
    u64* FramebufferLastFrameIndices;
    u64* RenderPassLastFrameIndices;
    u64* SamplerLastFrameIndices;
    u64* TextureViewLastFrameIndices;
    u64* TextureLastFrameIndices;
    u64* BufferLastFrameIndices;
    u64* SwapchainLastFrameIndices;
};

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

struct vk_cmd_list {
    gdi_context*         Context;
    VkCommandPool        CmdPool;
    VkCommandBuffer      CmdBuffer;
    VkPipelineStageFlags ExecuteLockWaitStage;
    VkSemaphore          SubmitLock;
    VkSemaphore          PresentLock;
    VkSwapchainKHR       Swapchain;
    u32                  SwapchainTextureIndex;
    vk_pipeline*         Pipeline;
    vk_cmd_list*         Next;
    vk_cmd_list*         Prev;
};

struct vk_cmd_pool {
    VkCommandBufferLevel Level;
    vk_cmd_list*         FreeCmdList;
    vk_cmd_list*         CurrentCmdListHead;
    vk_cmd_list*         CurrentCmdListTail;
};

struct vk_frame_context {
    VkCommandPool   CopyCmdPool;
    VkCommandBuffer CopyCmdBuffer;
    VkFence         Fence;
    vk_cmd_pool     PrimaryCmdPool;
    vk_cmd_pool     SecondaryCmdPool;
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

    ak_mutex            ThreadContextLock;
    arena*              ThreadContextArena;
    ak_atomic_ptr       ThreadContextList;
    ak_tls              ThreadContextTLS;
    vk_resource_context ResourceContext;
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

#endif