#include "vk_gdi.h"

#if defined(DEBUG_BUILD)
internal VkBool32 VKAPI_PTR VK_Debug_Util_Callback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageTypes, 
                                                   const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData) {
    gdi* GDI = (gdi*)UserData;
    if(MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        if(GDI->LogCallbacks.LogWarning)
            GDI->LogCallbacks.LogWarning(GDI, string(CallbackData->pMessage));
    }

    if(MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        if(GDI->LogCallbacks.LogError)
            GDI->LogCallbacks.LogError(GDI, string(CallbackData->pMessage));
    }

    return VK_FALSE;
}

internal VkBool32 VKAPI_PTR VK_Debug_Report_Callback(VkDebugReportFlagsEXT Flags, VkDebugReportObjectTypeEXT ObjectType, uint64_t Object, size_t Location,
                                                   int32_t MessageCode, const char* LayerPrefix, const char* Message, void* UserData) {
    gdi* GDI = (gdi*)UserData;
    if((Flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) || (Flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)) {
        if(GDI->LogCallbacks.LogWarning)
            GDI->LogCallbacks.LogWarning(GDI, string(Message));
    }

    if(Flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        if(GDI->LogCallbacks.LogError)
            GDI->LogCallbacks.LogError(GDI, string(Message));
    }
    //todo: Log
    Invalid_Code();
    return VK_FALSE;
}
#endif

typedef struct {
    uptr ByteSize;
} vk_aligned_block;

internal void* VKAPI_PTR VK__Alloc(void* UserData, uptr ByteSize, uptr Alignment, VkSystemAllocationScope AllocationScope) {
    Assert(Alignment > 0 && Is_Pow2(Alignment));

    allocator* Allocator = (allocator*)UserData;

    uptr Offset = Alignment - 1 + sizeof(void*);
    vk_aligned_block* Block = (vk_aligned_block*)Allocator_Allocate_Memory_Internal(Allocator, ByteSize+Offset+sizeof(vk_aligned_block));
    if(!Block) return NULL;

    Block->ByteSize = ByteSize;

    void* P1  = Block+1;
    void** P2 = (void**)(((uptr)(P1) + Offset) & ~(Alignment - 1));
    P2[-1] = P1;
        
    return P2;
}

internal void* VKAPI_PTR VK__Realloc(void* UserData, void* Original, uptr Size, uptr Alignment, VkSystemAllocationScope AllocationScope) {
    if(!Original) return VK__Alloc(UserData, Size, Alignment, AllocationScope);
    Assert(Alignment > 0 && Is_Pow2(Alignment));
    
    allocator* Allocator = (allocator*)UserData;
    
    void* OriginalUnaligned = ((void**)Original)[-1];
    vk_aligned_block* OriginalBlock = ((vk_aligned_block*)OriginalUnaligned)-1;

    uptr Offset = Alignment - 1 + sizeof(void*);

    vk_aligned_block* NewBlock = (vk_aligned_block*)Allocator_Allocate_Memory_Internal(Allocator, Size+Offset+sizeof(vk_aligned_block));
    NewBlock->ByteSize = Size;

    void* P1  = NewBlock+1;
    void** P2 = (void**)(((uptr)(P1) + Offset) & ~(Alignment - 1));
    P2[-1] = P1;

    Memory_Copy(P2, Original, Min(NewBlock->ByteSize, OriginalBlock->ByteSize));

    Allocator_Free_Memory_Internal(Allocator, OriginalBlock);

    return P2;
}

internal void VKAPI_PTR VK__Free(void* UserData, void* Memory) {
    if(Memory) {
        allocator* Allocator = (allocator*)UserData;
        void* OriginalUnaligned = ((void**)Memory)[-1];
        vk_aligned_block* Block = ((vk_aligned_block*)OriginalUnaligned)-1;
        Allocator_Free_Memory_Internal(Allocator, Block);
    }
}

