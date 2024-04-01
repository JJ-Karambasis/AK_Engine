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
        VK_ATTACHMENT_STORE_OP_STORE
    };
    static_assert(Array_Count(VKStoreOps) == GDI_STORE_OP_COUNT);
    Assert(StoreOp < GDI_STORE_OP_COUNT);
    return VKStoreOps[StoreOp];
}

internal VkFormat VK_Get_Format(gdi_format Format) {
    local_persist VkFormat VKFormats[] = {
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

internal VkBufferUsageFlags VK_Convert_To_Buffer_Usage_Flags(gdi_buffer_usage_flags Flags) {
    VkBufferUsageFlags Result = 0;

    if(Flags & GDI_BUFFER_USAGE_FLAG_VTX_BUFFER_BIT) {
        Result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

    if(Flags & GDI_BUFFER_USAGE_FLAG_IDX_BUFFER_BIT) {
        Result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }

    return Result;
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

inline internal void VK_Resource_Update_Frame_Indices(gdi_context* Context, u64* LastFrameIndices, ak_atomic_u32* InUse, uint32_t Count) {
    for(u32 i = 0; i < Count; i++) {
        if(AK_Atomic_Load_U32_Relaxed(&InUse[i])) {
            LastFrameIndices[i] = Context->TotalFramesRendered;
            AK_Atomic_Store_U32_Relaxed(&InUse[i], false);
        }
    }
}

template <typename type>
inline internal void VK_Delete_List_Init(vk_delete_list<type>* List, allocator* Allocator) {
    Array_Init(&List->List, Allocator, 128);
}

template <typename type>
inline internal void VK_Delete_List_Add(vk_delete_list<type>* List, type* Resource, u64 LastUsedFrameIndex) {
    Array_Push(&List->List, {
        .LastUsedFrameIndex = LastUsedFrameIndex,
        .Resource = *Resource
    });
}

template <typename type>
inline internal void VK_Delete_List_Clear(vk_delete_list<type>* List) {
    Array_Clear(&List->List);
}

template <typename type>
inline internal void VK_Delete_List_Free(vk_delete_list<type>* List) {
    Array_Free(&List->List);
}

vk_upload_buffer_block* VK_Upload_Buffer_Get_Current_Block(vk_upload_buffer* UploadBuffer, uptr Size, uptr Alignment) {
    vk_upload_buffer_block* Result = UploadBuffer->Current;
    while(Result && (Result->Data.Size < (Align_Pow2(Result->Used, Alignment)+Size))) {
        Result = Result->Next;
    }
    return Result;
}

vk_upload_buffer_block* VK_Upload_Buffer_Create_Block(vk_upload_buffer* UploadBuffer, size_t AllocationSize) {
    VkBufferCreateInfo BufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size  = AllocationSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VkBuffer Buffer;
    if(vkCreateBuffer(UploadBuffer->Device, &BufferCreateInfo, UploadBuffer->VKAllocator, &Buffer) != VK_SUCCESS) {
        //todo: Logging
        return NULL;
    }

    VkMemoryRequirements MemoryRequirements;
    vkGetBufferMemoryRequirements(UploadBuffer->Device, Buffer, &MemoryRequirements);

    vk_allocation Allocation;
    if(!VK_Memory_Allocate(UploadBuffer->MemoryManager, &MemoryRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &Allocation)) {
        //todo: Logging
        vkDestroyBuffer(UploadBuffer->Device, Buffer, UploadBuffer->VKAllocator);
        return NULL;
    }

    VkDeviceMemory Memory = VK_Get_Memory_Block(&Allocation.Allocate)->Memory;
    if(vkBindBufferMemory(UploadBuffer->Device, Buffer, Memory, Allocation.Allocate.Offset) != VK_SUCCESS) {
        //todo: Logging
        VK_Memory_Free(UploadBuffer->MemoryManager, &Allocation);
        vkDestroyBuffer(UploadBuffer->Device, Buffer, UploadBuffer->VKAllocator);
        return NULL;
    }

    buffer Data;
    if(!VK_Memory_Map(UploadBuffer->Device, &Allocation, MemoryRequirements.size, &Data)) {
        //todo: Logging
        VK_Memory_Free(UploadBuffer->MemoryManager, &Allocation);
        vkDestroyBuffer(UploadBuffer->Device, Buffer, UploadBuffer->VKAllocator);
        return NULL;
    }

    vk_upload_buffer_block* Block = Arena_Push_Struct(UploadBuffer->Arena, vk_upload_buffer_block);
    Block->Buffer     = Buffer;
    Block->Allocation = Allocation;
    Block->Data       = Data;
    Block->Used       = 0;
    return Block;
}

void VK_Upload_Buffer_Delete_Block(vk_upload_buffer* UploadBuffer, vk_upload_buffer_block* Block) {
    if(Block->Data.Ptr) {
        VK_Memory_Unmap(UploadBuffer->Device, &Block->Allocation);
        Block->Data = {};
    }

    VK_Memory_Free(UploadBuffer->MemoryManager, &Block->Allocation);

    if(Block->Buffer) {
        vkDestroyBuffer(UploadBuffer->Device, Block->Buffer, UploadBuffer->VKAllocator);
        Block->Buffer = VK_NULL_HANDLE;
    }
}

vk_upload_buffer* VK_Get_Current_Upload_Buffer(gdi_context* Context, vk_thread_context* ThreadContext) {
    u64 FrameIndex = Context->TotalFramesRendered % Context->Frames.Count;
    return &ThreadContext->UploadBuffers[FrameIndex];
}

void VK_Upload_Buffer_Create(gdi_context* Context, vk_upload_buffer* UploadBuffer) {
    UploadBuffer->Arena = Arena_Create(Context->GDI->MainAllocator);
    UploadBuffer->Device = Context->Device;
    UploadBuffer->VKAllocator = Context->VKAllocator;
    UploadBuffer->MemoryManager = &Context->MemoryManager;
}

void VK_Upload_Buffer_Delete(vk_upload_buffer* UploadBuffer) {
    if(UploadBuffer->Arena) {
        vk_upload_buffer_block* Block = UploadBuffer->First;
        while(Block) {
            vk_upload_buffer_block* BlockToDelete = Block;
            Block = BlockToDelete->Next;
            VK_Upload_Buffer_Delete_Block(UploadBuffer, BlockToDelete);
        }

        Arena_Delete(UploadBuffer->Arena);
        UploadBuffer->Arena = NULL;
    }

    UploadBuffer->Device = VK_NULL_HANDLE;
    UploadBuffer->VKAllocator = NULL;
    UploadBuffer->MemoryManager = NULL;
}

bool VK_Upload_Buffer_Push(vk_upload_buffer* UploadBuffer, const_buffer Data, vk_upload* Upload) {
    const uptr Alignment = DEFAULT_ALIGNMENT;

    vk_upload_buffer_block* Block = VK_Upload_Buffer_Get_Current_Block(UploadBuffer, Data.Size, Alignment);
    if(!Block) {
        uptr BlockSize = VK_UPLOAD_BUFFER_MINIMUM_BLOCK_SIZE;

        uptr Mask = Alignment-1;
        if(BlockSize < (Data.Size+Mask)) {
            BlockSize = Ceil_Pow2(Data.Size+Mask);
        }

        Block = VK_Upload_Buffer_Create_Block(UploadBuffer, BlockSize);
        if(!Block) {
            return false;
        }

        SLL_Push_Back(UploadBuffer->First, UploadBuffer->Last, Block);
    }

    UploadBuffer->Current = Block;
    vk_upload_buffer_block* CurrentBlock = UploadBuffer->Current;

    CurrentBlock->Used = Align_Pow2(CurrentBlock->Used, Alignment);
    Assert(CurrentBlock->Used+Data.Size <= CurrentBlock->Data.Size);

    Upload->Buffer = CurrentBlock->Buffer;
    Upload->Offset = CurrentBlock->Used;
    Upload->Size   = Data.Size;

    Memory_Copy(CurrentBlock->Data.Ptr+CurrentBlock->Used, Data.Ptr, Data.Size);
    CurrentBlock->Used += Data.Size;

    return true; 
}

void VK_Upload_Buffer_Clear(vk_upload_buffer* UploadBuffer) {
    for(vk_upload_buffer_block* Block = UploadBuffer->First; Block; Block = Block->Next) {
        Block->Used = 0;
    }
    UploadBuffer->Current = UploadBuffer->First;
}

internal vk_thread_context* VK_Get_Thread_Context(gdi_context* Context) {
    vk_thread_context* Result = (vk_thread_context*)AK_TLS_Get(&Context->ThreadContextTLS);
    if(!Result) {
        //One allocation for the entire delete context
        arena* ThreadContextArena = Context->ThreadContextArena;
        AK_Mutex_Lock(&Context->ThreadContextLock);
        Result = Arena_Push_Struct(ThreadContextArena, vk_thread_context);
        Result->UploadBuffers = fixed_array<vk_upload_buffer>(ThreadContextArena, Context->Frames.Count);
        AK_Mutex_Unlock(&Context->ThreadContextLock);

        SLL_Push_Front_Async(Context->ThreadContextList, Result);

        vk_delete_context* DeleteContext = &Result->DeleteContext;
        AK_RW_Lock_Create(&DeleteContext->RWLock);
        for(u32 i = 0; i < 2; i++) {
            VK_Delete_List_Init(&DeleteContext->RenderPassList[i], Context->GDI->MainAllocator);
            VK_Delete_List_Init(&DeleteContext->BufferList[i], Context->GDI->MainAllocator);
            VK_Delete_List_Init(&DeleteContext->TextureViewList[i], Context->GDI->MainAllocator);
            VK_Delete_List_Init(&DeleteContext->FramebufferList[i], Context->GDI->MainAllocator);
            VK_Delete_List_Init(&DeleteContext->SwapchainList[i], Context->GDI->MainAllocator);
        }

        vk_copy_context* CopyContext = &Result->CopyContext;
        AK_RW_Lock_Create(&CopyContext->RWLock);
        for(u32 i = 0; i < 2; i++) {
            Array_Init(&CopyContext->CopyUploadToBufferList[i], Context->GDI->MainAllocator);
        }

        for(vk_upload_buffer& UploadBuffer : Result->UploadBuffers) {
            VK_Upload_Buffer_Create(Context, &UploadBuffer);
        }

        AK_TLS_Set(&Context->ThreadContextTLS, Result);
    }
    return Result;
}

inline internal u32 VK_Copy_Context_Swap(vk_copy_context* CopyContext) {
    AK_RW_Lock_Writer(&CopyContext->RWLock);
    u32 Result = CopyContext->CurrentListIndex;
    CopyContext->CurrentListIndex = !CopyContext->CurrentListIndex;
    AK_RW_Unlock_Writer(&CopyContext->RWLock);
    return Result;
}

internal void VK_Copy_Context_Add_Upload_To_Buffer_Copy(vk_copy_context* CopyContext, const vk_copy_upload_to_buffer& CopyUploadToBuffer) {
    AK_RW_Lock_Reader(&CopyContext->RWLock);
    u32 ListIndex = CopyContext->CurrentListIndex;
    Array_Push(&CopyContext->CopyUploadToBufferList[ListIndex], CopyUploadToBuffer);
    AK_RW_Unlock_Reader(&CopyContext->RWLock);
}

internal vk_delete_context* VK_Get_Delete_Context(gdi_context* Context) {
    return &VK_Get_Thread_Context(Context)->DeleteContext;
}

inline internal u32 VK_Delete_Context_Swap(vk_delete_context* DeleteContext) {
    AK_RW_Lock_Writer(&DeleteContext->RWLock);
    u32 Result = DeleteContext->CurrentListIndex;
    DeleteContext->CurrentListIndex = !DeleteContext->CurrentListIndex;
    AK_RW_Unlock_Writer(&DeleteContext->RWLock);
    return Result;
}

inline internal void VK_Delete_Context_Add_Render_Pass(vk_delete_context* DeleteContext, vk_render_pass* RenderPass, u64 LastUsedFrameIndex) {
    AK_RW_Lock_Reader(&DeleteContext->RWLock);
    u32 ListIndex = DeleteContext->CurrentListIndex;
    VK_Delete_List_Add(&DeleteContext->RenderPassList[ListIndex], RenderPass, LastUsedFrameIndex);
    AK_RW_Unlock_Reader(&DeleteContext->RWLock);
}

inline internal void VK_Delete_Context_Add_Buffer(vk_delete_context* DeleteContext, vk_buffer* Buffer, u64 LastUsedFrameIndex) {
    AK_RW_Lock_Reader(&DeleteContext->RWLock);
    u32 ListIndex = DeleteContext->CurrentListIndex;
    VK_Delete_List_Add(&DeleteContext->BufferList[ListIndex], Buffer, LastUsedFrameIndex);
    AK_RW_Unlock_Reader(&DeleteContext->RWLock);
}

inline internal void VK_Delete_Context_Add_Texture_View(vk_delete_context* DeleteContext, vk_texture_view* TextureView, u64 LastUsedFrameIndex) {
    AK_RW_Lock_Reader(&DeleteContext->RWLock);
    u32 ListIndex = DeleteContext->CurrentListIndex;
    VK_Delete_List_Add(&DeleteContext->TextureViewList[ListIndex], TextureView, LastUsedFrameIndex);
    AK_RW_Unlock_Reader(&DeleteContext->RWLock);
}

inline internal void VK_Delete_Context_Add_Framebuffer(vk_delete_context* DeleteContext, vk_framebuffer* Framebuffer, u64 LastUsedFrameIndex) {
    AK_RW_Lock_Reader(&DeleteContext->RWLock);
    u32 ListIndex = DeleteContext->CurrentListIndex;
    VK_Delete_List_Add(&DeleteContext->FramebufferList[ListIndex], Framebuffer, LastUsedFrameIndex);
    AK_RW_Unlock_Reader(&DeleteContext->RWLock);
}

inline internal void VK_Delete_Context_Add_Swapchain(vk_delete_context* DeleteContext, vk_swapchain* Swapchain, u64 LastUsedFrameIndex) {
    AK_RW_Lock_Reader(&DeleteContext->RWLock);
    u32 ListIndex = DeleteContext->CurrentListIndex;
    VK_Delete_List_Add(&DeleteContext->SwapchainList[ListIndex], Swapchain, LastUsedFrameIndex);
    AK_RW_Unlock_Reader(&DeleteContext->RWLock);
}

inline internal void VK_Delete_Context_Add_Pipeline(vk_delete_context* DeleteContext, vk_pipeline* Pipeline, u64 LastUsedFrameIndex) {
    AK_RW_Lock_Reader(&DeleteContext->RWLock);
    u32 ListIndex = DeleteContext->CurrentListIndex;
    VK_Delete_List_Add(&DeleteContext->PipelineList[ListIndex], Pipeline, LastUsedFrameIndex);
    AK_RW_Unlock_Reader(&DeleteContext->RWLock);
}

inline internal async_handle<vk_render_pass> VK_Context_Allocate_Render_Pass(gdi_context* Context) {
    async_handle<vk_render_pass> RenderPassHandle = Async_Pool_Allocate(&Context->ResourceContext.RenderPasses);
    if(RenderPassHandle.Is_Null()) {
        return {};
    }
    Assert(Context->ResourceContext.RenderPassLastFrameIndices[RenderPassHandle.Index()] == (u64)-1);
    Assert(AK_Atomic_Load_U32_Relaxed(&Context->ResourceContext.RenderPassesInUse[RenderPassHandle.Index()]) == false);
    return RenderPassHandle;
}

inline internal async_handle<vk_buffer> VK_Context_Allocate_Buffer(gdi_context* Context) {
    async_handle<vk_buffer> BufferHandle = Async_Pool_Allocate(&Context->ResourceContext.Buffers);
    if(BufferHandle.Is_Null()) {
        return {};
    }
    Assert(Context->ResourceContext.BufferLastFrameIndices[BufferHandle.Index()] == (u64)-1);
    Assert(AK_Atomic_Load_U32_Relaxed(&Context->ResourceContext.BuffersInUse[BufferHandle.Index()]) == false);
    return BufferHandle;
}

inline internal async_handle<vk_texture> VK_Context_Allocate_Texture(gdi_context* Context) {
    async_handle<vk_texture> TextureHandle = Async_Pool_Allocate(&Context->ResourceContext.Textures);
    if(TextureHandle.Is_Null()) {
        return {};
    }
    Assert(Context->ResourceContext.TextureLastFrameIndices[TextureHandle.Index()] == (u64)-1);
    Assert(AK_Atomic_Load_U32_Relaxed(&Context->ResourceContext.TexturesInUse[TextureHandle.Index()]) == false);
    return TextureHandle;
}

inline internal async_handle<vk_texture_view> VK_Context_Allocate_Texture_View(gdi_context* Context) {
    async_handle<vk_texture_view> TextureViewHandle = Async_Pool_Allocate(&Context->ResourceContext.TextureViews);
    if(TextureViewHandle.Is_Null()) {
        return {};
    }
    Assert(Context->ResourceContext.TextureViewLastFrameIndices[TextureViewHandle.Index()] == (u64)-1);
    Assert(AK_Atomic_Load_U32_Relaxed(&Context->ResourceContext.TextureViewsInUse[TextureViewHandle.Index()]) == false);
    return TextureViewHandle;
}

inline internal async_handle<vk_framebuffer> VK_Context_Allocate_Framebuffer(gdi_context* Context) {
    async_handle<vk_framebuffer> FramebufferHandle = Async_Pool_Allocate(&Context->ResourceContext.Framebuffers);
    if(FramebufferHandle.Is_Null()) {
        return {};
    }
    Assert(Context->ResourceContext.FramebufferLastFrameIndices[FramebufferHandle.Index()] == (u64)-1);
    Assert(AK_Atomic_Load_U32_Relaxed(&Context->ResourceContext.FramebuffersInUse[FramebufferHandle.Index()]) == false);
    return FramebufferHandle;
}

inline internal async_handle<vk_swapchain> VK_Context_Allocate_Swapchain(gdi_context* Context) {
    async_handle<vk_swapchain> SwapchainHandle = Async_Pool_Allocate(&Context->ResourceContext.Swapchains);
    if(SwapchainHandle.Is_Null()) {
        return {};
    }
    Assert(Context->ResourceContext.SwapchainLastFrameIndices[SwapchainHandle.Index()] == (u64)-1);
    Assert(AK_Atomic_Load_U32_Relaxed(&Context->ResourceContext.SwapchainsInUse[SwapchainHandle.Index()]) == false);
    return SwapchainHandle;
}

inline internal async_handle<vk_pipeline> VK_Context_Allocate_Pipeline(gdi_context* Context) {
    async_handle<vk_pipeline> PipelineHandle = Async_Pool_Allocate(&Context->ResourceContext.Pipelines);
    if(PipelineHandle.Is_Null()) {
        return {};
    }
    Assert(Context->ResourceContext.PipelineLastFrameIndices[PipelineHandle.Index()] == (u64)-1);
    Assert(AK_Atomic_Load_U32_Relaxed(&Context->ResourceContext.PipelinesInUse[PipelineHandle.Index()]) == false);
    return PipelineHandle;
}

inline internal void VK_Context_Free_Render_Pass(gdi_context* Context, async_handle<vk_render_pass> RenderPassHandle) {
    Context->ResourceContext.RenderPassLastFrameIndices[RenderPassHandle.Index()] = (u64)-1;
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.RenderPassesInUse[RenderPassHandle.Index()], false);
    Async_Pool_Free(&Context->ResourceContext.RenderPasses, RenderPassHandle);
}

inline internal void VK_Context_Free_Buffer(gdi_context* Context, async_handle<vk_buffer> BufferHandle) {
    Context->ResourceContext.BufferLastFrameIndices[BufferHandle.Index()] = (u64)-1;
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.BuffersInUse[BufferHandle.Index()], false);
    Async_Pool_Free(&Context->ResourceContext.Buffers, BufferHandle);
}

inline internal void VK_Context_Free_Texture(gdi_context* Context, async_handle<vk_texture> TextureHandle) {
    Context->ResourceContext.TextureLastFrameIndices[TextureHandle.Index()] = (u64)-1;
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.TexturesInUse[TextureHandle.Index()], false);
    Async_Pool_Free(&Context->ResourceContext.Textures, TextureHandle);
}

inline internal void VK_Context_Free_Texture_View(gdi_context* Context, async_handle<vk_texture_view> TextureViewHandle) {
    Context->ResourceContext.TextureViewLastFrameIndices[TextureViewHandle.Index()] = (u64)-1;
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.TextureViewsInUse[TextureViewHandle.Index()], false);
    Async_Pool_Free(&Context->ResourceContext.TextureViews, TextureViewHandle);
}

