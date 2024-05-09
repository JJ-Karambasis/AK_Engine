#include "vk_gdi.h"

#if defined(DEBUG_BUILD)
internal VkBool32 VKAPI_PTR VK_Debug_Util_Callback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageTypes, 
                                                   const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData) {
    gdi* GDI = (gdi*)UserData;
    if(MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT ) {
        if(GDI->LogCallbacks.LogDebug)
            GDI->LogCallbacks.LogDebug(GDI, string(CallbackData->pMessage));
    }

    if(MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT ) {
        if(GDI->LogCallbacks.LogInfo)
            GDI->LogCallbacks.LogInfo(GDI, string(CallbackData->pMessage));
    }

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
    
    if(Flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        if(GDI->LogCallbacks.LogDebug)
            GDI->LogCallbacks.LogDebug(GDI, string(Message));
    }

    if(Flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
        if(GDI->LogCallbacks.LogInfo)
            GDI->LogCallbacks.LogInfo(GDI, string(Message));
    }

    if((Flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) || (Flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)) {
        if(GDI->LogCallbacks.LogWarning)
            GDI->LogCallbacks.LogWarning(GDI, string(Message));
    }

    if(Flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        if(GDI->LogCallbacks.LogError)
            GDI->LogCallbacks.LogError(GDI, string(Message));
    }

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

internal VkAttachmentLoadOp VK_Get_Load_Op(gdi_load_op LoadOp) {
    static VkAttachmentLoadOp VKLoadOps[] = {
        VK_ATTACHMENT_LOAD_OP_LOAD,
        VK_ATTACHMENT_LOAD_OP_CLEAR
    };
    static_assert(Array_Count(VKLoadOps) == GDI_LOAD_OP_COUNT);
    Assert(LoadOp < GDI_LOAD_OP_COUNT);
    return VKLoadOps[LoadOp];
}

internal VkAttachmentStoreOp VK_Get_Store_Op(gdi_store_op StoreOp) {
    static VkAttachmentStoreOp VKStoreOps[] = {
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE
    };
    static_assert(Array_Count(VKStoreOps) == GDI_STORE_OP_COUNT);
    Assert(StoreOp < GDI_STORE_OP_COUNT);
    return VKStoreOps[StoreOp];
}

internal VkFormat VK_Get_Format(gdi_format Format) {
    local_persist VkFormat VKFormats[] = {
        VK_FORMAT_UNDEFINED,
        VK_FORMAT_R8_UNORM,
        VK_FORMAT_R8_SRGB,
        VK_FORMAT_R8G8_UNORM,
        VK_FORMAT_R8G8_SRGB,
        VK_FORMAT_R8G8B8_UNORM,
        VK_FORMAT_R8G8B8_SRGB,
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
        VK_FORMAT_R32G32B32A32_SFLOAT,
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

internal VkIndexType VK_Get_Index_Type(gdi_format Format) {
    switch(Format) {
        case GDI_FORMAT_R16_UINT:
        return VK_INDEX_TYPE_UINT16;

        case GDI_FORMAT_R32_UINT:
        return VK_INDEX_TYPE_UINT32;

        Invalid_Default_Case();
    }
    return (VkIndexType)-1;
}

internal VkDescriptorType VK_Get_Descriptor_Type(gdi_bind_group_type Type) {
    const local_persist VkDescriptorType DescriptorTypes[] = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        VK_DESCRIPTOR_TYPE_SAMPLER
    };
    static_assert(Array_Count(DescriptorTypes) == GDI_BIND_GROUP_TYPE_COUNT);
    Assert(Type < GDI_BIND_GROUP_TYPE_COUNT);
    return DescriptorTypes[Type];
}

static VkFilter VK_Get_Filter(gdi_filter Filter) {
    local_persist const VkFilter Filters[] = {
        VK_FILTER_NEAREST,
        VK_FILTER_LINEAR
    };
    static_assert(Array_Count(Filters) == GDI_FILTER_COUNT);
    Assert(Filter < GDI_FILTER_COUNT);
    return Filters[Filter];
}

static VkSamplerAddressMode VK_Get_Address_Mode(gdi_address_mode AddressMode) {
    local_persist const VkSamplerAddressMode AddressModes[] = {
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_REPEAT
    };
    static_assert(Array_Count(AddressModes) == GDI_ADDRESS_MODE_COUNT);
    Assert(AddressMode < GDI_ADDRESS_MODE_COUNT);
    return AddressModes[AddressMode];
}

static VkCompareOp VK_Get_Compare_Op(gdi_comparison_func ComparisonFunc) {
    local_persist const VkCompareOp CompareOps[] = {
        VK_COMPARE_OP_LESS
    };
    static_assert(Array_Count(CompareOps) == GDI_COMPARISON_FUNC_COUNT);
    Assert(ComparisonFunc < GDI_COMPARISON_FUNC_COUNT);
    return CompareOps[ComparisonFunc];
}

static bool VK_Is_Pipeline_Depth(vk_pipeline_stage Stage) {
    local_persist const bool IsPipelineDepths[] = {
        false,
        false,
        false,
        true
    };
    static_assert(Array_Count(IsPipelineDepths) == (u32)vk_pipeline_stage::Count);
    Assert((u32)Stage < (u32)vk_pipeline_stage::Count);
    return IsPipelineDepths[(u32)Stage];
}

internal VkPrimitiveTopology VK_Get_Topology(gdi_topology Topology) {
    local_persist const VkPrimitiveTopology Topologies[] = {
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
    };
    static_assert(Array_Count(Topologies) == GDI_TOPOLOGY_COUNT);
    Assert(Topology < GDI_TOPOLOGY_COUNT);
    return Topologies[Topology];
}

internal VkBlendFactor VK_Get_Blend_Factor(gdi_blend Blend) {
    local_persist const VkBlendFactor BlendFactors[] = {
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_SRC_COLOR,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
        VK_BLEND_FACTOR_DST_COLOR,
        VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
        VK_BLEND_FACTOR_SRC_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        VK_BLEND_FACTOR_DST_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA
    };
    static_assert(Array_Count(BlendFactors) == GDI_BLEND_COUNT);
    Assert(Blend < GDI_BLEND_COUNT);
    return BlendFactors[Blend];
} 

internal VkBlendOp VK_Get_Blend_Op(gdi_blend_op BlendOp) {
    local_persist const VkBlendOp BlendOps[] = {
        VK_BLEND_OP_ADD,
        VK_BLEND_OP_SUBTRACT,
        VK_BLEND_OP_REVERSE_SUBTRACT,
        VK_BLEND_OP_MIN,
        VK_BLEND_OP_MAX
    };
    static_assert(Array_Count(BlendOps) == GDI_BLEND_OP_COUNT);
    Assert(BlendOp < GDI_BLEND_OP_COUNT);
    return BlendOps[BlendOp];
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

    if(Flags & GDI_TEXTURE_USAGE_FLAG_COPIED_BIT) {
        Result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    return Result;
}

internal VkBufferUsageFlags VK_Convert_To_Buffer_Usage_Flags(gdi_buffer_usage_flags Flags) {
    VkBufferUsageFlags Result = 0;

    if(Flags & GDI_BUFFER_USAGE_FLAG_VTX_BUFFER_BIT) {
        Result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

    if(Flags & GDI_BUFFER_USAGE_FLAG_IDX_BUFFER_BIT) {
        Result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }

    if(Flags & GDI_BUFFER_USAGE_FLAG_CONSTANT_BUFFER_BIT) {
        Result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }

    if(!(Flags & GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT)) {
        Result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    return Result;
}

internal VkShaderStageFlags VK_Convert_To_Shader_Stage_Flags(gdi_shader_stage_flags Flags) {
    VkShaderStageFlags Result = 0;

    if(Flags & GDI_SHADER_STAGE_VERTEX_BIT) {
        Result |= VK_SHADER_STAGE_VERTEX_BIT;
    }

    if(Flags & GDI_SHADER_STAGE_PIXEL_BIT) {
        Result |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    return Result;
}

internal VkVertexInputRate VK_Get_Input_Rate(gdi_vtx_input_rate InputRate) {
    VkVertexInputRate InputRates[] = {
        VK_VERTEX_INPUT_RATE_VERTEX,
        VK_VERTEX_INPUT_RATE_INSTANCE
    };
    static_assert(Array_Count(InputRates) == GDI_VTX_INPUT_RATE_COUNT);
    Assert(InputRate < GDI_VTX_INPUT_RATE_COUNT);
    return InputRates[InputRate];
} 

internal vk_pipeline_stage VK_Get_Pipeline_Stage(gdi_resource_state ResourceState) {
    Assert(ResourceState < GDI_RESOURCE_STATE_COUNT);
    return G_PipelineStages[ResourceState];
}

internal VkPipelineStageFlags VK_Get_Pipeline_Stage_Flags(gdi_resource_state ResourceState) {
    u32 PipelineStage = (u32)VK_Get_Pipeline_Stage(ResourceState);
    Assert(PipelineStage < (u32)vk_pipeline_stage::Count);
    return G_PipelineStageMasks[PipelineStage];
}

internal VkAccessFlags VK_Get_Access_Masks(gdi_resource_state ResourceState) {
    Assert(ResourceState < GDI_RESOURCE_STATE_COUNT);
    return G_VKAccessMasks[ResourceState];
}

internal VkImageLayout VK_Get_Image_Layout(gdi_resource_state ResourceState) {
    Assert(ResourceState < GDI_RESOURCE_STATE_COUNT);
    return G_VKImageLayouts[ResourceState];
}

internal bool VK_Create__Internal(gdi* GDI, const gdi_create_info& CreateInfo) {
    GDI->Arena = Arena_Create(GDI->MainAllocator);
    GDI->VKAllocator = {
        .pUserData = GDI->MainAllocator,
        .pfnAllocation = VK__Alloc,
        .pfnReallocation = VK__Realloc,
        .pfnFree = VK__Free
    };
    GDI->LogCallbacks = CreateInfo.LoggingCallbacks;
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

    const gdi_app_info* EngineInfo = &CreateInfo.EngineInfo;
    const gdi_app_info* AppInfo = &CreateInfo.AppInfo; 

    VkApplicationInfo ApplicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = AppInfo->Name.Str,
        .applicationVersion = VK_MAKE_API_VERSION(0, AppInfo->Version.Major, AppInfo->Version.Minor, AppInfo->Version.Patch),
        .pEngineName = EngineInfo->Name.Str,
        .engineVersion = VK_MAKE_API_VERSION(0, EngineInfo->Version.Major, EngineInfo->Version.Minor, EngineInfo->Version.Patch),
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
        DebugUtilsInfo.messageSeverity =
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |  
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT; 

        DebugUtilsInfo.messageType = 
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
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
        DebugReportInfo.flags = 
                                VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                                VK_DEBUG_REPORT_DEBUG_BIT_EXT |
                                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                                VK_DEBUG_REPORT_WARNING_BIT_EXT | 
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

    const VkPhysicalDeviceLimits* Limits = &Context->PhysicalDevice->Properties.limits; 
    gdi_context_info* Info = &Context->Info;
    Info->ConstantBufferAlignment = Limits->minUniformBufferOffsetAlignment;

    VK_Memory_Manager_Create(&Context->MemoryManager, Context);
    
    if(!VK_Descriptor_Pool_Create(Context, &Context->DescriptorPool)) {
        //todo: Logging
        return false;
    }

    Context->Frames = array<vk_frame_context>(Context->Arena, CreateInfo.FrameCount);
    Array_Resize(&Context->Frames, CreateInfo.FrameCount);
    
    VK_Thread_Context_Manager_Create(Context, &Context->ThreadContextManager);

    vk_resource_context* ResourceContext = &Context->ResourceContext;
    VK_Resource_Context_Create(Context, ResourceContext, CreateInfo);

    vk_thread_context* ThreadContext = VK_Get_Thread_Context(&Context->ThreadContextManager);
    for(u32 i = 0; i < CreateInfo.FrameCount; i++) {
        vk_frame_context* Frame = &Context->Frames[i];
        vk_cmd_pool* CmdPool = &ThreadContext->CmdPools[i];

        VkCommandPoolCreateInfo CmdPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = Context->PhysicalDevice->GraphicsQueueFamilyIndex
        };

        VkCommandBufferAllocateInfo CmdBufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = CmdPool->CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        if(vkAllocateCommandBuffers(Context->Device, &CmdBufferAllocateInfo, &Frame->CopyCmdBuffer) != VK_SUCCESS) {
            //todo: logging
            return false;
        }

        VkFenceCreateInfo FenceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
        };

        if(i != 0) {
            FenceCreateInfo.flags |= VK_FENCE_CREATE_SIGNALED_BIT;
        }
        vkCreateFence(Context->Device, &FenceCreateInfo, Context->VKAllocator, &Frame->Fence);
    }

    return true;
}

u64 VK_Get_Current_Frame_Index(gdi_context* Context) {
    return Context->TotalFramesRendered % Context->Frames.Count;
}

vk_frame_context* VK_Get_Current_Frame_Context(gdi_context* Context) {
    return &Context->Frames[Context->TotalFramesRendered % Context->Frames.Count];
}

vk_cmd_list* VK_Allocate_Cmd_List(gdi_context* Context, bool IsPrimary) {
    vk_thread_context* ThreadContext = VK_Get_Thread_Context(&Context->ThreadContextManager);
    vk_cmd_pool* CmdPool = VK_Get_Current_Cmd_Pool(&Context->ThreadContextManager, ThreadContext);

    vk_cmd_storage_list* CmdStorage = IsPrimary ? &CmdPool->PrimaryCmds : &CmdPool->SecondaryCmds;

    vk_cmd_list* CmdList = CmdStorage->Free;
    if(CmdList) SLL_Pop_Front(CmdStorage->Free);
    else {
        CmdList = Arena_Push_Struct(ThreadContext->Arena, vk_cmd_list);
        CmdList->Context = Context;

        VkCommandBufferAllocateInfo AllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = CmdPool->CommandPool,
            .level = IsPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
            .commandBufferCount = 1
        };

        if(vkAllocateCommandBuffers(Context->Device, &AllocateInfo, &CmdList->CmdBuffer) != VK_SUCCESS) {
            //todo: Diagnostic and error logging
            SLL_Push_Front(CmdStorage->Free, CmdList);
            return nullptr;
        }
    }
    CmdList->Pipeline = nullptr;
    DLL_Push_Back(CmdStorage->Head, CmdStorage->Tail, CmdList);

    return CmdList;
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
        Context->TotalFramesRendered = (u64)-1;
        vkDeviceWaitIdle(Context->Device);

        vk_resource_context* ResourceContext = &Context->ResourceContext;

        //First delete everything from the delete context
        VK_Thread_Context_Manager_Delete(&Context->ThreadContextManager);
        
        //Delete any remaining resources that are not on the delete queue
        VK_Resource_Context_Delete(&Context->ResourceContext);

        for(vk_frame_context& FrameContext : Context->Frames) {
            vkDestroyFence(Context->Device, FrameContext.Fence, Context->VKAllocator);
        }

        VK_Descriptor_Pool_Delete(&Context->DescriptorPool);
        VK_Memory_Manager_Delete(&Context->MemoryManager);

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

const gdi_context_info* GDI_Context_Get_Info(gdi_context* Context) {
    return &Context->Info;
}

gdi_handle<gdi_pipeline> GDI_Context_Create_Graphics_Pipeline(gdi_context* Context, const gdi_graphics_pipeline_create_info& CreateInfo) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_pipeline> PipelineHandle = VK_Resource_Alloc(ResourceContext->Pipelines);
    if(PipelineHandle.Is_Null()) {
        //todo: Diagnostics
        return {};
    }

    vk_pipeline* Pipeline = VK_Resource_Get(ResourceContext->Pipelines, PipelineHandle);
    if(!VK_Create_Pipeline(Context, Pipeline, CreateInfo)) {
        //todo: Diagnostics
        VK_Delete_Pipeline(Context, Pipeline);
        VK_Resource_Free(ResourceContext->Pipelines, PipelineHandle);
        return {};
    }

    return gdi_handle<gdi_pipeline>(PipelineHandle.ID);
}

void GDI_Context_Delete_Pipeline(gdi_context* Context, gdi_handle<gdi_pipeline> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_pipeline> PipelineHandle(Handle.ID);

    vk_pipeline* Pipeline = VK_Resource_Get(ResourceContext->Pipelines, PipelineHandle);
    if(Pipeline) {
        u64 FrameIndex;
        if(VK_Resource_Should_Delete(Context, Pipeline, &FrameIndex)) {
            VK_Delete_Pipeline(Context, Pipeline);
        } else {
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(&Context->ThreadContextManager);
            VK_Delete_Context_Add(DeleteContext, DeleteContext->PipelineList, Pipeline, FrameIndex);
        }
        Pipeline->Layout = VK_NULL_HANDLE;
        VK_Resource_Free(ResourceContext->Pipelines, PipelineHandle);
    }
}

gdi_handle<gdi_bind_group> GDI_Context_Create_Bind_Group(gdi_context* Context, const gdi_bind_group_create_info& CreateInfo) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_bind_group> BindGroupHandle = VK_Resource_Alloc(ResourceContext->BindGroups);
    if(BindGroupHandle.Is_Null()) {
        //todo: Diagnostics
        return {};
    }

    vk_bind_group* BindGroup = VK_Resource_Get(ResourceContext->BindGroups, BindGroupHandle);
    if(!VK_Create_Bind_Group(Context, BindGroup, CreateInfo)) {
        //todo: Diagnostics
        VK_Delete_Bind_Group(Context, BindGroup);
        VK_Resource_Free(ResourceContext->BindGroups, BindGroupHandle);
        return {};
    }

    return gdi_handle<gdi_bind_group>(BindGroupHandle.ID);
}

void GDI_Context_Delete_Bind_Group(gdi_context* Context, gdi_handle<gdi_bind_group> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_bind_group> BindGroupHandle(Handle.ID);
    
    vk_bind_group* BindGroup = VK_Resource_Get(ResourceContext->BindGroups, BindGroupHandle);
    if(BindGroup) {
        u64 FrameIndex;
        if(VK_Resource_Should_Delete(Context, BindGroup, &FrameIndex)) {
            VK_Delete_Bind_Group(Context, BindGroup);
        } else {
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(&Context->ThreadContextManager);
            VK_Delete_Context_Add(DeleteContext, DeleteContext->BindGroupList, BindGroup, FrameIndex);
        }
        VK_Resource_Free(ResourceContext->BindGroups, BindGroupHandle);
    }
}

gdi_handle<gdi_bind_group_layout> GDI_Context_Create_Bind_Group_Layout(gdi_context* Context, const gdi_bind_group_layout_create_info& CreateInfo) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_bind_group_layout> BindGroupLayoutHandle = VK_Resource_Alloc(ResourceContext->BindGroupLayouts);
    if(BindGroupLayoutHandle.Is_Null()) {
        //todo: Diagnostics
        return {};
    }

    vk_bind_group_layout* BindGroupLayout = VK_Resource_Get(ResourceContext->BindGroupLayouts, BindGroupLayoutHandle);
    if(!VK_Create_Bind_Group_Layout(Context, BindGroupLayout, CreateInfo)) {
        //todo: Diagnostics
        VK_Delete_Bind_Group_Layout(Context, BindGroupLayout);
        VK_Resource_Free(ResourceContext->BindGroupLayouts, BindGroupLayoutHandle);
        return {};
    }
    return gdi_handle<gdi_bind_group_layout>(BindGroupLayoutHandle.ID);
}

void GDI_Context_Delete_Bind_Group_Layout(gdi_context* Context, gdi_handle<gdi_bind_group_layout> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_bind_group_layout> BindGroupLayoutHandle(Handle.ID);
    
    vk_bind_group_layout* BindGroupLayout = VK_Resource_Get(ResourceContext->BindGroupLayouts, BindGroupLayoutHandle);
    if(BindGroupLayout) {
        u64 FrameIndex;
        if(VK_Resource_Should_Delete(Context, BindGroupLayout, &FrameIndex)) {
            VK_Delete_Bind_Group_Layout(Context, BindGroupLayout);
        } else {
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(&Context->ThreadContextManager);
            VK_Delete_Context_Add(DeleteContext, DeleteContext->BindGroupLayoutList, BindGroupLayout, FrameIndex); 
        }
        VK_Resource_Free(ResourceContext->BindGroupLayouts, BindGroupLayoutHandle);
    }
}

gdi_handle<gdi_framebuffer> GDI_Context_Create_Framebuffer(gdi_context* Context, const gdi_framebuffer_create_info& CreateInfo) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_framebuffer> FramebufferHandle = VK_Resource_Alloc(ResourceContext->Framebuffers);
    if(FramebufferHandle.Is_Null()) {
        //todo: Diagnostics
        return {};
    }

    vk_framebuffer* Framebuffer = VK_Resource_Get(ResourceContext->Framebuffers, FramebufferHandle);
    if(!VK_Create_Framebuffer(Context, Framebuffer, CreateInfo)) {
        //todo: Diagnostics
        VK_Delete_Framebuffer(Context, Framebuffer);
        VK_Resource_Free(ResourceContext->Framebuffers, FramebufferHandle);
        return {};
    }
    return gdi_handle<gdi_framebuffer>(FramebufferHandle.ID);
}

