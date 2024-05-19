template <typename type>
inline internal void VK_Delete_List_Init(vk_delete_list<type>* List, allocator* Allocator) {
    Array_Init(&List->List, Allocator, 128);
}

template <typename type>
inline internal void VK_Delete_List_Clear(vk_delete_list<type>* List) {
    Array_Clear(&List->List);
}

template <typename type>
inline internal void VK_Delete_List_Free(vk_delete_list<type>* List) {
    Array_Free(&List->List);
}

internal vk_upload_buffer_block* VK_Upload_Buffer_Get_Current_Block(vk_upload_buffer* UploadBuffer, uptr Size, uptr Alignment) {
    vk_upload_buffer_block* Result = UploadBuffer->Current;
    while(Result && (Result->Data.Size < (Align_Pow2(Result->Used, Alignment)+Size))) {
        Result = Result->Next;
    }
    return Result;
}

internal vk_upload_buffer_block* VK_Upload_Buffer_Create_Block(vk_upload_buffer* UploadBuffer, size_t AllocationSize) {
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

internal void VK_Upload_Buffer_Delete_Block(vk_upload_buffer* UploadBuffer, vk_upload_buffer_block* Block) {
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

internal void VK_Upload_Buffer_Clear(vk_upload_buffer* UploadBuffer) {
    for(vk_upload_buffer_block* Block = UploadBuffer->First; Block; Block = Block->Next) {
        Block->Used = 0;
    }
    UploadBuffer->Current = UploadBuffer->First;
}

internal void VK_Upload_Buffer_Create(gdi_context* Context, vk_upload_buffer* UploadBuffer) {
    UploadBuffer->Arena = Arena_Create(Context->GDI->MainAllocator);
    UploadBuffer->Device = Context->Device;
    UploadBuffer->VKAllocator = Context->VKAllocator;
    UploadBuffer->MemoryManager = &Context->MemoryManager;
}

internal void VK_Upload_Buffer_Delete(vk_upload_buffer* UploadBuffer) {
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

inline internal u32 VK_Copy_Context_Swap(vk_copy_context* CopyContext) {
    AK_RW_Lock_Writer(&CopyContext->RWLock);
    u32 Result = CopyContext->CurrentListIndex;
    CopyContext->CurrentListIndex = !CopyContext->CurrentListIndex;
    Arena_Clear(CopyContext->Arenas[CopyContext->CurrentListIndex]);
    AK_RW_Unlock_Writer(&CopyContext->RWLock);
    return Result;
}

inline internal u32 VK_Delete_Context_Swap(vk_delete_context* DeleteContext) {
    AK_RW_Lock_Writer(&DeleteContext->RWLock);
    u32 Result = DeleteContext->CurrentListIndex;
    DeleteContext->CurrentListIndex = !DeleteContext->CurrentListIndex;
    AK_RW_Unlock_Writer(&DeleteContext->RWLock);
    return Result;
}

template <typename type>
internal void VK_Delete_List_Entries(gdi_context* Context, vk_delete_context* DeleteContext, vk_delete_list<type>* DeleteLists, u32 DeleteIndex) {
    for(vk_delete_list_entry<type>& Entry : DeleteLists[DeleteIndex]) {
        uptr Difference = Context->TotalFramesRendered - Entry.LastUsedFrameIndex;
        if(Difference >= Context->Frames.Count) {
            VK_Delete_Resource(Context, &Entry.Resource);
        } else {
            VK_Delete_Context_Add(DeleteContext, DeleteLists, &Entry.Resource, Entry.LastUsedFrameIndex);
        }
    }
    VK_Delete_List_Clear(&DeleteLists[DeleteIndex]);
}

template <typename type>
internal void VK_Delete_List(gdi_context* Context, vk_delete_list<type>& DeleteList) {
    for(vk_delete_list_entry<type>& Entry : DeleteList) {
        VK_Delete_Resource(Context, &Entry.Resource);
    }
    VK_Delete_List_Free(&DeleteList);
}

u8* VK_Upload_Buffer_Push(vk_upload_buffer* UploadBuffer, size_t Size, vk_upload* Upload) {
    const uptr Alignment = DEFAULT_ALIGNMENT;

    vk_upload_buffer_block* Block = VK_Upload_Buffer_Get_Current_Block(UploadBuffer, Size, Alignment);
    if(!Block) {
        uptr BlockSize = VK_UPLOAD_BUFFER_MINIMUM_BLOCK_SIZE;

        uptr Mask = Alignment-1;
        if(BlockSize < (Size+Mask)) {
            BlockSize = Ceil_Pow2(Size+Mask);
        }

        Block = VK_Upload_Buffer_Create_Block(UploadBuffer, BlockSize);
        if(!Block) {
            return NULL;
        }

        SLL_Push_Back(UploadBuffer->First, UploadBuffer->Last, Block);
    }

    UploadBuffer->Current = Block;
    vk_upload_buffer_block* CurrentBlock = UploadBuffer->Current;

    CurrentBlock->Used = Align_Pow2(CurrentBlock->Used, Alignment);
    Assert(CurrentBlock->Used+Size <= CurrentBlock->Data.Size);

    Upload->Buffer = CurrentBlock->Buffer;
    Upload->Offset = CurrentBlock->Used;
    Upload->Size   = Size;

    u8* Result = CurrentBlock->Data.Ptr+CurrentBlock->Used;
    CurrentBlock->Used += Size;

    return Result; 
}

void VK_Cmd_Storage_Free_All(vk_cmd_storage_list* StorageList) {
    while(StorageList->Tail) {
        vk_cmd_list* CmdList = StorageList->Tail;
        DLL_Remove_Back(StorageList->Head, StorageList->Tail);
        SLL_Push_Front(StorageList->Free, CmdList);
    }
}

vk_cmd_pool* VK_Get_Current_Cmd_Pool(vk_thread_context_manager* Manager, vk_thread_context* ThreadContext) {
    gdi_context* Context = Manager->Context;
    u64 FrameIndex = Context->TotalFramesRendered % Context->Frames.Count;
    return &ThreadContext->CmdPools[FrameIndex];  
}

vk_upload_buffer* VK_Get_Current_Upload_Buffer(vk_thread_context_manager* Manager, vk_thread_context* ThreadContext) {
    gdi_context* Context = Manager->Context;
    u64 FrameIndex = Context->TotalFramesRendered % Context->Frames.Count;
    return &ThreadContext->UploadBuffers[FrameIndex];
}

void VK_Copy_Context_Add_Upload_To_Buffer_Copy(vk_copy_context* CopyContext, const vk_copy_upload_to_buffer& CopyUploadToBuffer) {
    AK_RW_Lock_Reader(&CopyContext->RWLock);
    u32 ListIndex = CopyContext->CurrentListIndex;
    Array_Push(&CopyContext->CopyUploadToBufferList[ListIndex], CopyUploadToBuffer);
    AK_RW_Unlock_Reader(&CopyContext->RWLock);
}

void VK_Copy_Context_Add_Uploads_To_Texture_Copy(vk_copy_context* CopyContext, vk_handle<vk_texture> Texture, vk_upload Upload, span<uptr> Offsets, span<vk_region> Regions) {
    AK_RW_Lock_Reader(&CopyContext->RWLock);
    u32 ListIndex = CopyContext->CurrentListIndex;
    Array_Push(&CopyContext->CopyUploadsToTextureList[ListIndex], {
        .Upload  = Upload,
        .Offsets = fixed_array<uptr>(CopyContext->Arenas[ListIndex], Offsets),
        .Regions = fixed_array<vk_region>(CopyContext->Arenas[ListIndex], Regions),
        .Texture = Texture
    });
    AK_RW_Unlock_Reader(&CopyContext->RWLock);
}

vk_copy_context* VK_Get_Copy_Context(vk_thread_context_manager* Manager) {
    return &VK_Get_Thread_Context(Manager)->CopyContext;
}

vk_delete_context* VK_Get_Delete_Context(vk_thread_context_manager* Manager) {
    return &VK_Get_Thread_Context(Manager)->DeleteContext;
}

vk_thread_context* VK_Get_Thread_Context(vk_thread_context_manager* Manager) {
    gdi_context* Context = Manager->Context;
    vk_thread_context* Result = (vk_thread_context*)AK_TLS_Get(&Manager->TLS);
    if(!Result) {
        //One allocation for the entire delete context
        arena* Arena = Arena_Create(Context->GDI->MainAllocator);
        Result = Arena_Push_Struct(Arena, vk_thread_context);
        Result->Arena = Arena;
        SLL_Push_Front_Async(&Manager->List, Result);
        
        Result->UploadBuffers = fixed_array<vk_upload_buffer>(Result->Arena, Context->Frames.Count);
        Result->CmdPools = fixed_array<vk_cmd_pool>(Result->Arena, Context->Frames.Count);
        for(u32 i = 0; i < Context->Frames.Count; i++) {
            vk_cmd_pool* CmdPool = &Result->CmdPools[i];

            VkCommandPoolCreateInfo CreateInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .queueFamilyIndex = Context->PhysicalDevice->GraphicsQueueFamilyIndex
            };

            if(vkCreateCommandPool(Context->Device, &CreateInfo, Context->VKAllocator, &CmdPool->CommandPool) != VK_SUCCESS) {
                //todo: Logging
                Arena_Delete(Arena);
                return nullptr;
            }

            if(i == 0) {
                vkResetCommandPool(Context->Device, CmdPool->CommandPool, 0);
            }
        }

        vk_delete_context* DeleteContext = &Result->DeleteContext;
        AK_RW_Lock_Create(&DeleteContext->RWLock);
        for(u32 i = 0; i < 2; i++) {
            VK_Delete_List_Init(&DeleteContext->PipelineList[i], Context->GDI->MainAllocator);
            VK_Delete_List_Init(&DeleteContext->BindGroupList[i], Context->GDI->MainAllocator);
            VK_Delete_List_Init(&DeleteContext->BindGroupLayoutList[i], Context->GDI->MainAllocator);
            VK_Delete_List_Init(&DeleteContext->FramebufferList[i], Context->GDI->MainAllocator);
            VK_Delete_List_Init(&DeleteContext->RenderPassList[i], Context->GDI->MainAllocator);
            VK_Delete_List_Init(&DeleteContext->SamplerList[i], Context->GDI->MainAllocator);
            VK_Delete_List_Init(&DeleteContext->TextureViewList[i], Context->GDI->MainAllocator);
            VK_Delete_List_Init(&DeleteContext->TextureList[i], Context->GDI->MainAllocator);
            VK_Delete_List_Init(&DeleteContext->BufferList[i], Context->GDI->MainAllocator);
            VK_Delete_List_Init(&DeleteContext->SwapchainList[i], Context->GDI->MainAllocator);
        }

        vk_copy_context* CopyContext = &Result->CopyContext;
        AK_RW_Lock_Create(&CopyContext->RWLock);
        for(u32 i = 0; i < 2; i++) {
            CopyContext->Arenas[i] = Arena_Create(Context->GDI->MainAllocator);
            Array_Init(&CopyContext->CopyUploadToBufferList[i], Context->GDI->MainAllocator);
            Array_Init(&CopyContext->CopyUploadsToTextureList[i], Context->GDI->MainAllocator);
        }

        for(vk_upload_buffer& UploadBuffer : Result->UploadBuffers) {
            VK_Upload_Buffer_Create(Context, &UploadBuffer);
        }

        AK_TLS_Set(&Manager->TLS, Result);
    }
    return Result;
}

void VK_Thread_Context_Manager_Create(gdi_context* Context, vk_thread_context_manager* Manager) {
    Manager->Context = Context;
    AK_Mutex_Create(&Manager->Lock);
    Manager->Arena = Arena_Create(Context->GDI->MainAllocator);
    AK_TLS_Create(&Manager->TLS);
}

void VK_Thread_Context_Manager_Delete(vk_thread_context_manager* Manager) {
    gdi_context* Context = Manager->Context;
    vk_thread_context* ThreadContext = (vk_thread_context*)AK_Atomic_Load_Ptr_Relaxed(&Manager->List);
    while(ThreadContext) {

        vk_delete_context* DeleteContext = &ThreadContext->DeleteContext;
        for(u32 i = 0; i < 2; i++) {
            VK_Delete_List(Context, DeleteContext->PipelineList[i]);
            VK_Delete_List(Context, DeleteContext->BindGroupList[i]);
            VK_Delete_List(Context, DeleteContext->BindGroupLayoutList[i]);
            VK_Delete_List(Context, DeleteContext->FramebufferList[i]);
            VK_Delete_List(Context, DeleteContext->RenderPassList[i]);
            VK_Delete_List(Context, DeleteContext->SamplerList[i]);
            VK_Delete_List(Context, DeleteContext->TextureViewList[i]);
            VK_Delete_List(Context, DeleteContext->TextureList[i]);
            VK_Delete_List(Context, DeleteContext->BufferList[i]);
            VK_Delete_List(Context, DeleteContext->SwapchainList[i]);
        }

        vk_copy_context* CopyContext = &ThreadContext->CopyContext;
        for(uptr i = 0; i < 2; i++) {
            Array_Free(&CopyContext->CopyUploadToBufferList[i]);
            Array_Free(&CopyContext->CopyUploadsToTextureList[i]);
            Arena_Delete(CopyContext->Arenas[i]);
        }

        for(vk_upload_buffer& UploadBuffer : ThreadContext->UploadBuffers) {
            VK_Upload_Buffer_Delete(&UploadBuffer);
            Zero_Struct(&UploadBuffer);
        }

        for(vk_cmd_pool& CmdPool : ThreadContext->CmdPools) {
            vkDestroyCommandPool(Context->Device, CmdPool.CommandPool, Context->VKAllocator);
            Zero_Struct(&CmdPool);            
        }

        Arena_Delete(ThreadContext->Arena);
        ThreadContext = ThreadContext->Next;
    }

    AK_Mutex_Delete(&Manager->Lock);
    AK_TLS_Delete(&Manager->TLS);
    Arena_Delete(Manager->Arena);
}

void VK_Thread_Context_Manager_Copy_Data(vk_thread_context_manager* Manager) {
    gdi_context* Context = Manager->Context;
    vk_resource_context* ResourceContext = &Context->ResourceContext;

    vk_frame_context* FrameContext = VK_Get_Current_Frame_Context(Context);

    vk_thread_context* ThreadContext = (vk_thread_context*)AK_Atomic_Load_Ptr_Relaxed(&Manager->List);
    while(ThreadContext) {
        vk_copy_context* CopyContext = &ThreadContext->CopyContext;
        u32 CopyIndex = VK_Copy_Context_Swap(CopyContext);
        
        for(const vk_copy_upload_to_buffer& CopyUploadToBuffer : CopyContext->CopyUploadToBufferList[CopyIndex]) {
            vk_buffer* Buffer = VK_Resource_Get(ResourceContext->Buffers, CopyUploadToBuffer.Buffer);
            if(Buffer) {
                VkBufferCopy BufferCopy = {
                    .srcOffset = CopyUploadToBuffer.Upload.Offset,
                    .dstOffset = CopyUploadToBuffer.Offset,
                    .size = CopyUploadToBuffer.Upload.Size
                };
                vkCmdCopyBuffer(FrameContext->CopyCmdBuffer, CopyUploadToBuffer.Upload.Buffer, Buffer->Handle, 1, &BufferCopy);
                VK_Resource_Record_Frame(Buffer);
            }
        }

        scratch Scratch = Scratch_Get();

        Array_Clear(&CopyContext->CopyUploadToBufferList[CopyIndex]);

        u32 TextureCapacity = ResourceContext->Textures.FreeIndices.Capacity;
        
        uptr MaxImageCount = 0;
        uptr InitialImageCount = 0;
        uptr UpdateImageCount = 0;
        bool* TexturesInUse = Scratch_Push_Array(&Scratch, TextureCapacity, bool);
        for(const vk_copy_uploads_to_texture& CopyUploadsToTexture : CopyContext->CopyUploadsToTextureList[CopyIndex]) {
            vk_texture* Texture = VK_Resource_Get(ResourceContext->Textures, CopyUploadsToTexture.Texture);
            if(Texture) {
                u32 TextureIndex = CopyUploadsToTexture.Texture.Index;
                if(!TexturesInUse[TextureIndex]) {
                    if(Texture->JustAllocated) InitialImageCount++;
                    else UpdateImageCount++;

                    TexturesInUse[TextureIndex] = true;
                    MaxImageCount++;
                }
            }
        }

        array<VkImageMemoryBarrier> ImageMemoryBarriers(&Scratch, MaxImageCount);
        array<VkImageMemoryBarrier> InitialMemoryBarriers(&Scratch, InitialImageCount);
        array<VkImageMemoryBarrier> UpdateMemoryBarriers(&Scratch, UpdateImageCount);

        for(uptr i = 0; i < TextureCapacity; i++) {
            if(TexturesInUse[i]) {
                vk_texture* Texture = ResourceContext->Textures.Resources + i;
                if(Texture->JustAllocated) {
                    Array_Push(&InitialMemoryBarriers, {
                        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                        .srcAccessMask = 0,
                        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        .image = Texture->Handle,
                        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
                    });
                    Texture->JustAllocated = false;
                } else {
                    Array_Push(&UpdateMemoryBarriers, {
                        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                        .srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
                        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                        .oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        .image = Texture->Handle,
                        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
                    });
                }
            }
        }

        if(InitialMemoryBarriers.Count) {
            vkCmdPipelineBarrier(FrameContext->CopyCmdBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
                                VK_DEPENDENCY_BY_REGION_BIT, 0, NULL, 0, NULL, Safe_U32(InitialMemoryBarriers.Count), InitialMemoryBarriers.Ptr);
        }

        if(UpdateMemoryBarriers.Count) {
            vkCmdPipelineBarrier(FrameContext->CopyCmdBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
                                VK_DEPENDENCY_BY_REGION_BIT, 0, NULL, 0, NULL, Safe_U32(UpdateMemoryBarriers.Count), UpdateMemoryBarriers.Ptr);
        }

        for(const vk_copy_uploads_to_texture& CopyUploadsToTexture : CopyContext->CopyUploadsToTextureList[CopyIndex]) {
            vk_texture* Texture = VK_Resource_Get(ResourceContext->Textures, CopyUploadsToTexture.Texture);
            if(Texture) {
                fixed_array<VkBufferImageCopy> Regions(&Scratch, CopyUploadsToTexture.Regions.Count);
                for(uptr i = 0; i < Regions.Count; i++) {
                    Regions[i] = {
                        .bufferOffset = CopyUploadsToTexture.Upload.Offset + CopyUploadsToTexture.Offsets[i],
                        .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                        .imageOffset = {CopyUploadsToTexture.Regions[i].Offset.x, CopyUploadsToTexture.Regions[i].Offset.y, 0},
                        .imageExtent = {(u32)CopyUploadsToTexture.Regions[i].Size.width, (u32)CopyUploadsToTexture.Regions[i].Size.height, 1}
                    };
                }

                vkCmdCopyBufferToImage(FrameContext->CopyCmdBuffer, CopyUploadsToTexture.Upload.Buffer, Texture->Handle, 
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Safe_U32(Regions.Count), Regions.Ptr);
                VK_Resource_Record_Frame(Texture);
            }
        }

        Array_Clear(&ImageMemoryBarriers);
        for(uptr i = 0; i < TextureCapacity; i++) {
            if(TexturesInUse[i]) {
                vk_texture* Texture = ResourceContext->Textures.Resources + i;
                Array_Push(&ImageMemoryBarriers, {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    .image = Texture->Handle,
                    .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
                });
            }
        }

        vkCmdPipelineBarrier(FrameContext->CopyCmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
                             VK_DEPENDENCY_BY_REGION_BIT, 0, NULL, 0, NULL, Safe_U32(ImageMemoryBarriers.Count), ImageMemoryBarriers.Ptr);


        Array_Clear(&CopyContext->CopyUploadsToTextureList[CopyIndex]);
        ThreadContext = ThreadContext->Next;
    }
}

void VK_Thread_Context_Manager_New_Frame(vk_thread_context_manager* Manager) {
    gdi_context* Context = Manager->Context;
    vk_thread_context* ThreadContext = (vk_thread_context*)AK_Atomic_Load_Ptr_Relaxed(&Manager->List);
    while(ThreadContext) {
        vk_upload_buffer* UploadBuffer = VK_Get_Current_Upload_Buffer(Manager, ThreadContext);
        VK_Upload_Buffer_Clear(UploadBuffer);

        vk_cmd_pool* CmdPool = VK_Get_Current_Cmd_Pool(Manager, ThreadContext);
        vkResetCommandPool(Context->Device, CmdPool->CommandPool, 0);

        VK_Cmd_Storage_Free_All(&CmdPool->PrimaryCmds);
        VK_Cmd_Storage_Free_All(&CmdPool->SecondaryCmds);

        vk_delete_context* DeleteContext = &ThreadContext->DeleteContext;
        u32 DeleteIndex = VK_Delete_Context_Swap(DeleteContext);

        VK_Delete_List_Entries(Context, DeleteContext, DeleteContext->PipelineList, DeleteIndex);
        VK_Delete_List_Entries(Context, DeleteContext, DeleteContext->BindGroupList, DeleteIndex);
        VK_Delete_List_Entries(Context, DeleteContext, DeleteContext->BindGroupLayoutList, DeleteIndex);
        VK_Delete_List_Entries(Context, DeleteContext, DeleteContext->FramebufferList, DeleteIndex);
        VK_Delete_List_Entries(Context, DeleteContext, DeleteContext->RenderPassList, DeleteIndex);
        VK_Delete_List_Entries(Context, DeleteContext, DeleteContext->SamplerList, DeleteIndex);
        VK_Delete_List_Entries(Context, DeleteContext, DeleteContext->TextureViewList, DeleteIndex);
        VK_Delete_List_Entries(Context, DeleteContext, DeleteContext->TextureList, DeleteIndex);
        VK_Delete_List_Entries(Context, DeleteContext, DeleteContext->BufferList, DeleteIndex);
        VK_Delete_List_Entries(Context, DeleteContext, DeleteContext->SwapchainList, DeleteIndex);

        ThreadContext = ThreadContext->Next;
    }
}