inline internal void VK_Context_Free_Framebuffer(gdi_context* Context, async_handle<vk_framebuffer> FramebufferHandle) {
    Context->ResourceContext.FramebufferLastFrameIndices[FramebufferHandle.Index()] = (u64)-1;
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.FramebuffersInUse[FramebufferHandle.Index()], false);
    Async_Pool_Free(&Context->ResourceContext.Framebuffers, FramebufferHandle);
}

inline internal void VK_Context_Free_Swapchain(gdi_context* Context, async_handle<vk_swapchain> SwapchainHandle) {
    Context->ResourceContext.SwapchainLastFrameIndices[SwapchainHandle.Index()] = (u64)-1;
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.SwapchainsInUse[SwapchainHandle.Index()], false);
    Async_Pool_Free(&Context->ResourceContext.Swapchains, SwapchainHandle);
}

inline internal void VK_Context_Free_Pipeline(gdi_context* Context, async_handle<vk_pipeline> PipelineHandle) {
    Context->ResourceContext.PipelineLastFrameIndices[PipelineHandle.Index()] = (u64)-1;
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.PipelinesInUse[PipelineHandle.Index()], false);
    Async_Pool_Free(&Context->ResourceContext.Pipelines, PipelineHandle);
} 

void VK_Free_Cmd_List(gdi_context* Context, vk_cmd_pool* CmdPool, vk_cmd_list* CmdList) {
    if(CmdList) {
        if(CmdList->CmdPool) {
            vkDestroyCommandPool(Context->Device, CmdList->CmdPool, Context->VKAllocator);
            CmdList->CmdPool = VK_NULL_HANDLE;
        }

        DLL_Remove(CmdPool->CurrentCmdListHead, CmdPool->CurrentCmdListTail, CmdList);
        SLL_Push_Front(CmdPool->FreeCmdList, CmdList);
    }
}