void GDI_Context_Delete_Framebuffer(gdi_context* Context, gdi_handle<gdi_framebuffer> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_framebuffer> FramebufferHandle(Handle.ID);
    vk_framebuffer* Framebuffer = VK_Resource_Get(ResourceContext->Framebuffers, FramebufferHandle);
    if(Framebuffer) {
        u64 FrameIndex;
        if(VK_Resource_Should_Delete(Context, Framebuffer, &FrameIndex)) {
            VK_Delete_Framebuffer(Context, Framebuffer);
        } else {
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(&Context->ThreadContextManager);
            VK_Delete_Context_Add(DeleteContext, DeleteContext->FramebufferList, Framebuffer, FrameIndex);
        }
        VK_Resource_Free(ResourceContext->Framebuffers, FramebufferHandle);
    }
}

uvec2 GDI_Context_Get_Framebuffer_Resolution(gdi_context* Context, gdi_handle<gdi_framebuffer> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_framebuffer> FramebufferHandle(Handle.ID);
    vk_framebuffer* Framebuffer = VK_Resource_Get(ResourceContext->Framebuffers, FramebufferHandle);
    if(!Framebuffer) return {};
    return uvec2(Framebuffer->Width, Framebuffer->Height);
}

fixed_array<gdi_handle<gdi_texture_view>> GDI_Context_Get_Framebuffer_Attachments(gdi_context* Context, gdi_handle<gdi_framebuffer> Handle, allocator* Allocator) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_framebuffer> FramebufferHandle(Handle.ID);
    vk_framebuffer* Framebuffer = VK_Resource_Get(ResourceContext->Framebuffers, FramebufferHandle);
    if(!Framebuffer) return {};
    return VK_Framebuffer_Get_Attachments(Context, Framebuffer, Allocator);
}