internal VkFormat VK_Get_Format(gdi_format Format) {
    static VkFormat VKFormats[] = {
        VK_FORMAT_UNDEFINED,
        VK_FORMAT_R8G8_UNORM,
        VK_FORMAT_R8G8B8_UNORM,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_FORMAT_R16_UINT,
        VK_FORMAT_R32_UINT,
        VK_FORMAT_R32_SFLOAT,
        VK_FORMAT_R32G32_SFLOAT,
        VK_FORMAT_R32G32B32_SFLOAT,
        VK_FORMAT_D16_UNORM,
    };
    static_assert(Array_Count(VKFormats) == GDI_FORMAT_COUNT);
    Assert(Format < GDI_FORMAT_COUNT);
    return VKFormats[Format];
}

internal gdi_format VK_Get_GDI_Format_Unsafe(VkFormat Format) {
    switch(Format) {
        case VK_FORMAT_UNDEFINED:
        return GDI_FORMAT_NONE;

        case VK_FORMAT_R8G8_UNORM:
        return GDI_FORMAT_R8G8_UNORM;

        case VK_FORMAT_R8G8B8_UNORM:
        return GDI_FORMAT_R8G8B8_UNORM;

        case VK_FORMAT_R8G8B8A8_UNORM:
        return GDI_FORMAT_R8G8B8A8_UNORM;

        case VK_FORMAT_R8G8B8A8_SRGB:
        return GDI_FORMAT_R8G8B8A8_SRGB;

        case VK_FORMAT_B8G8R8A8_UNORM:
        return GDI_FORMAT_B8G8R8A8_UNORM;

        case VK_FORMAT_B8G8R8A8_SRGB:
        return GDI_FORMAT_B8G8R8A8_SRGB;

        case VK_FORMAT_R16G16B16A16_SFLOAT:
        return GDI_FORMAT_R16G16B16A16_FLOAT;

        case VK_FORMAT_R16_UINT:
        return GDI_FORMAT_R16_UINT;

        case VK_FORMAT_R32_UINT:
        return GDI_FORMAT_R32_UINT;

        case VK_FORMAT_R32_SFLOAT:
        return GDI_FORMAT_R32_FLOAT;

        case VK_FORMAT_R32G32_SFLOAT:
        return GDI_FORMAT_R32G32_FLOAT;

        case VK_FORMAT_R32G32B32_SFLOAT:
        return GDI_FORMAT_R32G32B32_FLOAT;

        case VK_FORMAT_D16_UNORM:
        return GDI_FORMAT_D16_UNORM;
    }

    return GDI_FORMAT_NONE;
}

internal gdi_format VK_Get_GDI_Format(VkFormat Format) {
    gdi_format Result = VK_Get_GDI_Format_Unsafe(Format);
    Assert(Result != GDI_FORMAT_NONE);
    return Result;
}