vk_cmd_list* VK_Allocate_Cmd_List(gdi_context* Context, vk_cmd_pool* CmdPool) {
    vk_cmd_list* Result = CmdPool->FreeCmdList;
    if(Result) SLL_Pop_Front(CmdPool->FreeCmdList);
    else {
        VkSemaphore SubmitLock = VK_NULL_HANDLE;
        VkSemaphore PresentLock = VK_NULL_HANDLE;
        if(CmdPool->Level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
            VkSemaphoreCreateInfo SemaphoreCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };

            vkCreateSemaphore(Context->Device, &SemaphoreCreateInfo, Context->VKAllocator, &SubmitLock);
            vkCreateSemaphore(Context->Device, &SemaphoreCreateInfo, Context->VKAllocator, &PresentLock);
        }

        VkCommandPoolCreateInfo CmdPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = Context->PhysicalDevice->GraphicsQueueFamilyIndex
        };

        VkCommandPool VkCmdPool;
        if(vkCreateCommandPool(Context->Device, &CmdPoolCreateInfo, Context->VKAllocator, &VkCmdPool) != VK_SUCCESS) {
            //todo: diagnostics
            if(SubmitLock)  vkDestroySemaphore(Context->Device, SubmitLock, Context->VKAllocator);
            if(PresentLock) vkDestroySemaphore(Context->Device, PresentLock, Context->VKAllocator);
            return NULL;
        }

        VkCommandBufferAllocateInfo CmdBufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = VkCmdPool,
            .level = CmdPool->Level,
            .commandBufferCount = 1,
        };

        VkCommandBuffer CmdBuffer;
        if(vkAllocateCommandBuffers(Context->Device, &CmdBufferAllocateInfo, &CmdBuffer) != VK_SUCCESS) {
            //todo: diagnostics
            if(SubmitLock)  vkDestroySemaphore(Context->Device, SubmitLock, Context->VKAllocator);
            if(PresentLock) vkDestroySemaphore(Context->Device, PresentLock, Context->VKAllocator);
            vkDestroyCommandPool(Context->Device, VkCmdPool, Context->VKAllocator);
            return NULL;
        }

        Result = Arena_Push_Struct(Context->Arena, vk_cmd_list);
        Result->Context = Context;
        Result->CmdPool = VkCmdPool;
        Result->CmdBuffer = CmdBuffer;
        Result->SubmitLock = SubmitLock;
        Result->PresentLock = PresentLock;
    }
    Result->SwapchainTextureIndex = (u32)-1;
    Result->Next = Result->Prev = NULL;

    DLL_Push_Back(CmdPool->CurrentCmdListHead, CmdPool->CurrentCmdListTail, Result);

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

    VK_Memory_Manager_Create(&Context->MemoryManager, Context);

    Context->Frames = array<vk_frame_context>(Context->Arena, CreateInfo.FrameCount);
    Array_Resize(&Context->Frames, CreateInfo.FrameCount);
    
    for(u32 i = 0; i < CreateInfo.FrameCount; i++) {
        vk_frame_context* Frame = &Context->Frames[i];

        VkCommandPoolCreateInfo CmdPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = Context->PhysicalDevice->GraphicsQueueFamilyIndex
        };

        if(vkCreateCommandPool(Context->Device, &CmdPoolCreateInfo, Context->VKAllocator, &Frame->CopyCmdPool) != VK_SUCCESS) {
            //todo: logging
            return false;
        }

        VkCommandBufferAllocateInfo CmdBufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = Frame->CopyCmdPool,
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
        } else {
            if(vkResetCommandPool(Context->Device, Frame->CopyCmdPool, 0) != VK_SUCCESS) {
                //todo: logging
                return false;
            }

            VkCommandBufferBeginInfo BeginInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            };

            if(vkBeginCommandBuffer(Frame->CopyCmdBuffer, &BeginInfo) != VK_SUCCESS) {
                //todo: logging
                return false;
            }
        }

        vkCreateFence(Context->Device, &FenceCreateInfo, Context->VKAllocator, &Frame->Fence);

        Frame->PrimaryCmdPool.Level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        Frame->SecondaryCmdPool.Level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    }

    AK_Mutex_Create(&Context->ThreadContextLock);
    Context->ThreadContextArena = Arena_Create(Context->GDI->MainAllocator);
    AK_TLS_Create(&Context->ThreadContextTLS);

    vk_resource_context* ResourceContext = &Context->ResourceContext;
    Async_Pool_Create(&ResourceContext->RenderPasses, Context->Arena, CreateInfo.RenderPassCount);
    Async_Pool_Create(&ResourceContext->Buffers, Context->Arena, CreateInfo.BufferCount);
    Async_Pool_Create(&ResourceContext->Textures, Context->Arena, CreateInfo.TextureCount);
    Async_Pool_Create(&ResourceContext->TextureViews, Context->Arena, CreateInfo.TextureViewCount);
    Async_Pool_Create(&ResourceContext->Framebuffers, Context->Arena, CreateInfo.FramebufferCount);
    Async_Pool_Create(&ResourceContext->Swapchains, Context->Arena, CreateInfo.SwapchainCount);
    Async_Pool_Create(&ResourceContext->Pipelines, Context->Arena, CreateInfo.PipelineCount);

    ResourceContext->RenderPassesInUse = Arena_Push_Array(Context->Arena, CreateInfo.RenderPassCount, ak_atomic_u32);
    ResourceContext->BuffersInUse = Arena_Push_Array(Context->Arena, CreateInfo.BufferCount, ak_atomic_u32);
    ResourceContext->TexturesInUse = Arena_Push_Array(Context->Arena, CreateInfo.TextureCount, ak_atomic_u32);
    ResourceContext->TextureViewsInUse = Arena_Push_Array(Context->Arena, CreateInfo.TextureViewCount, ak_atomic_u32);
    ResourceContext->FramebuffersInUse = Arena_Push_Array(Context->Arena, CreateInfo.FramebufferCount, ak_atomic_u32);
    ResourceContext->SwapchainsInUse = Arena_Push_Array(Context->Arena, CreateInfo.SwapchainCount, ak_atomic_u32);
    ResourceContext->PipelinesInUse = Arena_Push_Array(Context->Arena, CreateInfo.PipelineCount, ak_atomic_u32);

    ResourceContext->RenderPassLastFrameIndices = Arena_Push_Array(Context->Arena, CreateInfo.RenderPassCount, u64);
    ResourceContext->BufferLastFrameIndices = Arena_Push_Array(Context->Arena, CreateInfo.BufferCount, u64);
    ResourceContext->TextureLastFrameIndices = Arena_Push_Array(Context->Arena, CreateInfo.TextureCount, u64);
    ResourceContext->TextureViewLastFrameIndices = Arena_Push_Array(Context->Arena, CreateInfo.TextureViewCount, u64);
    ResourceContext->FramebufferLastFrameIndices = Arena_Push_Array(Context->Arena, CreateInfo.FramebufferCount, u64);
    ResourceContext->SwapchainLastFrameIndices = Arena_Push_Array(Context->Arena, CreateInfo.SwapchainCount, u64);
    ResourceContext->PipelineLastFrameIndices = Arena_Push_Array(Context->Arena, CreateInfo.PipelineCount, u64);

    for(u32 i = 0; i < CreateInfo.RenderPassCount; i++) { ResourceContext->RenderPassLastFrameIndices[i] = (u64)-1; }
    for(u32 i = 0; i < CreateInfo.BufferCount; i++) { ResourceContext->BufferLastFrameIndices[i] = (u64)-1; }
    for(u32 i = 0; i < CreateInfo.TextureCount; i++) { ResourceContext->TextureLastFrameIndices[i] = (u64)-1; }
    for(u32 i = 0; i < CreateInfo.TextureViewCount; i++) { ResourceContext->TextureViewLastFrameIndices[i] = (u64)-1; }
    for(u32 i = 0; i < CreateInfo.FramebufferCount; i++) { ResourceContext->FramebufferLastFrameIndices[i] = (u64)-1; }
    for(u32 i = 0; i < CreateInfo.SwapchainCount; i++) { ResourceContext->SwapchainLastFrameIndices[i] = (u64)-1; }
    for(u32 i = 0; i < CreateInfo.PipelineCount; i++) { ResourceContext->PipelineLastFrameIndices[i] = (u64)-1; }

    return true;
}