gdi_handle<gdi_render_pass> GDI_Context_Create_Render_Pass(gdi_context* Context, const gdi_render_pass_create_info& CreateInfo) { 
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_render_pass> RenderPassHandle = VK_Resource_Alloc(ResourceContext->RenderPasses);
    if(RenderPassHandle.Is_Null()) {
        //todo: Diagnostics
        return {};
    }

    vk_render_pass* RenderPass = VK_Resource_Get(ResourceContext->RenderPasses, RenderPassHandle);
    if(!VK_Create_Render_Pass(Context, RenderPass, CreateInfo)) {
        //todo: Diagnostic 
        VK_Delete_Render_Pass(Context, RenderPass);
        VK_Resource_Free(ResourceContext->RenderPasses, RenderPassHandle);
        return {};
    }
    return gdi_handle<gdi_render_pass>(RenderPassHandle.ID);
}

void GDI_Context_Delete_Render_Pass(gdi_context* Context, gdi_handle<gdi_render_pass> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_render_pass> RenderPassHandle(Handle.ID);
    vk_render_pass* RenderPass = VK_Resource_Get(ResourceContext->RenderPasses, RenderPassHandle);
    if(RenderPass) {
        u64 FrameIndex;
        if(VK_Resource_Should_Delete(Context, RenderPass, &FrameIndex)) {
            VK_Delete_Render_Pass(Context, RenderPass);
        } else {
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(&Context->ThreadContextManager);
            VK_Delete_Context_Add(DeleteContext, DeleteContext->RenderPassList, RenderPass, FrameIndex);
        }
        VK_Resource_Free(ResourceContext->RenderPasses, RenderPassHandle);
    }
}