internal VkImageUsageFlags VK_Convert_To_Image_Usage_Flags(gdi_texture_usage_flags Flags) {
    VkImageUsageFlags Result = 0;

    if(Flags & GDI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT_BIT) {
        Result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    if(Flags & GDI_TEXTURE_USAGE_FLAG_DEPTH_ATTACHMENT_BIT) {
        Result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    if(Flags & GDI_TEXTURE_USAGE_FLAG_SAMPLED_BIT) {
        Result |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    return Result;
}

internal bool VK_Create__Internal(gdi* GDI, const gdi_create_info& CreateInfo) {
    GDI->Arena = Arena_Create(GDI->MainAllocator);
    GDI->VKAllocator = {
        .pUserData = GDI->MainAllocator,
        .pfnAllocation = VK__Alloc,
        .pfnReallocation = VK__Realloc,
        .pfnFree = VK__Free
    };
    GDI->Loader = VK_Get_Loader();
    VK_Load_Global_Funcs(GDI);

    u32 PropertyCount;
    vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &PropertyCount, VK_NULL_HANDLE);

    scratch Scratch = Scratch_Get();
    VkExtensionProperties* ExtensionProps = Scratch_Push_Array(&Scratch, PropertyCount, VkExtensionProperties);
    vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &PropertyCount, ExtensionProps);

    vk_instance_extension_support InstanceExtensions = {0};

    array<const char*> Extensions(&Scratch, PropertyCount);

    for(u32 PropertyIndex = 0; PropertyIndex < PropertyCount; PropertyIndex++) {
        string ExtensionName = ExtensionProps[PropertyIndex].extensionName;

        if(ExtensionName == String_Lit(VK_KHR_SURFACE_EXTENSION_NAME)) {
            InstanceExtensions.SurfaceKHR = true;
            Array_Push(&Extensions, ExtensionName.Str);
        } else if(ExtensionName == String_Lit(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
            InstanceExtensions.GetPhysicalDeviceProperties2KHR = true;
            Array_Push(&Extensions, ExtensionName.Str);
        } else if(VK_Is_Surface_Extension(ExtensionName)) {
            VK_Set_Surface_Extension(&InstanceExtensions);
            Array_Push(&Extensions, ExtensionName.Str);
        }
#ifdef DEBUG_BUILD
        else if(ExtensionName == String_Lit(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            InstanceExtensions.DebugUtilsEXT = true;
            Array_Push(&Extensions, ExtensionName.Str);
        } else if(ExtensionName == String_Lit(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
            InstanceExtensions.DebugReportEXT = true;
            Array_Push(&Extensions, ExtensionName.Str);
        }
#endif
    }

    if(!InstanceExtensions.SurfaceKHR || !VK_Has_Surface_Extension(&InstanceExtensions)) {
        Log_Error(modules::Vulkan, "Vulkan surface extensions not found!");
        return false;
    }

#if defined(VK_ENABLE_BETA_EXTENSIONS)
    Array_Push(&Extensions, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    array<const char*> Layers;
#if defined(DEBUG_BUILD)
    u32 LayerCount;
    vkEnumerateInstanceLayerProperties(&LayerCount, VK_NULL_HANDLE);
    
    VkLayerProperties* LayerProps = Scratch_Push_Array(&Scratch, LayerCount, VkLayerProperties);
    vkEnumerateInstanceLayerProperties(&LayerCount, LayerProps);

    Layers = array<const char*>(&Scratch, LayerCount);
    for(u32 LayerIndex = 0; LayerIndex < LayerCount; LayerIndex++) {
        string LayerName = string(LayerProps[LayerIndex].layerName);
        if(LayerName == String_Lit("VK_LAYER_KHRONOS_validation")) {
            Array_Push(&Layers, LayerName.Str);
        }
    }
#endif

    const gdi_app_info* AppInfo = &CreateInfo.AppInfo; 

    VkApplicationInfo ApplicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = AppInfo->Name.Str,
        .applicationVersion = VK_MAKE_API_VERSION(0, AppInfo->Version.Major, AppInfo->Version.Minor, AppInfo->Version.Patch),
        .pEngineName = "AK_Engine",
        .engineVersion = VK_MAKE_API_VERSION(0, AK_ENGINE_MAJOR_VERSION, AK_ENGINE_MINOR_VERSION, AK_ENGINE_PATCH_VERSION),
        .apiVersion = VK_API_VERSION_1_0
    };

    VkInstanceCreateInfo InstanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &ApplicationInfo,
        .enabledLayerCount = Safe_U32(Layers.Count),
        .ppEnabledLayerNames = Layers.Ptr,
        .enabledExtensionCount = Safe_U32(Extensions.Count),
        .ppEnabledExtensionNames = Extensions.Ptr
    };

    if(vkCreateInstance(&InstanceCreateInfo, &GDI->VKAllocator, &GDI->Instance) != VK_SUCCESS) {
        Log_Error(modules::Vulkan, "Failed to create the vulkan instance!");
        return false;
    } 

    VK_Load_Instance_Funcs(GDI, &InstanceExtensions);

#ifdef DEBUG_BUILD
    //Create the debug utils after the instance has been created
    if(GDI->InstanceFuncs->DebugUtilsEXT.Enabled) {
        const vk_ext_debug_utils* DebugUtilsEXT = &GDI->InstanceFuncs->DebugUtilsEXT;
        VkDebugUtilsMessengerCreateInfoEXT DebugUtilsInfo = {};
        DebugUtilsInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        DebugUtilsInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; 
        DebugUtilsInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT; 
        DebugUtilsInfo.pfnUserCallback = VK_Debug_Util_Callback; 
        DebugUtilsInfo.pUserData = GDI;

        if (DebugUtilsEXT->vkCreateDebugUtilsMessengerEXT(GDI->Instance, &DebugUtilsInfo, &GDI->VKAllocator, &GDI->DebugMessenger) != VK_SUCCESS) {
            Log_Warning(modules::Vulkan, "Failed to create the vulkan debug messenger!");
        }
    } else if(GDI->InstanceFuncs->DebugReportEXT.Enabled) {
        const vk_ext_debug_report* DebugReportEXT = &GDI->InstanceFuncs->DebugReportEXT;
        VkDebugReportCallbackCreateInfoEXT DebugReportInfo = {};
        DebugReportInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        DebugReportInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | 
                                VK_DEBUG_REPORT_ERROR_BIT_EXT;
        DebugReportInfo.pfnCallback = VK_Debug_Report_Callback;  
        DebugReportInfo.pUserData = GDI;

        if (DebugReportEXT->vkCreateDebugReportCallbackEXT(GDI->Instance, &DebugReportInfo, &GDI->VKAllocator, &GDI->DebugReportCallback) != VK_SUCCESS) {
            Log_Warning(modules::Vulkan, "Failed to create the vulkan debug report callback!");
        }
    }
#endif

    VkSurfaceKHR TempSurface = VK_Create_Temp_Surface(GDI);

    u32 PhysicalDeviceCount;
    vkEnumeratePhysicalDevices(GDI->Instance, &PhysicalDeviceCount, VK_NULL_HANDLE);

    VkPhysicalDevice* PhysicalDevices = Scratch_Push_Array(&Scratch, PhysicalDeviceCount, VkPhysicalDevice);
    vkEnumeratePhysicalDevices(GDI->Instance, &PhysicalDeviceCount, PhysicalDevices);

    array<vk_device>* Devices = &GDI->Devices;
    Array_Init(Devices, GDI->Arena, PhysicalDeviceCount);

    for(u32 PhysicalDeviceIndex = 0; PhysicalDeviceIndex < PhysicalDeviceCount; PhysicalDeviceIndex++) {
        VkPhysicalDevice PhysicalDevice = PhysicalDevices[PhysicalDeviceIndex];

        vkEnumerateDeviceExtensionProperties(PhysicalDevice, VK_NULL_HANDLE, &PropertyCount, VK_NULL_HANDLE);
        ExtensionProps = Scratch_Push_Array(&Scratch, PropertyCount, VkExtensionProperties);
        vkEnumerateDeviceExtensionProperties(PhysicalDevice, VK_NULL_HANDLE, &PropertyCount, ExtensionProps);

        vk_device_extension_support DeviceExtensions = {0};

        for(u32 PropertyIndex = 0; PropertyIndex < PropertyCount; PropertyIndex++) {
            string ExtensionName = ExtensionProps[PropertyIndex].extensionName;
            if(ExtensionName == String_Lit(VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
                DeviceExtensions.SwapchainKHR = true;
            }
        }

        if(!DeviceExtensions.SwapchainKHR) {
            continue;
        }

        u32 QueueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, VK_NULL_HANDLE);

        VkQueueFamilyProperties* QueueFamilyProps = Scratch_Push_Array(&Scratch, QueueFamilyCount, VkQueueFamilyProperties);
        vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, QueueFamilyProps);

        u32 GraphicsQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        u32 PresentQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        for(u32 QueueFamilyIndex = 0; QueueFamilyIndex < QueueFamilyCount; QueueFamilyIndex++) {
            if(QueueFamilyProps[QueueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                GraphicsQueueFamilyIndex = QueueFamilyIndex;
            }
        }

        if(GraphicsQueueFamilyIndex != VK_QUEUE_FAMILY_IGNORED) {
            VkBool32 PresentIsSupported;
            vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, GraphicsQueueFamilyIndex, TempSurface, &PresentIsSupported);
            
            if(PresentIsSupported) {
                PresentQueueFamilyIndex = GraphicsQueueFamilyIndex;
            } else {
                for(u32 QueueFamilyIndex = 0; QueueFamilyIndex < QueueFamilyCount; QueueFamilyIndex++) {
                    vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, QueueFamilyIndex, TempSurface, &PresentIsSupported);
                    if(PresentIsSupported) {
                        PresentQueueFamilyIndex = QueueFamilyIndex;
                        break;
                    }
                }
            }

            if(PresentQueueFamilyIndex != VK_QUEUE_FAMILY_IGNORED) {
                VkPhysicalDeviceProperties Properties;
                VkPhysicalDeviceMemoryProperties MemoryProperties;

                vkGetPhysicalDeviceProperties(PhysicalDevice, &Properties);
                vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemoryProperties);

                Array_Push(Devices, {
                    .Device = PhysicalDevice,
                    .GraphicsQueueFamilyIndex = GraphicsQueueFamilyIndex,
                    .PresentQueueFamilyIndex = PresentQueueFamilyIndex,
                    .DeviceInfo = DeviceExtensions,
                    .PhysicalDevice = PhysicalDevice,
                    .Properties = Properties,
                    .MemoryProperties = MemoryProperties
                });
            }
        }
    }

    vkDestroySurfaceKHR(GDI->Instance, TempSurface, &GDI->VKAllocator);
    
    if(!Devices->Count) {
        Log_Error(modules::Vulkan, "No valid vulkan devices!");
        //todo: Logging
        return false;
    }

    return true;
}