vk_frame_context* VK_Get_Current_Frame_Context(gdi_context* Context) {
    return &Context->Frames[Context->TotalFramesRendered % Context->Frames.Count];
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
        

        vk_thread_context* ThreadContext = (vk_thread_context*)AK_Atomic_Load_Ptr_Relaxed(&Context->ThreadContextList);
        while(ThreadContext) {

            vk_delete_context* DeleteContext = &ThreadContext->DeleteContext;
            for(u32 i = 0; i < 2; i++) {
                for(vk_delete_list_entry<vk_render_pass>& RenderPassEntry : DeleteContext->RenderPassList[i]) {
                    VK_Delete_Render_Pass(Context, &RenderPassEntry.Resource);
                }

                for(vk_delete_list_entry<vk_buffer>& BufferEntry : DeleteContext->BufferList[i]) {
                    VK_Delete_Buffer(Context, &BufferEntry.Resource);
                }

                for(vk_delete_list_entry<vk_texture_view>& TextureViewEntry : DeleteContext->TextureViewList[i]) {
                    VK_Delete_Texture_View(Context, &TextureViewEntry.Resource);
                }

                for(vk_delete_list_entry<vk_framebuffer>& FramebufferEntry : DeleteContext->FramebufferList[i]) {
                    VK_Delete_Framebuffer(Context, &FramebufferEntry.Resource);
                }

                for(vk_delete_list_entry<vk_swapchain>& SwapchainEntry : DeleteContext->SwapchainList[i]) {
                    VK_Delete_Swapchain(Context, &SwapchainEntry.Resource);
                }

                for(vk_delete_list_entry<vk_pipeline>& PipelineEntry : DeleteContext->PipelineList[i]) {
                    VK_Delete_Pipeline(Context, &PipelineEntry.Resource);
                }

                VK_Delete_List_Free(&DeleteContext->RenderPassList[i]);
                VK_Delete_List_Free(&DeleteContext->BufferList[i]);
                VK_Delete_List_Free(&DeleteContext->TextureViewList[i]);
                VK_Delete_List_Free(&DeleteContext->FramebufferList[i]);
                VK_Delete_List_Free(&DeleteContext->SwapchainList[i]);
                VK_Delete_List_Free(&DeleteContext->PipelineList[i]);
            }

            for(vk_upload_buffer& UploadBuffer : ThreadContext->UploadBuffers) {
                VK_Upload_Buffer_Delete(&UploadBuffer);
            }

            ThreadContext = ThreadContext->Next;
        }

        //Delete any remaining resources that are not on the delete queue
        for(u32 i = 0; i < Async_Pool_Capacity(&ResourceContext->RenderPasses); i++) {
            vk_render_pass* RenderPass = ResourceContext->RenderPasses.Ptr + i; 
            if(RenderPass->RenderPass) {
                VK_Delete_Render_Pass(Context, RenderPass);
            }
        }

        for(u32 i = 0; i < Async_Pool_Capacity(&ResourceContext->Buffers); i++) {
            vk_buffer* Buffer = ResourceContext->Buffers.Ptr + i; 
            if(Buffer->Buffer) {
                VK_Delete_Buffer(Context, Buffer);
            }
        }

        for(u32 i = 0; i < Async_Pool_Capacity(&ResourceContext->TextureViews); i++) {
            vk_texture_view* TextureView = ResourceContext->TextureViews.Ptr + i; 
            if(TextureView->ImageView) {
                VK_Delete_Texture_View(Context, TextureView);
            }
        }

        for(u32 i = 0; i < Async_Pool_Capacity(&ResourceContext->Framebuffers); i++) {
            vk_framebuffer* Framebuffer = ResourceContext->Framebuffers.Ptr + i; 
            if(Framebuffer->Framebuffer) {
                VK_Delete_Framebuffer(Context, Framebuffer);
            }
        }

        for(u32 i = 0; i < Async_Pool_Capacity(&ResourceContext->Swapchains); i++) {
            vk_swapchain* Swapchain = ResourceContext->Swapchains.Ptr + i; 
            if(Swapchain->Swapchain) {
                VK_Delete_Swapchain_Full(Context, Swapchain);
            }
        }

        for(u32 i = 0; i < Async_Pool_Capacity(&ResourceContext->Pipelines); i++) {
            vk_pipeline* Pipeline = ResourceContext->Pipelines.Ptr + i;
            if(Pipeline->Pipeline) {
                VK_Delete_Pipeline(Context, Pipeline);
            }
        }

        AK_Mutex_Delete(&Context->ThreadContextLock);
        AK_TLS_Delete(&Context->ThreadContextTLS);
        VK_Memory_Manager_Delete(&Context->MemoryManager);
        Arena_Delete(Context->ThreadContextArena);

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

gdi_handle<gdi_buffer> GDI_Context_Create_Buffer(gdi_context* Context, const gdi_buffer_create_info& CreateInfo) {
    async_handle<vk_buffer> BufferHandle = VK_Context_Allocate_Buffer(Context);
    if(BufferHandle.Is_Null()) {
        //todo: Diagnostics
        return {};
    }

    pool_writer_lock BufferLock(&Context->ResourceContext.Buffers, BufferHandle);
    if(!VK_Create_Buffer(Context, BufferLock.Ptr, CreateInfo)) {
        VK_Delete_Buffer(Context, BufferLock.Ptr);
        BufferLock.Unlock();
        VK_Context_Free_Buffer(Context, BufferHandle);
        return {};
    }

    if(CreateInfo.InitialData.Ptr && CreateInfo.InitialData.Size) {
        if(!VK_Buffer_Upload(Context, BufferHandle, CreateInfo.InitialData)) {
            VK_Delete_Buffer(Context, BufferLock.Ptr);
            BufferLock.Unlock();
            VK_Context_Free_Buffer(Context, BufferHandle);
            return {};
        }
    }

    BufferLock.Unlock();
    return gdi_handle<gdi_buffer>(BufferHandle.ID);
}

void GDI_Context_Delete_Buffer(gdi_context* Context, gdi_handle<gdi_buffer> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    async_handle<vk_buffer> BufferHandle(Handle.ID);
    pool_reader_lock BufferReader(&ResourceContext->Buffers, BufferHandle);
    if(BufferReader.Ptr) {
        vk_buffer Buffer = *BufferReader.Ptr;
        BufferReader.Unlock();
        u64 LastUsedFrameIndex = ResourceContext->BufferLastFrameIndices[BufferHandle.Index()];
        u64 Difference = Context->TotalFramesRendered - LastUsedFrameIndex;
        if(LastUsedFrameIndex == (u64)-1 || 
           (!AK_Atomic_Load_U32_Relaxed(&ResourceContext->BuffersInUse[BufferHandle.Index()]) &&
           (!Difference || Difference > Context->Frames.Count))) {
            VK_Delete_Buffer(Context, &Buffer);
        } else {
            u64 FrameIndex = AK_Atomic_Load_U32_Relaxed(&ResourceContext->BuffersInUse[BufferHandle.Index()]) ? Context->TotalFramesRendered : LastUsedFrameIndex;
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(Context);
            VK_Delete_Context_Add_Buffer(DeleteContext, &Buffer, FrameIndex);
        }
    }

    VK_Context_Free_Buffer(Context, BufferHandle);
}

gdi_handle<gdi_render_pass> GDI_Context_Create_Render_Pass(gdi_context* Context, const gdi_render_pass_create_info& CreateInfo) { 
    async_handle<vk_render_pass> RenderPassHandle = VK_Context_Allocate_Render_Pass(Context);
    if(RenderPassHandle.Is_Null()) {
        //todo: Diagnostics
        return {};
    }

    pool_writer_lock RenderPassLock(&Context->ResourceContext.RenderPasses, RenderPassHandle);
    if(!VK_Create_Render_Pass(Context, RenderPassLock.Ptr, CreateInfo)) {
        //todo: Diagnostic 
        VK_Delete_Render_Pass(Context, RenderPassLock.Ptr);
        RenderPassLock.Unlock();
        VK_Context_Free_Render_Pass(Context, RenderPassHandle);
        return {};
    }
    RenderPassLock.Unlock();
    return gdi_handle<gdi_render_pass>(RenderPassHandle.ID);
}

void GDI_Context_Delete_Render_Pass(gdi_context* Context, gdi_handle<gdi_render_pass> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    async_handle<vk_render_pass> RenderPassHandle(Handle.ID);
    pool_reader_lock RenderPassReader(&ResourceContext->RenderPasses, RenderPassHandle);
    if(RenderPassReader.Ptr) {
        vk_render_pass RenderPass = *RenderPassReader.Ptr;
        RenderPassReader.Unlock();
        u64 LastUsedFrameIndex = ResourceContext->RenderPassLastFrameIndices[RenderPassHandle.Index()];
        u64 Difference = Context->TotalFramesRendered - LastUsedFrameIndex;
        if(LastUsedFrameIndex == (u64)-1 || 
           (!AK_Atomic_Load_U32_Relaxed(&ResourceContext->RenderPassesInUse[RenderPassHandle.Index()]) &&
           (!Difference || Difference > Context->Frames.Count))) {
            VK_Delete_Render_Pass(Context, &RenderPass);
        } else {
            u64 FrameIndex = AK_Atomic_Load_U32_Relaxed(&ResourceContext->RenderPassesInUse[RenderPassHandle.Index()]) ? Context->TotalFramesRendered : LastUsedFrameIndex;
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(Context);
            VK_Delete_Context_Add_Render_Pass(DeleteContext, &RenderPass, FrameIndex);
        }
    }

    VK_Context_Free_Render_Pass(Context, RenderPassHandle);
}

gdi_handle<gdi_texture_view> GDI_Context_Create_Texture_View(gdi_context* Context, const gdi_texture_view_create_info& CreateInfo) {
    async_handle<vk_texture_view> TextureViewHandle = VK_Context_Allocate_Texture_View(Context);
    if(TextureViewHandle.Is_Null()) {
        //todo: Diagnostics
        return {};
    }

    pool_writer_lock TextureViewLock(&Context->ResourceContext.TextureViews, TextureViewHandle);
    if(!VK_Create_Texture_View(Context, TextureViewLock.Ptr, CreateInfo)) {
        //todo: Diagnostic 
        VK_Delete_Texture_View(Context, TextureViewLock.Ptr);
        TextureViewLock.Unlock();
        VK_Context_Free_Texture_View(Context, TextureViewHandle);
        return {};
    }
    TextureViewLock.Unlock();
    return gdi_handle<gdi_texture_view>(TextureViewHandle.ID);
}

void GDI_Context_Delete_Texture_View(gdi_context* Context, gdi_handle<gdi_texture_view> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    async_handle<vk_texture_view> TextureViewHandle(Handle.ID);
    pool_reader_lock TextureViewReader(&ResourceContext->TextureViews, TextureViewHandle);
    if(TextureViewReader.Ptr) {
        vk_texture_view TextureView = *TextureViewReader.Ptr;
        TextureViewReader.Unlock();

        u64 LastUsedFrameIndex = ResourceContext->TextureViewLastFrameIndices[TextureViewHandle.Index()];
        u64 Difference = Context->TotalFramesRendered - LastUsedFrameIndex;
        if(LastUsedFrameIndex == (u64)-1 || 
           (!AK_Atomic_Load_U32_Relaxed(&ResourceContext->TextureViewsInUse[TextureViewHandle.Index()]) && 
           (!Difference || Difference > Context->Frames.Count))) {
            VK_Delete_Texture_View(Context, &TextureView);
        } else {
            u64 FrameIndex = AK_Atomic_Load_U32_Relaxed(&ResourceContext->TextureViewsInUse[TextureViewHandle.Index()]) ? Context->TotalFramesRendered : LastUsedFrameIndex;
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(Context);
            VK_Delete_Context_Add_Texture_View(DeleteContext, &TextureView, FrameIndex);
        }
    }

    VK_Context_Free_Texture_View(Context, TextureViewHandle);
}

gdi_handle<gdi_framebuffer> GDI_Context_Create_Framebuffer(gdi_context* Context, const gdi_framebuffer_create_info& CreateInfo) {
    async_handle<vk_framebuffer> FramebufferHandle = VK_Context_Allocate_Framebuffer(Context);
    if(FramebufferHandle.Is_Null()) {
        //todo: Diagnostics
        return {};
    }

    pool_writer_lock FramebufferLock(&Context->ResourceContext.Framebuffers, FramebufferHandle);
    if(!VK_Create_Framebuffer(Context, FramebufferLock.Ptr, CreateInfo)) {
        //todo: Diagnostics
        VK_Delete_Framebuffer(Context, FramebufferLock.Ptr);
        FramebufferLock.Unlock();
        VK_Context_Free_Framebuffer(Context, FramebufferHandle);
        return {};
    }
    FramebufferLock.Unlock();
    return gdi_handle<gdi_framebuffer>(FramebufferHandle.ID);
}

void GDI_Context_Delete_Framebuffer(gdi_context* Context, gdi_handle<gdi_framebuffer> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    async_handle<vk_framebuffer> FramebufferHandle(Handle.ID);
    pool_reader_lock FramebufferReader(&ResourceContext->Framebuffers, FramebufferHandle);
    if(FramebufferReader.Ptr) {
        vk_framebuffer Framebuffer = *FramebufferReader.Ptr;
        FramebufferReader.Unlock();

        u64 LastUsedFrameIndex = ResourceContext->FramebufferLastFrameIndices[FramebufferHandle.Index()];
        u64 Difference = Context->TotalFramesRendered - LastUsedFrameIndex;
        if(LastUsedFrameIndex == (u64)-1 ||
           (!AK_Atomic_Load_U32_Relaxed(&ResourceContext->FramebuffersInUse[FramebufferHandle.Index()]) &&
           (!Difference || Difference > Context->Frames.Count))) {
            VK_Delete_Framebuffer(Context, &Framebuffer);
        } else {
            u64 FrameIndex = AK_Atomic_Load_U32_Relaxed(&ResourceContext->FramebuffersInUse[FramebufferHandle.Index()]) ? Context->TotalFramesRendered : LastUsedFrameIndex;
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(Context);
            VK_Delete_Context_Add_Framebuffer(DeleteContext, &Framebuffer, FrameIndex);
        }
    }

    VK_Context_Free_Framebuffer(Context, FramebufferHandle);
}

gdi_handle<gdi_swapchain> GDI_Context_Create_Swapchain(gdi_context* Context, const gdi_swapchain_create_info& CreateInfo) {
    async_handle<vk_swapchain> SwapchainHandle = VK_Context_Allocate_Swapchain(Context);
    if(SwapchainHandle.Is_Null()) {
        //todo: diagnostics
        return {};
    }

    pool_writer_lock SwapchainLock(&Context->ResourceContext.Swapchains, SwapchainHandle);
    if(!VK_Create_Swapchain(Context, SwapchainLock.Ptr, CreateInfo)) {
        //todo: Diagnostics
        VK_Delete_Swapchain(Context, SwapchainLock.Ptr);
        SwapchainLock.Unlock();
        VK_Context_Free_Swapchain(Context, SwapchainHandle);
        return {};
    }

    if(!VK_Create_Swapchain_Textures(Context, SwapchainLock.Ptr)) {
        //todo: Diagnostics
        VK_Delete_Swapchain_Full(Context, SwapchainLock.Ptr);
        SwapchainLock.Unlock();
        VK_Context_Free_Swapchain(Context, SwapchainHandle);
        return {};
    }

    SwapchainLock.Unlock();
    return gdi_handle<gdi_swapchain>(SwapchainHandle.ID);
}

void GDI_Context_Delete_Swapchain(gdi_context* Context, gdi_handle<gdi_swapchain> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    async_handle<vk_swapchain> SwapchainHandle(Handle.ID);
    pool_reader_lock SwapchainReader(&ResourceContext->Swapchains, SwapchainHandle);
    if(SwapchainReader.Ptr) {
        vk_swapchain Swapchain = *SwapchainReader.Ptr;
        SwapchainReader.Unlock();

        VK_Delete_Swapchain_Textures(Context, &Swapchain);    

        u64 LastUsedFrameIndex = ResourceContext->SwapchainLastFrameIndices[SwapchainHandle.Index()];
        u64 Difference = Context->TotalFramesRendered - LastUsedFrameIndex;
        if(LastUsedFrameIndex == (u64)-1 || 
           (!AK_Atomic_Load_U32_Relaxed(&ResourceContext->SwapchainsInUse[SwapchainHandle.Index()]) &&
           (!Difference || Difference > Context->Frames.Count))) {
            VK_Delete_Swapchain(Context, &Swapchain);
        } else {
            u64 FrameIndex = AK_Atomic_Load_U32_Relaxed(&ResourceContext->SwapchainsInUse[SwapchainHandle.Index()]) ? Context->TotalFramesRendered : LastUsedFrameIndex;
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(Context);
            VK_Delete_Context_Add_Swapchain(DeleteContext, &Swapchain, FrameIndex);
        }
    }

    VK_Context_Free_Swapchain(Context, SwapchainHandle);
}

bool GDI_Context_Resize_Swapchain(gdi_context* Context, gdi_handle<gdi_swapchain> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    async_handle<vk_swapchain> SwapchainHandle(Handle.ID);
    pool_writer_lock SwapchainWriter(&ResourceContext->Swapchains, SwapchainHandle);
    if(SwapchainWriter.Ptr) {
        //Copy the old swapchain so we can write the new one into the writer lock
        vk_swapchain Swapchain = *SwapchainWriter.Ptr;

        gdi_swapchain_create_info CreateInfo = {
            .TargetFormat = SwapchainWriter->Format,
            .UsageFlags = SwapchainWriter->UsageFlags
        };

        VK_Delete_Swapchain_Textures(Context, SwapchainWriter.Ptr);    
        if(!VK_Create_Swapchain(Context, SwapchainWriter.Ptr, CreateInfo)) {
            //todo: some error logging
            return false;
        }

        if(!VK_Create_Swapchain_Textures(Context, SwapchainWriter.Ptr)) {
            //todo: some error logging
            return false;
        }
        SwapchainWriter.Unlock();

        //Prevent the surface from being deleted by setting it to null when we resize
        Swapchain.Surface = VK_NULL_HANDLE;
        u64 LastUsedFrameIndex = ResourceContext->SwapchainLastFrameIndices[SwapchainHandle.Index()];
        u64 Difference = Context->TotalFramesRendered - LastUsedFrameIndex;
        if(LastUsedFrameIndex == (u64)-1 || 
           (!AK_Atomic_Load_U32_Relaxed(&ResourceContext->SwapchainsInUse[SwapchainHandle.Index()]) &&
           (!Difference || Difference > Context->Frames.Count))) {
            VK_Delete_Swapchain(Context, &Swapchain);
        } else {
            u64 FrameIndex = AK_Atomic_Load_U32_Relaxed(&ResourceContext->SwapchainsInUse[SwapchainHandle.Index()]) ? Context->TotalFramesRendered : LastUsedFrameIndex;
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(Context);
            VK_Delete_Context_Add_Swapchain(DeleteContext, &Swapchain, FrameIndex);
        
        }
    }
    return true;
}

span<gdi_handle<gdi_texture>> GDI_Context_Get_Swapchain_Textures(gdi_context* Context, gdi_handle<gdi_swapchain> Handle) {
    async_handle<vk_swapchain> SwapchainHandle(Handle.ID);
    pool_reader_lock SwapchainReader(&Context->ResourceContext.Swapchains, SwapchainHandle);
    pool_scoped_lock SwapchainReaderLock(&SwapchainReader);
    if(SwapchainReader.Ptr) {
        return span<gdi_handle<gdi_texture>>(SwapchainReader->Textures);
    }
    return span<gdi_handle<gdi_texture>>();
}

gdi_handle<gdi_pipeline> GDI_Context_Create_Graphics_Pipeline(gdi_context* Context, const gdi_graphics_pipeline_create_info& CreateInfo) {
    async_handle<vk_pipeline> PipelineHandle = VK_Context_Allocate_Pipeline(Context);
    if(PipelineHandle.Is_Null()) {
        //todo: Diagnostics
        return {};
    }

    pool_writer_lock PipelineLock(&Context->ResourceContext.Pipelines, PipelineHandle);
    if(!VK_Create_Pipeline(Context, PipelineLock.Ptr, CreateInfo)) {
        //todo: Diagnostics
        VK_Delete_Pipeline(Context, PipelineLock.Ptr);
        PipelineLock.Unlock();
        VK_Context_Free_Pipeline(Context, PipelineHandle);
        return {};
    }

    PipelineLock.Unlock();
    return gdi_handle<gdi_pipeline>(PipelineHandle.ID);
}

void GDI_Context_Delete_Pipeline(gdi_context* Context, gdi_handle<gdi_pipeline> Handle) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    async_handle<vk_pipeline> PipelineHandle(Handle.ID);
    pool_reader_lock PipelineReader(&ResourceContext->Pipelines, PipelineHandle);
    if(PipelineReader.Ptr) {
        vk_pipeline Pipeline = *PipelineReader.Ptr;
        PipelineReader.Unlock();
        u64 LastUsedFrameIndex = ResourceContext->PipelineLastFrameIndices[PipelineHandle.Index()];
        u64 Difference = Context->TotalFramesRendered - LastUsedFrameIndex;
        if(LastUsedFrameIndex == (u64)-1 || 
           (!AK_Atomic_Load_U32_Relaxed(&ResourceContext->PipelinesInUse[PipelineHandle.Index()]) &&
           (!Difference || Difference > Context->Frames.Count))) {
            VK_Delete_Pipeline(Context, &Pipeline);
        } else {
            u64 FrameIndex = AK_Atomic_Load_U32_Relaxed(&ResourceContext->PipelinesInUse[PipelineHandle.Index()]) ? Context->TotalFramesRendered : LastUsedFrameIndex;
            vk_delete_context* DeleteContext = VK_Get_Delete_Context(Context);
            VK_Delete_Context_Add_Pipeline(DeleteContext, &Pipeline, FrameIndex);
        }
    }
    VK_Context_Free_Pipeline(Context, PipelineHandle);
}