gdi_handle<gdi_sampler> GDI_Context_Create_Sampler(gdi_context* Context, const gdi_sampler_create_info& CreateInfo) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_sampler> SamplerHandle = VK_Resource_Alloc(ResourceContext->Samplers);
    if(SamplerHandle.Is_Null()) {
        //todo: Diagnostics
        return {};
    }

    vk_sampler* Sampler = VK_Resource_Get(ResourceContext->Samplers, SamplerHandle);
    if(!VK_Create_Sampler(Context, Sampler, CreateInfo)) {
        //todo: Diagnostics
        VK_Delete_Sampler(Context, Sampler);
        VK_Resource_Free(ResourceContext->Samplers, SamplerHandle);
        return {};
    }
    return gdi_handle<gdi_sampler>(SamplerHandle.ID);
}

void GDI_Context_Delete_Sampler(gdi_context* Context, gdi_handle<gdi_sampler> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_sampler> SamplerHandle(Handle.ID);
    vk_sampler* Sampler = VK_Resource_Get(ResourceContext->Samplers, SamplerHandle);
    if(Sampler) {
        u64 FrameIndex;
        if(VK_Resource_Should_Delete(Context, Sampler, &FrameIndex)) {
            VK_Delete_Sampler(Context, Sampler);
        } else {
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(&Context->ThreadContextManager);
            VK_Delete_Context_Add(DeleteContext, DeleteContext->SamplerList, Sampler, FrameIndex);
        }
        VK_Resource_Free(ResourceContext->Samplers, SamplerHandle);
    }
}

gdi_handle<gdi_texture_view> GDI_Context_Create_Texture_View(gdi_context* Context, const gdi_texture_view_create_info& CreateInfo) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_texture_view> TextureViewHandle = VK_Resource_Alloc(ResourceContext->TextureViews);
    if(TextureViewHandle.Is_Null()) {
        //todo: Diagnostics
        return {};
    }

    vk_texture_view* TextureView = VK_Resource_Get(ResourceContext->TextureViews, TextureViewHandle);
    if(!VK_Create_Texture_View(Context, TextureView, CreateInfo)) {
        //todo: Diagnostic 
        VK_Delete_Texture_View(Context, TextureView);
        VK_Resource_Free(ResourceContext->TextureViews, TextureViewHandle);
        return {};
    }
    return gdi_handle<gdi_texture_view>(TextureViewHandle.ID);
}

void GDI_Context_Delete_Texture_View(gdi_context* Context, gdi_handle<gdi_texture_view> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_texture_view> TextureViewHandle(Handle.ID);
    vk_texture_view* TextureView = VK_Resource_Get(ResourceContext->TextureViews, TextureViewHandle);
    if(TextureView) {
        u64 FrameIndex;
        if(VK_Resource_Should_Delete(Context, TextureView, &FrameIndex)) {
            VK_Delete_Texture_View(Context, TextureView);
        } else {
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(&Context->ThreadContextManager);
            VK_Delete_Context_Add(DeleteContext, DeleteContext->TextureViewList, TextureView, FrameIndex);
        }
        VK_Resource_Free(ResourceContext->TextureViews, TextureViewHandle);
    }
}

gdi_handle<gdi_texture> GDI_Context_Get_Texture(gdi_context* Context, gdi_handle<gdi_texture_view> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_texture_view> TextureViewHandle(Handle.ID);
    vk_texture_view* TextureView = VK_Resource_Get(ResourceContext->TextureViews, TextureViewHandle);
    if(!TextureView) return {};

    vk_texture* Texture = TextureView->References[0].Get_Texture();
    if(!Texture) return {};

    vk_handle<vk_texture> TextureHandle(Safe_U32(ResourceContext->Textures.Get_Index(Texture)), 
                                        TextureView->References[0].Generation);
    return gdi_handle<gdi_texture>(TextureHandle.ID);
}

gdi_handle<gdi_texture> GDI_Context_Create_Texture(gdi_context* Context, const gdi_texture_create_info& CreateInfo) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_texture> TextureHandle = VK_Resource_Alloc(ResourceContext->Textures);
    if(TextureHandle.Is_Null()) {
        //todo: Diagnostics
        return {};
    }

    vk_texture* Texture = VK_Resource_Get(ResourceContext->Textures, TextureHandle);
    if(!VK_Create_Texture(Context, Texture, CreateInfo)) {
        //todo: Diagnostics
        VK_Delete_Texture(Context, Texture);
        VK_Resource_Free(ResourceContext->Textures, TextureHandle);
        return {};
    }
    
    if(CreateInfo.InitialData.Size && CreateInfo.InitialData.Ptr) {
        vk_thread_context* ThreadContext = VK_Get_Thread_Context(&Context->ThreadContextManager);
        vk_upload_buffer* UploadBuffer = VK_Get_Current_Upload_Buffer(&Context->ThreadContextManager, ThreadContext);
        
        vk_upload Upload;
        u8* Ptr = VK_Upload_Buffer_Push(UploadBuffer, CreateInfo.InitialData.Size, &Upload);
        Memory_Copy(Ptr, CreateInfo.InitialData.Ptr, CreateInfo.InitialData.Size);

        VK_Copy_Context_Add_Upload_To_Texture_Copy(&ThreadContext->CopyContext, TextureHandle, Upload, {
            .Width = CreateInfo.Width,
            .Height = CreateInfo.Height
        });
    }

    return gdi_handle<gdi_texture>(TextureHandle.ID);
}

void GDI_Context_Delete_Texture(gdi_context* Context, gdi_handle<gdi_texture> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_texture> TextureHandle(Handle.ID);
    vk_texture* Texture = VK_Resource_Get(ResourceContext->Textures, TextureHandle);
    if(Texture) {
        u64 FrameIndex;
        if(VK_Resource_Should_Delete(Context, Texture, &FrameIndex)) {
            VK_Delete_Texture(Context, Texture);
        } else {
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(&Context->ThreadContextManager);
            VK_Delete_Context_Add(DeleteContext, DeleteContext->TextureList, Texture, FrameIndex);
        }
        VK_Resource_Free(ResourceContext->Textures, TextureHandle);
    }
}

void GDI_Context_Upload_Texture(gdi_context* Context, gdi_handle<gdi_texture> Handle, span<gdi_texture_upload> SrcUploads) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_texture> TextureHandle(Handle.ID);
    vk_texture* Texture = VK_Resource_Get(ResourceContext->Textures, TextureHandle);
    if(Texture) {
        vk_thread_context* ThreadContext = VK_Get_Thread_Context(&Context->ThreadContextManager);
        vk_upload_buffer* UploadBuffer = VK_Get_Current_Upload_Buffer(&Context->ThreadContextManager, ThreadContext);

        scratch Scratch = Scratch_Get();
        fixed_array<uptr> Offsets(&Scratch, SrcUploads.Count);
        fixed_array<vk_region> Regions(&Scratch, SrcUploads.Count);

        uptr TotalSize = 0;
        for(uptr i = 0; i < SrcUploads.Count; i++) {
            Offsets[i] = TotalSize;
            TotalSize += SrcUploads[i].Texels.Size;
        }

        vk_upload Upload;
        u8* Ptr = VK_Upload_Buffer_Push(UploadBuffer, TotalSize, &Upload);

        for(uptr i = 0; i < SrcUploads.Count; i++) {
            Memory_Copy(Ptr, SrcUploads[i].Texels.Ptr, SrcUploads[i].Texels.Size);
            Ptr += SrcUploads[i].Texels.Size;

            Regions[i] = {
                .XOffset = SrcUploads[i].XOffset,
                .YOffset = SrcUploads[i].YOffset,
                .Width   = SrcUploads[i].Width,
                .Height  = SrcUploads[i].Height
            };
        }

        VK_Copy_Context_Add_Uploads_To_Texture_Copy(&ThreadContext->CopyContext, TextureHandle, Upload, Offsets, Regions);
    }
}