bool GDI_Create_Context__Internal(gdi_context* Context, gdi* GDI, const gdi_context_create_info& CreateInfo) {
    Assert(CreateInfo.ResourceQueue);
    
    vk_device* Device = &GDI->Devices[CreateInfo.DeviceIndex];

    f32 Priority = 1.0f;
    u32 DeviceQueueCreateCount = 1;
    VkDeviceQueueCreateInfo DeviceQueueCreateInfos[2] = { {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = Device->GraphicsQueueFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = &Priority
    }};

    if(Device->PresentQueueFamilyIndex != Device->GraphicsQueueFamilyIndex) {
        DeviceQueueCreateCount = 2; 
        DeviceQueueCreateInfos[1] = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = Device->GraphicsQueueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &Priority
        };
    }

    u32 MaxExtensionCount;
    vkEnumerateDeviceExtensionProperties(Device->PhysicalDevice, VK_NULL_HANDLE, &MaxExtensionCount, VK_NULL_HANDLE);

    scratch Scratch = Scratch_Get();
    array<const char*> Extensions(&Scratch, MaxExtensionCount);

    vk_device_extension_support* DeviceInfo = &Device->DeviceInfo;

    Assert(DeviceInfo->SwapchainKHR);
    Array_Push(&Extensions, (const char*)VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#ifdef VK_ENABLE_BETA_EXTENSIONS
    Array_Push(&Extensions, (const char*)VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

    VkDeviceCreateInfo DeviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = DeviceQueueCreateCount,
        .pQueueCreateInfos = DeviceQueueCreateInfos,
        .enabledExtensionCount = Safe_U32(Extensions.Count),
        .ppEnabledExtensionNames = Extensions.Ptr
    };

    Context->ResourceQueue = CreateInfo.ResourceQueue;
    Context->GDI = GDI;
    Context->PhysicalDevice = Device;
    Context->VKAllocator = &GDI->VKAllocator;
    if(vkCreateDevice(Device->PhysicalDevice, &DeviceCreateInfo, Context->VKAllocator, &Context->Device) != VK_SUCCESS) {
        //todo: Logging
        return false;
    }

    vkGetDeviceQueue(Context->Device, Device->GraphicsQueueFamilyIndex, 0, &Context->GraphicsQueue);
    if(Device->GraphicsQueueFamilyIndex != Device->PresentQueueFamilyIndex) {
        vkGetDeviceQueue(Context->Device, Device->PresentQueueFamilyIndex, 0, &Context->PresentQueue);
    } else {
        Context->PresentQueue = Context->GraphicsQueue;
    }

    VK_Load_Device_Funcs(GDI, Context);

    VK_Resource_Manager_Create(&Context->TextureManager, Context, {
        .ResourceSize = sizeof(vk_texture),
        .MaxCount = CreateInfo.TextureCount,
        .AllocateCallback = VK_Texture_Allocate_Callback,
        .FreeCallback = VK_Texture_Free_Callback
    });

    VK_Resource_Manager_Create(&Context->SwapchainManager, Context, {
        .ResourceSize = sizeof(vk_swapchain),
        .MaxCount = CreateInfo.SwapchainCount,
        .AllocateCallback = VK_Swapchain_Allocate_Callback,
        .FreeCallback = VK_Swapchain_Free_Callback
    });

    return true;
}