gdi_cmd_list* GDI_Context_Begin_Cmd_List(gdi_context* Context, gdi_cmd_list_type Type, gdi_handle<gdi_swapchain> Swapchain) {
    vk_frame_context* FrameContext = VK_Get_Current_Frame_Context(Context);
    
    async_handle<vk_swapchain> SwapchainHandle(Swapchain.ID);
    pool_reader_lock SwapchainReader(&Context->ResourceContext.Swapchains, SwapchainHandle);

    vk_cmd_pool* CmdPool;
    vk_cmd_list* CmdList;

    if(SwapchainReader.Ptr) {
        pool_scoped_lock SwapchainScoped(&SwapchainReader);
        CmdPool = &FrameContext->PrimaryCmdPool;
        CmdList = VK_Allocate_Cmd_List(Context, CmdPool);
        //Just in case the swapchain is suboptimal we will need to perform 
        //this in a loop to try again
        for(;;) {
            if(!SwapchainReader.Ptr) break;
            VkResult SwapchainStatus = vkAcquireNextImageKHR(Context->Device, SwapchainReader->Swapchain, UINT64_MAX, CmdList->SubmitLock, VK_NULL_HANDLE, &CmdList->SwapchainTextureIndex);
            if(SwapchainStatus == VK_SUBOPTIMAL_KHR) {
                SwapchainScoped.Unlock();
                if(!GDI_Context_Resize_Swapchain(Context, Swapchain)) {
                    //todo: error logging
                    VK_Free_Cmd_List(Context, CmdPool, CmdList);
                    return NULL;
                }
                SwapchainReader = pool_reader_lock(&Context->ResourceContext.Swapchains, SwapchainHandle);
                SwapchainScoped = pool_scoped_lock(&SwapchainReader);
            } else if(SwapchainStatus != VK_SUCCESS) {
                //todo: error logging
                VK_Free_Cmd_List(Context, CmdPool, CmdList);
                return NULL;
            } else {
                //Success and we can break and write that the swapchain is
                //used in this frame
                SwapchainScoped.Unlock();
                CmdList->SwapchainHandle = async_handle<vk_swapchain>(Swapchain.ID);
                VK_Swapchain_Record_Frame(Context, SwapchainHandle);
                break;
            }
        }
    } else {
        CmdPool = &FrameContext->SecondaryCmdPool;
        CmdList = VK_Allocate_Cmd_List(Context, CmdPool);
    }
    
    if(vkResetCommandPool(Context->Device, CmdList->CmdPool, 0) != VK_SUCCESS) {
        //todo: error logging
        VK_Free_Cmd_List(Context, CmdPool, CmdList);
        return NULL;
    }

    VkCommandBufferBeginInfo BeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    if(CmdPool->Level == VK_COMMAND_BUFFER_LEVEL_SECONDARY) {
        BeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

    if(vkBeginCommandBuffer(CmdList->CmdBuffer, &BeginInfo) != VK_SUCCESS) {
        //todo: error logging
        VK_Free_Cmd_List(Context, CmdPool, CmdList);
        return NULL;
    }

    return (gdi_cmd_list*)CmdList;
}

void GDI_Context_Execute(gdi_context* Context) {
    scratch Scratch = Scratch_Get();
    vk_frame_context* FrameContext = VK_Get_Current_Frame_Context(Context);
    array<VkCommandBuffer>      CmdBuffers(&Scratch);
    array<VkPipelineStageFlags> SubmitWaitStages(&Scratch);
    array<VkSemaphore>          SubmitSemaphores(&Scratch);
    array<VkSemaphore>          PresentSemaphores(&Scratch);
    array<VkSwapchainKHR>       Swapchains(&Scratch);
    array<uint32_t>             SwapchainImageIndices(&Scratch);
    
    array<pool_reader_lock<vk_swapchain>> SwapchainReaders(&Scratch);

    vk_thread_context* ThreadContext = (vk_thread_context*)AK_Atomic_Load_Ptr_Relaxed(&Context->ThreadContextList);
    while(ThreadContext) {
        vk_copy_context* CopyContext = &ThreadContext->CopyContext;
        u32 CopyIndex = VK_Copy_Context_Swap(CopyContext);
        
        for(const vk_copy_upload_to_buffer& CopyUploadToBuffer : CopyContext->CopyUploadToBufferList[CopyIndex]) {
            pool_reader_lock BufferReader(&Context->ResourceContext.Buffers, CopyUploadToBuffer.Buffer);
            if(BufferReader.Ptr) {
                VkBufferCopy BufferCopy = {
                    .srcOffset = CopyUploadToBuffer.Upload.Offset,
                    .size = CopyUploadToBuffer.Upload.Size
                };
                vkCmdCopyBuffer(FrameContext->CopyCmdBuffer, CopyUploadToBuffer.Upload.Buffer, BufferReader->Buffer, 1, &BufferCopy);
                BufferReader.Unlock();
                VK_Buffer_Record_Frame(Context, CopyUploadToBuffer.Buffer);
            }
        }

        Array_Clear(&CopyContext->CopyUploadToBufferList[CopyIndex]);

        ThreadContext = ThreadContext->Next;
    }

    vkEndCommandBuffer(FrameContext->CopyCmdBuffer);
    Array_Push(&CmdBuffers, FrameContext->CopyCmdBuffer);

    vk_cmd_pool* CmdPool = &FrameContext->PrimaryCmdPool;
    uptr i = 0;
    for(vk_cmd_list* CmdList = CmdPool->CurrentCmdListHead; CmdList; CmdList = CmdList->Next) {
        vkEndCommandBuffer(CmdList->CmdBuffer);
        Array_Push(&CmdBuffers, CmdList->CmdBuffer);
        Array_Push(&SubmitWaitStages, CmdList->ExecuteLockWaitStage);
        Array_Push(&SubmitSemaphores, CmdList->SubmitLock);
        Array_Push(&PresentSemaphores, CmdList->PresentLock);
        Array_Push(&SwapchainImageIndices, CmdList->SwapchainTextureIndex);
        
        pool_reader_lock<vk_swapchain> SwapchainReader(&Context->ResourceContext.Swapchains, CmdList->SwapchainHandle);
        Assert(SwapchainReader.Ptr);
        Array_Push(&Swapchains, SwapchainReader->Swapchain);
        Array_Push(&SwapchainReaders, SwapchainReader);
    }

    VkSubmitInfo SubmitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = Safe_U32(SubmitSemaphores.Count),
        .pWaitSemaphores = SubmitSemaphores.Ptr,
        .commandBufferCount = Safe_U32(CmdBuffers.Count),
        .pCommandBuffers = CmdBuffers.Ptr,
        .signalSemaphoreCount = Safe_U32(PresentSemaphores.Count),
        .pSignalSemaphores = PresentSemaphores.Ptr
    };

    vkQueueSubmit(Context->GraphicsQueue, 1, &SubmitInfo, FrameContext->Fence);

    VkPresentInfoKHR PresentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = Safe_U32(PresentSemaphores.Count),
        .pWaitSemaphores = PresentSemaphores.Ptr,
        .swapchainCount = Safe_U32(Swapchains.Count),
        .pSwapchains = Swapchains.Ptr,
        .pImageIndices = SwapchainImageIndices.Ptr
    };

    vkQueuePresentKHR(Context->PresentQueue, &PresentInfo);

    for(pool_reader_lock<vk_swapchain>& SwapchainReader : SwapchainReaders) {
        SwapchainReader.Unlock();
    }

    while(CmdPool->CurrentCmdListHead) {
        vk_cmd_list* CmdListToDelete = CmdPool->CurrentCmdListHead;
        DLL_Remove_Front(CmdPool->CurrentCmdListHead, CmdPool->CurrentCmdListTail);
        SLL_Push_Front(CmdPool->FreeCmdList, CmdListToDelete);
    }

    CmdPool = &FrameContext->SecondaryCmdPool;
    while(CmdPool->CurrentCmdListHead) {
        vk_cmd_list* CmdListToDelete = CmdPool->CurrentCmdListHead;
        DLL_Remove_Front(CmdPool->CurrentCmdListHead, CmdPool->CurrentCmdListTail);
        SLL_Push_Front(CmdPool->FreeCmdList, CmdListToDelete);
    }

    vk_resource_context* ResourceContext = &Context->ResourceContext;
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->RenderPassLastFrameIndices, ResourceContext->RenderPassesInUse, Async_Pool_Capacity(&ResourceContext->RenderPasses));
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->BufferLastFrameIndices, ResourceContext->BuffersInUse, Async_Pool_Capacity(&ResourceContext->Buffers));
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->TextureLastFrameIndices, ResourceContext->RenderPassesInUse, Async_Pool_Capacity(&ResourceContext->Textures));
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->TextureViewLastFrameIndices, ResourceContext->RenderPassesInUse, Async_Pool_Capacity(&ResourceContext->TextureViews));
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->FramebufferLastFrameIndices, ResourceContext->FramebuffersInUse, Async_Pool_Capacity(&ResourceContext->Framebuffers));
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->SwapchainLastFrameIndices, ResourceContext->SwapchainsInUse, Async_Pool_Capacity(&ResourceContext->Swapchains));
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->PipelineLastFrameIndices, ResourceContext->PipelinesInUse, Async_Pool_Capacity(&ResourceContext->Pipelines));

    Context->TotalFramesRendered++;
    FrameContext = VK_Get_Current_Frame_Context(Context);
    if(vkGetFenceStatus(Context->Device, FrameContext->Fence) == VK_NOT_READY) {
        vkWaitForFences(Context->Device, 1, &FrameContext->Fence, VK_TRUE, UINT64_MAX);
    }
    vkResetFences(Context->Device, 1, &FrameContext->Fence);

    ThreadContext = (vk_thread_context*)AK_Atomic_Load_Ptr_Relaxed(&Context->ThreadContextList);
    while(ThreadContext) {
        vk_upload_buffer* UploadBuffer = VK_Get_Current_Upload_Buffer(Context, ThreadContext);
        VK_Upload_Buffer_Clear(UploadBuffer);

        vk_delete_context* DeleteContext = &ThreadContext->DeleteContext;
        u32 DeleteIndex = VK_Delete_Context_Swap(DeleteContext);
        
        for(vk_delete_list_entry<vk_render_pass>& RenderPassEntry : DeleteContext->RenderPassList[DeleteIndex]) {
            uptr Difference = Context->TotalFramesRendered - RenderPassEntry.LastUsedFrameIndex;
            if(Difference >= Context->Frames.Count) {
                VK_Delete_Render_Pass(Context, &RenderPassEntry.Resource);
            } else {
                VK_Delete_Context_Add_Render_Pass(DeleteContext, &RenderPassEntry.Resource, RenderPassEntry.LastUsedFrameIndex);
            }
        }
        VK_Delete_List_Clear(&DeleteContext->RenderPassList[DeleteIndex]);

        for(vk_delete_list_entry<vk_buffer>& BufferEntry : DeleteContext->BufferList[DeleteIndex]) {
            uptr Difference = Context->TotalFramesRendered - BufferEntry.LastUsedFrameIndex;
            if(Difference >= Context->Frames.Count) {
                VK_Delete_Buffer(Context, &BufferEntry.Resource);
            } else {
                VK_Delete_Context_Add_Buffer(DeleteContext, &BufferEntry.Resource, BufferEntry.LastUsedFrameIndex);
            }
        }
        VK_Delete_List_Clear(&DeleteContext->BufferList[DeleteIndex]);

        for(vk_delete_list_entry<vk_texture_view>& TextureViewEntry : DeleteContext->TextureViewList[DeleteIndex]) {
            uptr Difference = Context->TotalFramesRendered - TextureViewEntry.LastUsedFrameIndex;
            if(Difference >= Context->Frames.Count) {
                VK_Delete_Texture_View(Context, &TextureViewEntry.Resource);
            } else {
                VK_Delete_Context_Add_Texture_View(DeleteContext, &TextureViewEntry.Resource, TextureViewEntry.LastUsedFrameIndex);
            }
        }
        VK_Delete_List_Clear(&DeleteContext->TextureViewList[DeleteIndex]);

        for(vk_delete_list_entry<vk_framebuffer>& FramebufferEntry : DeleteContext->FramebufferList[DeleteIndex]) {
            uptr Difference = Context->TotalFramesRendered - FramebufferEntry.LastUsedFrameIndex;
            if(Difference >= Context->Frames.Count) {
                VK_Delete_Framebuffer(Context, &FramebufferEntry.Resource);
            } else {
                VK_Delete_Context_Add_Framebuffer(DeleteContext, &FramebufferEntry.Resource, FramebufferEntry.LastUsedFrameIndex);
            }
        }
        VK_Delete_List_Clear(&DeleteContext->FramebufferList[DeleteIndex]);

        for(vk_delete_list_entry<vk_swapchain>& SwapchainEntry : DeleteContext->SwapchainList[DeleteIndex]) {
            uptr Difference = Context->TotalFramesRendered - SwapchainEntry.LastUsedFrameIndex;
            if(Difference >= Context->Frames.Count) {
                VK_Delete_Swapchain(Context, &SwapchainEntry.Resource);
            } else {
                VK_Delete_Context_Add_Swapchain(DeleteContext, &SwapchainEntry.Resource, SwapchainEntry.LastUsedFrameIndex);
            }
        }
        VK_Delete_List_Clear(&DeleteContext->SwapchainList[DeleteIndex]);

        for(vk_delete_list_entry<vk_pipeline>& PipelineEntry : DeleteContext->PipelineList[DeleteIndex]) {
            uptr Difference = Context->TotalFramesRendered - PipelineEntry.LastUsedFrameIndex;
            if(Difference >= Context->Frames.Count) {
                VK_Delete_Pipeline(Context, &PipelineEntry.Resource);
            } else {
                VK_Delete_Context_Add_Pipeline(DeleteContext, &PipelineEntry.Resource, PipelineEntry.LastUsedFrameIndex);
            }
        }
        VK_Delete_List_Clear(&DeleteContext->PipelineList[DeleteIndex]);

        ThreadContext = ThreadContext->Next;
    }
}