gdi_handle<gdi_buffer> GDI_Context_Create_Buffer(gdi_context* Context, const gdi_buffer_create_info& CreateInfo) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_buffer> BufferHandle = VK_Resource_Alloc(ResourceContext->Buffers);
    if(BufferHandle.Is_Null()) {
        //todo: Diagnostics
        return {};
    }

    vk_buffer* Buffer = VK_Resource_Get(ResourceContext->Buffers, BufferHandle);
    if(!VK_Create_Buffer(Context, Buffer, CreateInfo)) {
        VK_Delete_Buffer(Context, Buffer);
        VK_Resource_Free(ResourceContext->Buffers, BufferHandle);
        return {};
    }

    gdi_handle<gdi_buffer> Result(BufferHandle.ID);
    if(CreateInfo.InitialData.Ptr && CreateInfo.InitialData.Size) {
        u8* Ptr = GDI_Context_Buffer_Map(Context, Result);
        if(!Ptr) {
            VK_Delete_Buffer(Context, Buffer);
            VK_Resource_Free(ResourceContext->Buffers, BufferHandle);
            return {}; 
        }
        Memory_Copy(Ptr, CreateInfo.InitialData.Ptr, CreateInfo.InitialData.Size);
        GDI_Context_Buffer_Unmap(Context, Result);
    }

    return Result;
}

u8* GDI_Context_Buffer_Map(gdi_context* Context, gdi_handle<gdi_buffer> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_buffer> BufferHandle(Handle.ID);
    vk_buffer* Buffer = VK_Resource_Get(ResourceContext->Buffers, BufferHandle);
    if(!Buffer) {
        Assert(false);
        return NULL;
    }

    if(!(Buffer->UsageFlags & GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT)) {
        vk_thread_context* ThreadContext = VK_Get_Thread_Context(&Context->ThreadContextManager);
        vk_upload_buffer* UploadBuffer = VK_Get_Current_Upload_Buffer(&Context->ThreadContextManager, ThreadContext);
        Buffer->Ptr = VK_Upload_Buffer_Push(UploadBuffer, Buffer->Size, &Buffer->Upload);
        return Buffer->Ptr;
    } else {
        u64 FrameIndex = VK_Get_Current_Frame_Index(Context);
        return Buffer->Ptr + (FrameIndex*Buffer->Size);    
    }
}

void GDI_Context_Buffer_Unmap(gdi_context* Context, gdi_handle<gdi_buffer> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_buffer> BufferHandle(Handle.ID);
    vk_buffer* Buffer = VK_Resource_Get(ResourceContext->Buffers, BufferHandle);
    if(!(Buffer->UsageFlags & GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT)) {
        vk_thread_context* ThreadContext = VK_Get_Thread_Context(&Context->ThreadContextManager);
        VK_Copy_Context_Add_Upload_To_Buffer_Copy(&ThreadContext->CopyContext, {
            .Upload = Buffer->Upload,
            .Buffer = BufferHandle,
            .Offset = 0
        });        
    } else {
        //Noop 
    }
}

void GDI_Context_Delete_Buffer(gdi_context* Context, gdi_handle<gdi_buffer> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_buffer> BufferHandle(Handle.ID);
    vk_buffer* Buffer = VK_Resource_Get(ResourceContext->Buffers, BufferHandle);
    if(Buffer) {
        u64 FrameIndex;
        if(VK_Resource_Should_Delete(Context, Buffer, &FrameIndex)) {
            VK_Delete_Buffer(Context, Buffer);
        } else {
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(&Context->ThreadContextManager);
            VK_Delete_Context_Add(DeleteContext, DeleteContext->BufferList, Buffer, FrameIndex);
        }
        VK_Resource_Free(ResourceContext->Buffers, BufferHandle);
    }
}

gdi_handle<gdi_swapchain> GDI_Context_Create_Swapchain(gdi_context* Context, const gdi_swapchain_create_info& CreateInfo) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_swapchain> SwapchainHandle = VK_Resource_Alloc(ResourceContext->Swapchains);
    if(SwapchainHandle.Is_Null()) {
        //todo: diagnostics
        return {};
    }

    vk_swapchain* Swapchain = VK_Resource_Get(ResourceContext->Swapchains, SwapchainHandle);
    if(!VK_Create_Swapchain(Context, Swapchain, CreateInfo)) {
        //todo: Diagnostics
        VK_Delete_Swapchain(Context, Swapchain);
        VK_Resource_Free(ResourceContext->Swapchains, SwapchainHandle);
        return {};
    }

    if(!VK_Create_Swapchain_Textures(Context, Swapchain)) {
        //todo: Diagnostics
        VK_Delete_Swapchain_Full(Context, Swapchain);
        VK_Resource_Free(ResourceContext->Swapchains, SwapchainHandle);
        return {};
    }

    return gdi_handle<gdi_swapchain>(SwapchainHandle.ID);
}

void GDI_Context_Delete_Swapchain(gdi_context* Context, gdi_handle<gdi_swapchain> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_swapchain> SwapchainHandle(Handle.ID);
    vk_swapchain* Swapchain = VK_Resource_Get(ResourceContext->Swapchains, SwapchainHandle);
    if(Swapchain) {
        VK_Delete_Swapchain_Textures(Context, Swapchain);    
        
        u64 FrameIndex;
        if(VK_Resource_Should_Delete(Context, Swapchain, &FrameIndex)) {
            VK_Delete_Swapchain(Context, Swapchain);
        } else {
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(&Context->ThreadContextManager);
            VK_Delete_Context_Add(DeleteContext, DeleteContext->SwapchainList, Swapchain, FrameIndex);
        }
        VK_Resource_Free(ResourceContext->Swapchains, SwapchainHandle);
    }
}

bool GDI_Context_Resize_Swapchain(gdi_context* Context, gdi_handle<gdi_swapchain> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_swapchain> SwapchainHandle(Handle.ID);
    vk_swapchain* Swapchain = VK_Resource_Get(ResourceContext->Swapchains, SwapchainHandle);
    if(Swapchain) {
        //Copy the old swapchain so we can write the new one into the writer lock
        gdi_swapchain_create_info CreateInfo = {
            .TargetFormat = Swapchain->Format,
            .UsageFlags = Swapchain->UsageFlags
        };

        vk_swapchain SwapchainCopy = *Swapchain;

        VK_Delete_Swapchain_Textures(Context, Swapchain);    
        if(!VK_Create_Swapchain(Context, Swapchain, CreateInfo)) {
            //todo: some error logging
            return false;
        }

        if(!VK_Create_Swapchain_Textures(Context, Swapchain)) {
            //todo: some error logging
            return false;
        }

        //Prevent the surface from being deleted by setting it to null when we resize
        SwapchainCopy.Surface = VK_NULL_HANDLE;

        u64 FrameIndex;
        if(VK_Resource_Should_Delete(Context, Swapchain, &FrameIndex)) {
            VK_Delete_Swapchain(Context, &SwapchainCopy);
        } else {
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(&Context->ThreadContextManager);
            VK_Delete_Context_Add(DeleteContext, DeleteContext->SwapchainList, &SwapchainCopy, FrameIndex);
        }
    }
    return true;
}

span<gdi_handle<gdi_texture>> GDI_Context_Get_Swapchain_Textures(gdi_context* Context, gdi_handle<gdi_swapchain> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;

    vk_handle<vk_swapchain> SwapchainHandle(Handle.ID);
    vk_swapchain* Swapchain = VK_Resource_Get(ResourceContext->Swapchains, SwapchainHandle);
    if(Swapchain) {
        return span<gdi_handle<gdi_texture>>(Swapchain->Textures);
    }
    return span<gdi_handle<gdi_texture>>();
}