vk_frame_context* VK_Get_Current_Frame_Context(gdi_context* Context) {
    return &Context->Frames[Context->CurrentFrameIndex];
}

gdi* GDI_Create(const gdi_create_info& CreateInfo) {
    heap* Heap         = Heap_Create(Core_Get_Base_Allocator());
    gdi*  GDI          = Allocator_Allocate_Struct(Heap, gdi);
    GDI->MainHeap      = Heap;
    GDI->MainAllocator = Lock_Allocator_Create(GDI->MainHeap);
    Heap_Track(GDI->MainHeap, String_Lit("Main GDI Heap"));

    if(!VK_Create__Internal(GDI, CreateInfo)) {
        GDI_Delete(GDI);
        return NULL;
    }

    return GDI;
}

void GDI_Delete(gdi* GDI) {
    if(GDI && GDI->MainHeap) {
#ifdef DEBUG_BUILD
        if(GDI->DebugReportCallback) {
            const vk_ext_debug_report* DebugReportEXT = &GDI->InstanceFuncs->DebugReportEXT;
            DebugReportEXT->vkDestroyDebugReportCallbackEXT(GDI->Instance, GDI->DebugReportCallback, &GDI->VKAllocator);
            GDI->DebugReportCallback = VK_NULL_HANDLE;
        }

        if(GDI->DebugMessenger) {
            const vk_ext_debug_utils* DebugUtilsEXT = &GDI->InstanceFuncs->DebugUtilsEXT;
            DebugUtilsEXT->vkDestroyDebugUtilsMessengerEXT(GDI->Instance, GDI->DebugMessenger, &GDI->VKAllocator);
            GDI->DebugMessenger = VK_NULL_HANDLE;
        }
#endif

        if(GDI->Instance) {
            vkDestroyInstance(GDI->Instance, &GDI->VKAllocator);
            GDI->Instance = NULL;
        }

        if(GDI->MainAllocator) {
            Lock_Allocator_Delete(GDI->MainAllocator);
            GDI->MainAllocator = NULL;
        }

        Heap_Delete(GDI->MainHeap);
    }
}