u32 GDI_Cmd_List_Get_Swapchain_Texture_Index(gdi_cmd_list* _CmdList, gdi_resource_state ResourceState) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    Assert(CmdList->SwapchainTextureIndex != (u32)-1);
    CmdList->ExecuteLockWaitStage = VK_Get_Pipeline_Stage_Flags(ResourceState);
    return CmdList->SwapchainTextureIndex;
}

void GDI_Cmd_List_Barrier(gdi_cmd_list* _CmdList, span<gdi_barrier> Barriers) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    gdi_context* Context = CmdList->Context;
    
    scratch Scratch = Scratch_Get();
    vk_pipeline_barrier PipelineBarriers[(u32)vk_pipeline_stage::Count][(u32)vk_pipeline_stage::Count] = {};
    array<texture_reader_lock> BarrierTextures(&Scratch);

    for(const gdi_barrier& Barrier : Barriers) {
        u32 OldStage = (u32)VK_Get_Pipeline_Stage(Barrier.OldState);
        u32 NewStage = (u32)VK_Get_Pipeline_Stage(Barrier.NewState);

        PipelineBarriers[OldStage][NewStage].InUse = true;

        if(Barrier.Resource.Type == gdi_resource_type::Texture) {
            if(!PipelineBarriers[OldStage][NewStage].ImageMemoryBarriers.Allocator) {
                PipelineBarriers[OldStage][NewStage].ImageMemoryBarriers = array<VkImageMemoryBarrier>(&Scratch);
            }

            async_handle<vk_texture> TextureHandle(Barrier.Resource.TextureHandle.ID);
            texture_reader_lock TextureLock(&Context->ResourceContext.Textures, TextureHandle);        
            if(TextureLock.Ptr) {
                Array_Push(&PipelineBarriers[OldStage][NewStage].ImageMemoryBarriers, {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .srcAccessMask = VK_Get_Access_Masks(Barrier.OldState),
                    .dstAccessMask = VK_Get_Access_Masks(Barrier.NewState),
                    .oldLayout     = VK_Get_Image_Layout(Barrier.OldState),
                    .newLayout     = VK_Get_Image_Layout(Barrier.NewState),
                    .srcQueueFamilyIndex = Context->PhysicalDevice->GraphicsQueueFamilyIndex,
                    .dstQueueFamilyIndex = Context->PhysicalDevice->GraphicsQueueFamilyIndex,
                    .image = TextureLock->Image,
                    .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
                });

                Array_Push(&BarrierTextures, TextureLock);
            }
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

    for(texture_reader_lock& Lock : BarrierTextures) {
        async_handle<vk_texture> Handle = Lock.Handle;
        Lock.Unlock();
        VK_Texture_Record_Frame(Context, Handle);
    }
}

void GDI_Cmd_List_Begin_Render_Pass(gdi_cmd_list* _CmdList, const gdi_render_pass_begin_info& BeginInfo) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    gdi_context* Context = CmdList->Context;

    async_handle<vk_framebuffer> FramebufferHandle(BeginInfo.Framebuffer.ID);
    pool_reader_lock FramebufferReader(&Context->ResourceContext.Framebuffers, FramebufferHandle);
    if(!FramebufferReader.Ptr) {
        return;
    }

    pool_scoped_lock FramebufferScoped(&FramebufferReader);

    async_handle<vk_render_pass> RenderPassHandle(BeginInfo.RenderPass.ID);
    pool_reader_lock RenderPassReader(&Context->ResourceContext.RenderPasses, RenderPassHandle);
    if(!RenderPassReader.Ptr) {
        return;
    }

    pool_scoped_lock RenderPassScoped(&RenderPassReader);

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
        .renderPass  = RenderPassReader->RenderPass,
        .framebuffer = FramebufferReader->Framebuffer,
        .renderArea  = {{}, {FramebufferReader->Width, FramebufferReader->Height}},
        .clearValueCount = Safe_U32(ClearValues.Count),
        .pClearValues = ClearValues.Ptr
    };

    vkCmdBeginRenderPass(CmdList->CmdBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkRect2D Scissor = {{}, {FramebufferReader->Width, FramebufferReader->Height}};
    VkViewport Viewport = {
        .width = (f32)FramebufferReader->Width,
        .height = (f32)FramebufferReader->Height,
        .maxDepth = 1.0f
    };

    vkCmdSetScissor(CmdList->CmdBuffer, 0, 1, &Scissor);
    vkCmdSetViewport(CmdList->CmdBuffer, 0, 1, &Viewport);

    RenderPassScoped.Unlock();
    FramebufferScoped.Unlock();

    VK_Render_Pass_Record_Frame(Context, RenderPassHandle);
    VK_Framebuffer_Record_Frame(Context, FramebufferHandle);
}