s32 GDI_Context_Get_Swapchain_Texture_Index(gdi_context* Context, gdi_handle<gdi_swapchain> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_swapchain> SwapchainHandle(Handle.ID);
    vk_swapchain* Swapchain = VK_Resource_Get(ResourceContext->Swapchains, SwapchainHandle);
    if(!Swapchain) {
        return -1;
    }

    u64 FrameIndex = VK_Get_Current_Frame_Index(Context);

    if(Swapchain->TextureIndex == -1) {
        u32 ImageIndex;
        VkResult SwapchainStatus = vkAcquireNextImageKHR(Context->Device, Swapchain->Handle, UINT64_MAX, Swapchain->AcquireLocks[FrameIndex], VK_NULL_HANDLE, &ImageIndex);
        if(SwapchainStatus != VK_SUCCESS) {
            Swapchain->Status = SwapchainStatus == VK_SUBOPTIMAL_KHR ? GDI_SWAPCHAIN_STATUS_RESIZE : GDI_SWAPCHAIN_STATUS_ERROR;
            return -1;
        }
        Swapchain->Status = GDI_SWAPCHAIN_STATUS_OK;
        Swapchain->TextureIndex = (s32)ImageIndex;
    }

    return Swapchain->TextureIndex;
}

gdi_swapchain_status GDI_Context_Get_Swapchain_Status(gdi_context* Context, gdi_handle<gdi_swapchain> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    vk_handle<vk_swapchain> SwapchainHandle(Handle.ID);
    vk_swapchain* Swapchain = VK_Resource_Get(ResourceContext->Swapchains, SwapchainHandle);
    if(!Swapchain) {
        return GDI_SWAPCHAIN_STATUS_ERROR;
    } 
    return Swapchain->Status;
}

gdi_cmd_list* GDI_Context_Begin_Cmd_List(gdi_context* Context, gdi_cmd_list_type Type, gdi_handle<gdi_render_pass> RenderPassID, gdi_handle<gdi_framebuffer> FramebufferID) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;

    vk_handle<vk_render_pass> RenderPassHandle(RenderPassID.ID);
    vk_handle<vk_framebuffer> FramebufferHandle(FramebufferID.ID);

    vk_render_pass* RenderPass = VK_Resource_Get(ResourceContext->RenderPasses, RenderPassHandle);
    vk_framebuffer* Framebuffer = VK_Resource_Get(ResourceContext->Framebuffers, FramebufferHandle);

    bool IsPrimary = !RenderPass || !Framebuffer;
    vk_cmd_list* CmdList = VK_Allocate_Cmd_List(Context, IsPrimary);

    VkCommandBufferBeginInfo BeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    VkCommandBufferInheritanceInfo InheritanceInfo;

    if(!IsPrimary) {
        BeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT|VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        InheritanceInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
            .renderPass = RenderPass->Handle,
            .framebuffer = Framebuffer->Handle
        };
        BeginInfo.pInheritanceInfo = &InheritanceInfo;

        //todo: Should we record renderpass and framebuffers here. Not sure
        //if the driver thinks these resources are in use from just 
        //vkBeginCommandBuffer        
        // VK_Resource_Record_Frame(RenderPass);
        // VK_Resource_Record_Frame(Framebuffer);
    }

    vkBeginCommandBuffer(CmdList->CmdBuffer, &BeginInfo);

    if(!IsPrimary) {
        VkRect2D Scissor = {{}, {Framebuffer->Width, Framebuffer->Height}};
        VkViewport Viewport = {
            .width = (f32)Framebuffer->Width,
            .height = (f32)Framebuffer->Height,
            .maxDepth = 1.0f
        };
        vkCmdSetScissor(CmdList->CmdBuffer, 0, 1, &Scissor);
        vkCmdSetViewport(CmdList->CmdBuffer, 0, 1, &Viewport);
    }

    return (gdi_cmd_list*)CmdList;
}

//todo: If we fail to submit the command buffer and we want to retry next frame
//the command list don't get cleaned up and reused and will leak. Currently
//VK_Thread_Context_Manager_New_Frame will recycle old command buffers, but we need
//a way to recycle them in this case
bool GDI_Context_Execute(gdi_context* Context, span<gdi_swapchain_present_info> PresentInfos) {
    scratch Scratch = Scratch_Get();
    vk_frame_context* FrameContext = VK_Get_Current_Frame_Context(Context);
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    array<VkCommandBuffer> CmdBuffers(&Scratch);
    
    VkCommandBufferBeginInfo BeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(FrameContext->CopyCmdBuffer, &BeginInfo);
    
    VK_Thread_Context_Manager_Copy_Data(&Context->ThreadContextManager);
    
    vkEndCommandBuffer(FrameContext->CopyCmdBuffer);
    Array_Push(&CmdBuffers, FrameContext->CopyCmdBuffer);

    u64 FrameIndex = VK_Get_Current_Frame_Index(Context);

    vk_thread_context* ThreadContext = (vk_thread_context*)AK_Atomic_Load_Ptr_Relaxed(&Context->ThreadContextManager.List);
    while(ThreadContext) {
        vk_cmd_pool* CmdPool = VK_Get_Current_Cmd_Pool(&Context->ThreadContextManager, ThreadContext);
        
        for(vk_cmd_list* CmdList = CmdPool->PrimaryCmds.Head; CmdList; CmdList = CmdList->Next) {
            vkEndCommandBuffer(CmdList->CmdBuffer);
            Array_Push(&CmdBuffers, CmdList->CmdBuffer);
        }

        ThreadContext = ThreadContext->Next;
    }

    fixed_array<VkPipelineStageFlags> SubmitWaitStages(&Scratch, PresentInfos.Count);
    fixed_array<VkSemaphore>          SubmitSemaphores(&Scratch, PresentInfos.Count);
    fixed_array<VkSemaphore>          PresentSemaphores(&Scratch, PresentInfos.Count);
    fixed_array<VkSwapchainKHR>       Swapchains(&Scratch, PresentInfos.Count);
    fixed_array<uint32_t>             SwapchainImageIndices(&Scratch, PresentInfos.Count);
    fixed_array<vk_swapchain*>        SwapchainPtrs(&Scratch, PresentInfos.Count);
    for(uptr i = 0; i < PresentInfos.Count; i++) {
        vk_handle<vk_swapchain> SwapchainHandle(PresentInfos[i].Swapchain.ID);
        vk_swapchain* Swapchain = VK_Resource_Get(ResourceContext->Swapchains, SwapchainHandle);
        if(!Swapchain || Swapchain->TextureIndex < 0) {
            //todo: Error logging
            //todo: See todo above the function
            Swapchain->Status = GDI_SWAPCHAIN_STATUS_ERROR;
            return false;
        }

        SubmitWaitStages[i]      = VK_Get_Pipeline_Stage_Flags(PresentInfos[i].InitialState);
        SubmitSemaphores[i]      = Swapchain->AcquireLocks[FrameIndex];
        PresentSemaphores[i]     = Swapchain->ExecuteLocks[FrameIndex];
        SwapchainImageIndices[i] = (u32)Swapchain->TextureIndex;
        Swapchains[i]            = Swapchain->Handle;
        SwapchainPtrs[i]         = Swapchain;
        VK_Resource_Record_Frame(Swapchain);
    }

    if(CmdBuffers.Count > 0) {
        VkSubmitInfo SubmitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = Safe_U32(SubmitSemaphores.Count),
            .pWaitSemaphores = SubmitSemaphores.Ptr,
            .pWaitDstStageMask = SubmitWaitStages.Ptr,
            .commandBufferCount = Safe_U32(CmdBuffers.Count),
            .pCommandBuffers = CmdBuffers.Ptr,
            .signalSemaphoreCount = Safe_U32(PresentSemaphores.Count),
            .pSignalSemaphores = PresentSemaphores.Ptr
        };

        if(vkQueueSubmit(Context->GraphicsQueue, 1, &SubmitInfo, FrameContext->Fence) != VK_SUCCESS) {
            Assert(false);
            //todo: See todo above the function
            return false;
        }
    }

    //If we fail to submit a swapchain for presentation. This is an error
    //however, the command buffers have been submitted, and therefore they must
    //be processed as a normal frame, we just won't be able to present the image
    //while the swapchain is resizing or it has been destroyed! So we need to actually
    //finish the rest of the function regardless and switch to the next frame
    bool Result = true;
    if(Swapchains.Count > 0) {
        fixed_array<VkResult> Results(&Scratch, Swapchains.Count);
        VkPresentInfoKHR PresentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = Safe_U32(PresentSemaphores.Count),
            .pWaitSemaphores = PresentSemaphores.Ptr,
            .swapchainCount = Safe_U32(Swapchains.Count),
            .pSwapchains = Swapchains.Ptr,
            .pImageIndices = SwapchainImageIndices.Ptr,
            .pResults = Results.Ptr
        };
        
        VkResult PresentStatus = vkQueuePresentKHR(Context->PresentQueue, &PresentInfo);
        if(PresentStatus == VK_ERROR_OUT_OF_DATE_KHR) {
            for(u32 i = 0; i < Swapchains.Count; i++) {
                SwapchainPtrs[i]->Status = Results[i] == VK_ERROR_OUT_OF_DATE_KHR ? GDI_SWAPCHAIN_STATUS_RESIZE : GDI_SWAPCHAIN_STATUS_OK;
            }
            //While this is an error case. Command buffers
            Result = false;
        } else if(PresentStatus != VK_SUCCESS) {
            Assert(false);
            for(u32 i = 0; i < Swapchains.Count; i++) {
                SwapchainPtrs[i]->Status = GDI_SWAPCHAIN_STATUS_ERROR;
            }
            //todo: See todo above the function
            Result = false;
        }

        if(PresentStatus == VK_SUCCESS) {
            for(u32 i = 0; i < Swapchains.Count; i++) {
                SwapchainPtrs[i]->Status = GDI_SWAPCHAIN_STATUS_OK;
                SwapchainPtrs[i]->TextureIndex = -1;
            }
        }
    }

    VK_Resource_Update_Last_Frame_Indices(ResourceContext);

    Context->TotalFramesRendered++;
    FrameContext = VK_Get_Current_Frame_Context(Context);
    VkResult FenceStatus = vkGetFenceStatus(Context->Device, FrameContext->Fence); 
    if(FenceStatus == VK_NOT_READY) {
        if(vkWaitForFences(Context->Device, 1, &FrameContext->Fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
            Assert(false);
            return false;
        }
    } else if (FenceStatus != VK_SUCCESS) {
        Assert(false);
        return false;
    }
    
    if(vkResetFences(Context->Device, 1, &FrameContext->Fence) != VK_SUCCESS) {
        Assert(false);
        return false;
    }

    VK_Thread_Context_Manager_New_Frame(&Context->ThreadContextManager);

    return true;
}