u32 GDI_Get_Device_Count(gdi* GDI) {
    return Safe_U32(GDI->Devices.Count);
}

void GDI_Get_Device(gdi* GDI, gdi_device* Device, u32 DeviceIndex) {
    Device->Name = string(GDI->Devices[DeviceIndex].Properties.deviceName);
}

gdi_context* GDI_Create_Context(gdi* GDI, const gdi_context_create_info& CreateInfo) {
    arena* Arena = Arena_Create(GDI->MainAllocator);
    gdi_context* Result = Arena_Push_Struct(Arena, gdi_context);
    Result->Arena = Arena;
    if(!GDI_Create_Context__Internal(Result, GDI, CreateInfo)) {
        GDI_Context_Delete(Result);
        return NULL;
    }

    return Result;
}

void GDI_Context_Delete(gdi_context* Context) {
    if(Context && Context->Arena) {
        if(Context->Device) {
            vkDestroyDevice(Context->Device, Context->VKAllocator);
            Context->Device = VK_NULL_HANDLE;
        }
        Arena_Delete(Context->Arena);
    }
}


array<gdi_format> GDI_Context_Supported_Window_Formats(gdi_context* Context, const gdi_window_data& WindowData, arena* Arena) {
    VkSurfaceKHR Surface = VK_Create_Surface(Context->GDI, &WindowData);
    if(!Surface) {
        return {};
    }

    u32 SurfaceFormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Context->PhysicalDevice->Device, Surface, &SurfaceFormatCount, VK_NULL_HANDLE);

    scratch Scratch = Scratch_Get();
    VkSurfaceFormatKHR* SurfaceFormats = Scratch_Push_Array(&Scratch, SurfaceFormatCount, VkSurfaceFormatKHR);
    vkGetPhysicalDeviceSurfaceFormatsKHR(Context->PhysicalDevice->Device, Surface, &SurfaceFormatCount, SurfaceFormats);

    array<gdi_format> Formats(Arena, SurfaceFormatCount);
    for(u32 i = 0; i < SurfaceFormatCount; i++) {
        gdi_format Format = VK_Get_GDI_Format_Unsafe(SurfaceFormats[i].format);
        if(Format != GDI_FORMAT_NONE) {
            Array_Push(&Formats, Format);
        }
    }
    
    vkDestroySurfaceKHR(Context->GDI->Instance, Surface, Context->VKAllocator);
    return Formats;
}