void GDI_Cmd_List_End_Render_Pass(gdi_cmd_list* _CmdList) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    vkCmdEndRenderPass(CmdList->CmdBuffer);
}

void GDI_Cmd_List_Set_Vtx_Buffers(gdi_cmd_list* _CmdList, span<gdi_handle<gdi_buffer>> VtxBuffers) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    gdi_context* Context = CmdList->Context;


    scratch Scratch = Scratch_Get();
    VkDeviceSize* Offsets = Scratch_Push_Array(&Scratch, VtxBuffers.Count, VkDeviceSize);
    VkBuffer* Buffers = Scratch_Push_Array(&Scratch, VtxBuffers.Count, VkBuffer);
    pool_reader_lock<vk_buffer>* BufferReaderLocks = Scratch_Push_Array(&Scratch, VtxBuffers.Count, pool_reader_lock<vk_buffer>);

    for(uptr i = 0; i < VtxBuffers.Count; i++) {
        async_handle<vk_buffer> BufferHandle(VtxBuffers[i].ID);
        Offsets[i] = 0;
        BufferReaderLocks[i] = pool_reader_lock(&Context->ResourceContext.Buffers, BufferHandle);
        if(BufferReaderLocks[i].Ptr)
            Buffers[i] = BufferReaderLocks[i]->Buffer;
    }

    vkCmdBindVertexBuffers(CmdList->CmdBuffer, 0, Safe_U32(VtxBuffers.Count), Buffers, Offsets);

    for(uptr i = 0; i < VtxBuffers.Count; i++) {
        if(BufferReaderLocks[i].Ptr) {
            async_handle<vk_buffer> Handle = BufferReaderLocks[i].Handle;
            BufferReaderLocks[i].Unlock();
            VK_Buffer_Record_Frame(Context, Handle);
        }
    }
}