void GDI_Cmd_List_Barrier(gdi_cmd_list* _CmdList, span<gdi_barrier> Barriers) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    gdi_context* Context = CmdList->Context;
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    
    scratch Scratch = Scratch_Get();
    vk_pipeline_barrier PipelineBarriers[(u32)vk_pipeline_stage::Count][(u32)vk_pipeline_stage::Count] = {};

    for(const gdi_barrier& Barrier : Barriers) {
        u32 OldStage = (u32)VK_Get_Pipeline_Stage(Barrier.OldState);
        u32 NewStage = (u32)VK_Get_Pipeline_Stage(Barrier.NewState);

        PipelineBarriers[OldStage][NewStage].InUse = true;

        if(Barrier.Resource.Type == gdi_resource_type::Texture) {
            if(!PipelineBarriers[OldStage][NewStage].ImageMemoryBarriers.Allocator) {
                PipelineBarriers[OldStage][NewStage].ImageMemoryBarriers = array<VkImageMemoryBarrier>(&Scratch);
            }

            vk_handle<vk_texture> TextureHandle(Barrier.Resource.TextureHandle.ID);   
            vk_texture* Texture = VK_Resource_Get(ResourceContext->Textures, TextureHandle);
            if(!Texture) {
                Assert(false);
                return;
            }

            VkImageAspectFlags ImageAspect = VK_IMAGE_ASPECT_COLOR_BIT;
            if(VK_Is_Pipeline_Depth((vk_pipeline_stage)NewStage)) {
                ImageAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
            }
            
            Array_Push(&PipelineBarriers[OldStage][NewStage].ImageMemoryBarriers, {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = VK_Get_Access_Masks(Barrier.OldState),
                .dstAccessMask = VK_Get_Access_Masks(Barrier.NewState),
                .oldLayout     = VK_Get_Image_Layout(Barrier.OldState),
                .newLayout     = VK_Get_Image_Layout(Barrier.NewState),
                .srcQueueFamilyIndex = Context->PhysicalDevice->GraphicsQueueFamilyIndex,
                .dstQueueFamilyIndex = Context->PhysicalDevice->GraphicsQueueFamilyIndex,
                .image = Texture->Handle,
                .subresourceRange = {ImageAspect, 0, 1, 0, 1}
            });

            VK_Resource_Record_Frame(Texture);
        } else if(Barrier.Resource.Type == gdi_resource_type::Buffer) {
            Not_Implemented();
        }
    }

    for(u32 i = 0; i < (u32)vk_pipeline_stage::Count; i++) {
        for(u32 j = 0; j < (u32)vk_pipeline_stage::Count; j++) {
            if(PipelineBarriers[i][j].InUse) {
                span<VkImageMemoryBarrier> ImageMemoryBarriers = PipelineBarriers[i][j].ImageMemoryBarriers;
                vkCmdPipelineBarrier(CmdList->CmdBuffer, G_PipelineStageMasks[i], G_PipelineStageMasks[j], VK_DEPENDENCY_BY_REGION_BIT, 
                                    0, nullptr, 0, nullptr, Safe_U32(ImageMemoryBarriers.Count), ImageMemoryBarriers.Ptr);
            }
        }
    }
}

void GDI_Cmd_List_Begin_Render_Pass(gdi_cmd_list* _CmdList, const gdi_render_pass_begin_info& BeginInfo) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    gdi_context* Context = CmdList->Context;
    vk_resource_context* ResourceContext = &Context->ResourceContext;

    vk_handle<vk_framebuffer> FramebufferHandle(BeginInfo.Framebuffer.ID);
    vk_framebuffer* Framebuffer = VK_Resource_Get(ResourceContext->Framebuffers, FramebufferHandle);
    if(!Framebuffer) {
        Assert(false);
        return;
    }

    vk_handle<vk_render_pass> RenderPassHandle(BeginInfo.RenderPass.ID);
    vk_render_pass* RenderPass = VK_Resource_Get(ResourceContext->RenderPasses, RenderPassHandle);
    if(!RenderPass) {
        Assert(false);
        return;
    }

    scratch Scratch = Scratch_Get();
    fixed_array<VkClearValue> ClearValues(&Scratch, BeginInfo.ClearValues.Count);

    for(u32 i = 0; i < BeginInfo.ClearValues.Count; i++) {
        if(BeginInfo.ClearValues[i].Type == gdi_clear_type::Color) {
            Memory_Copy(&ClearValues[i].color, &BeginInfo.ClearValues[i].ClearColor, sizeof(VkClearColorValue));
        } else if(BeginInfo.ClearValues[i].Type == gdi_clear_type::Depth) {
            ClearValues[i].depthStencil.depth = BeginInfo.ClearValues[i].ClearDepth.Depth;
        }
    }

    VkRenderPassBeginInfo RenderPassBeginInfo = {
        .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass  = RenderPass->Handle,
        .framebuffer = Framebuffer->Handle,
        .renderArea  = {{}, {Framebuffer->Width, Framebuffer->Height}},
        .clearValueCount = Safe_U32(ClearValues.Count),
        .pClearValues = ClearValues.Ptr
    };

    vkCmdBeginRenderPass(CmdList->CmdBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

    VK_Resource_Record_Frame(RenderPass);
    VK_Resource_Record_Frame(Framebuffer);
}

void GDI_Cmd_List_End_Render_Pass(gdi_cmd_list* _CmdList) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    vkCmdEndRenderPass(CmdList->CmdBuffer);
}