gdi_swapchain GDI_Context_Create_Swapchain(gdi_context* Context, const gdi_swapchain_create_info& CreateInfo) {
    resource_id ResourceID = Resource_Manager_Allocate(&Context->SwapchainManager, &CreateInfo, sizeof(gdi_context_create_info));
    return (gdi_swapchain)ResourceID;
}

void GDI_Context_Delete_Swapchain(gdi_context* Context, gdi_swapchain SwapchainID) {
    if(SwapchainID) {
        resource_id ResourceID = (resource_id)SwapchainID;
        vk_swapchain_delete_info DeleteInfo = {};
        Resource_Manager_Free(&Context->SwapchainManager, ResourceID, &DeleteInfo, sizeof(vk_swapchain_delete_info));
    }
}

void GDI_Context_Resize_Swapchain(gdi_context* Context, gdi_swapchain SwapchainID) {
    if(SwapchainID) {
        resource_id ResourceID = (resource_id)SwapchainID;
        vk_swapchain* Swapchain = (vk_swapchain*)Resource_Manager_Get_Resource_No_Callback(&Context->SwapchainManager, ResourceID);
        if(Swapchain) {
            gdi_format TargetFormat            = Swapchain->Format;
            gdi_texture_usage_flags UsageFlags = Swapchain->UsageFlags;

            vk_swapchain_delete_info DeleteInfo = {
                .PartialDelete = true //Only remove the images
            };

            Resource_Manager_Dispatch_Free(&Context->SwapchainManager, ResourceID, &DeleteInfo, sizeof(vk_swapchain_delete_info));
            
            gdi_swapchain_create_info CreateInfo = {
                .TargetFormat = TargetFormat,
                .UsageFlags = UsageFlags
            };
            
            Resource_Manager_Dispatch_Allocate(&Context->SwapchainManager, ResourceID, &CreateInfo, sizeof(gdi_context_create_info));
        }
    }
}

#include "vk_swapchain.cpp"
#include "vk_texture.cpp"
#include "vk_resource.cpp"
#include "vk_functions.cpp"

#include <resources/resource.cpp>