void GDI_Cmd_List_Set_Idx_Buffer(gdi_cmd_list* _CmdList, gdi_handle<gdi_buffer> IdxBuffer, gdi_format Format) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    gdi_context* Context = CmdList->Context;

    async_handle<vk_buffer> BufferHandle(IdxBuffer.ID);
    pool_reader_lock BufferReader(&Context->ResourceContext.Buffers, BufferHandle);
    if(BufferReader.Ptr) {
        vkCmdBindIndexBuffer(CmdList->CmdBuffer, BufferReader->Buffer, 0, VK_Get_Index_Type(Format));
        BufferReader.Unlock();
        VK_Buffer_Record_Frame(Context, BufferHandle);
    }
}

void GDI_Cmd_List_Set_Pipeline(gdi_cmd_list* _CmdList, gdi_handle<gdi_pipeline> Pipeline) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    gdi_context* Context = CmdList->Context;

    async_handle<vk_pipeline> PipelineHandle(Pipeline.ID);
    pool_reader_lock PipelineReader(&Context->ResourceContext.Pipelines, PipelineHandle);
    if(PipelineReader.Ptr) {
        vkCmdBindPipeline(CmdList->CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineReader->Pipeline);
        PipelineReader.Unlock();
        VK_Pipeline_Record_Frame(Context, PipelineHandle);
    }
}

void GDI_Cmd_List_Draw_Indexed_Instance(gdi_cmd_list* _CmdList, u32 IdxCount, u32 IdxOffset, u32 VtxOffset, u32 InstanceCount, u32 InstanceOffset) {
    vk_cmd_list* CmdList = (vk_cmd_list*)_CmdList;
    vkCmdDrawIndexed(CmdList->CmdBuffer, IdxCount, InstanceCount, IdxOffset, (s32)VtxOffset, InstanceOffset);
}

#include "vk_pipeline.cpp"
#include "vk_swapchain.cpp"
#include "vk_texture.cpp"
#include "vk_buffer.cpp"
#include "vk_render_pass.cpp"
#include "vk_memory.cpp"
#include "vk_functions.cpp"

#include <gdi/gdi_shared.cpp>