void GDI_Cmd_List_Set_Vtx_Buffers(gdi_cmd_list* _CmdList, span<gdi_handle<gdi_buffer>> VtxBuffers) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    gdi_context* Context = CmdList->Context;
    vk_resource_context* ResourceContext = &Context->ResourceContext;

    scratch Scratch = Scratch_Get();
    VkDeviceSize* Offsets = Scratch_Push_Array(&Scratch, VtxBuffers.Count, VkDeviceSize);
    VkBuffer* Buffers = Scratch_Push_Array(&Scratch, VtxBuffers.Count, VkBuffer);

    for(uptr i = 0; i < VtxBuffers.Count; i++) {
        vk_handle<vk_buffer> BufferHandle(VtxBuffers[i].ID);
        vk_buffer* Buffer = VK_Resource_Get(ResourceContext->Buffers, BufferHandle);
        if(!Buffer) {
            Assert(false);
            return;
        }


        Offsets[i] = 0;
        Buffers[i] = Buffer->Handle;

        if(Buffer->UsageFlags & GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT) {
            Offsets[i] += Buffer->Size*VK_Get_Current_Frame_Index(Context);
        }

        VK_Resource_Record_Frame(Buffer);
    }

    vkCmdBindVertexBuffers(CmdList->CmdBuffer, 0, Safe_U32(VtxBuffers.Count), Buffers, Offsets);
}

void GDI_Cmd_List_Set_Idx_Buffer(gdi_cmd_list* _CmdList, gdi_handle<gdi_buffer> IdxBuffer, gdi_format Format) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    gdi_context* Context = CmdList->Context;
    vk_resource_context* ResourceContext = &Context->ResourceContext;

    vk_handle<vk_buffer> BufferHandle(IdxBuffer.ID);
    vk_buffer* Buffer = VK_Resource_Get(ResourceContext->Buffers, BufferHandle);
    if(!Buffer) {
        Assert(false);
        return;
    }

    uptr Offset = 0;
    if(Buffer->UsageFlags & GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT) {
        Offset += Buffer->Size*VK_Get_Current_Frame_Index(Context);
    }

    vkCmdBindIndexBuffer(CmdList->CmdBuffer, Buffer->Handle, Offset, VK_Get_Index_Type(Format));
    VK_Resource_Record_Frame(Buffer);
}

void GDI_Cmd_List_Set_Pipeline(gdi_cmd_list* _CmdList, gdi_handle<gdi_pipeline> PipelineID) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    gdi_context* Context = CmdList->Context;
    vk_resource_context* ResourceContext = &Context->ResourceContext;

    vk_handle<vk_pipeline> PipelineHandle(PipelineID.ID);
    vk_pipeline* Pipeline = VK_Resource_Get(ResourceContext->Pipelines, PipelineHandle);
    if(!Pipeline) {
        Assert(false);
        return;
    }
    
    vkCmdBindPipeline(CmdList->CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->Handle);
    CmdList->Pipeline = Pipeline;
    VK_Resource_Record_Frame(Pipeline);
}

void GDI_Cmd_List_Set_Bind_Groups(gdi_cmd_list* _CmdList, u32 StartingIndex, span<gdi_handle<gdi_bind_group>> BindGroups) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    gdi_context* Context = CmdList->Context;
    vk_resource_context* ResourceContext = &Context->ResourceContext;

    scratch Scratch = Scratch_Get();

    fixed_array<VkDescriptorSet> DescriptorSets(&Scratch, BindGroups.Count);

    for(uptr i = 0; i < BindGroups.Count; i++) {
        gdi_handle<gdi_bind_group> Handle = BindGroups[i];
        vk_handle<vk_bind_group> BindGroupHandle(Handle.ID);
        vk_bind_group* BindGroup = VK_Resource_Get(ResourceContext->BindGroups, BindGroupHandle);
        if(!BindGroup) {
            Assert(false);
            return;
        }

        DescriptorSets[i] = BindGroup->Handle;

        VK_Resource_Record_Frame(BindGroup);
    }

    vk_pipeline* Pipeline = CmdList->Pipeline;
    Assert(Pipeline);
    vkCmdBindDescriptorSets(CmdList->CmdBuffer, Pipeline->BindPoint, Pipeline->Layout, StartingIndex, 
                            Safe_U32(DescriptorSets.Count), DescriptorSets.Ptr, 0, VK_NULL_HANDLE);
}

void GDI_Cmd_List_Set_Dynamic_Bind_Groups(gdi_cmd_list* _CmdList, u32 StartingIndex, span<gdi_handle<gdi_bind_group>> BindGroups, span<uptr> Offsets) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    gdi_context* Context = CmdList->Context;
    vk_resource_context* ResourceContext = &Context->ResourceContext;

    scratch Scratch = Scratch_Get();

    fixed_array<VkDescriptorSet> DescriptorSets(&Scratch, BindGroups.Count);
    fixed_array<uint32_t> DescriptorOffsets(&Scratch, Offsets.Count);

    uptr OffsetIndex = 0;
    for(uptr i = 0; i < BindGroups.Count; i++) {
        vk_handle<vk_bind_group> BindGroupHandle(BindGroups[i].ID);
        vk_bind_group* BindGroup = VK_Resource_Get(ResourceContext->BindGroups, BindGroupHandle);
        if(!BindGroup) {
            Assert(false);
            return;
        }

        //For dynamic buffers and dynamic bind groups, we need to make
        //sure we offset the proper frame index
        DescriptorSets[i] = BindGroup->Handle;
        for(uptr j = 0; j < BindGroup->DynamicOffsets.Count; j++) {
            uptr FrameOffset = BindGroup->DynamicOffsets[j]*VK_Get_Current_Frame_Index(Context);
            DescriptorOffsets[OffsetIndex] = Safe_U32(Offsets[OffsetIndex]+FrameOffset);
            OffsetIndex++;
        }

        VK_Resource_Record_Frame(BindGroup);
    }

    vk_pipeline* Pipeline = CmdList->Pipeline;
    Assert(Pipeline);
    vkCmdBindDescriptorSets(CmdList->CmdBuffer, Pipeline->BindPoint, Pipeline->Layout, StartingIndex, 
                            Safe_U32(DescriptorSets.Count), DescriptorSets.Ptr, Safe_U32(DescriptorOffsets.Count), DescriptorOffsets.Ptr);
}

void GDI_Cmd_List_Draw_Instance(gdi_cmd_list* _CmdList, u32 VtxCount, u32 InstanceCount, u32 VtxOffset, u32 InstanceOffset) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    vkCmdDraw(CmdList->CmdBuffer, VtxCount, InstanceCount, VtxOffset, InstanceOffset);
}

void GDI_Cmd_List_Draw_Indexed_Instance(gdi_cmd_list* _CmdList, u32 IdxCount, u32 IdxOffset, u32 VtxOffset, u32 InstanceCount, u32 InstanceOffset) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    vkCmdDrawIndexed(CmdList->CmdBuffer, IdxCount, InstanceCount, IdxOffset, (s32)VtxOffset, InstanceOffset);
}

void GDI_Cmd_List_Execute_Cmds(gdi_cmd_list* _MainCmdList, span<gdi_cmd_list*> CmdLists) {
    vk_cmd_list* MainCmdList = (vk_cmd_list*)_MainCmdList;
    scratch Scratch = Scratch_Get();

    fixed_array<VkCommandBuffer> CmdBuffers(&Scratch, CmdLists.Count);
    for(uptr i = 0; i < CmdLists.Count; i++) {
        vk_cmd_list* CmdList = (vk_cmd_list*)CmdLists[i];
        CmdBuffers[i] = CmdList->CmdBuffer;
        vkEndCommandBuffer(CmdList->CmdBuffer);
    }

    vkCmdExecuteCommands(MainCmdList->CmdBuffer, Safe_U32(CmdBuffers.Count), CmdBuffers.Ptr);
}

#include "vk_thread_context.cpp"
#include "vk_resource.cpp"
#include "vk_memory.cpp"
#include "vk_functions.cpp"

#include <gdi/gdi_shared.cpp>