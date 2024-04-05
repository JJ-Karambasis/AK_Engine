internal bool VK_Create_Buffer(gdi_context* Context, vk_buffer* Buffer, const gdi_buffer_create_info& CreateInfo) {
    VkDeviceSize TrueSize = CreateInfo.ByteSize;

    //By default all memory is device local. If we have a dynamic buffer
    //this will need to be host visible instead
    VkMemoryPropertyFlags MemoryFlag = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
    if(CreateInfo.UsageFlags & GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT) {
        MemoryFlag = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

        //Dynamic buffers need (frame_count)*(buffer_byte_size) so we can
        //use a portion of it each frame without having to wait for fences
        TrueSize *= Context->Frames.Count;
    }

    //If we want our memory host to be host visible and gpu local for fast
    //memory access by both cpu and gpu 
    //note: This can cause VK_Memory_Allocate to fail so we will fallback
    //to host visible only if this is the case
    if(CreateInfo.UsageFlags & GDI_BUFFER_USAGE_FLAG_GPU_LOCAL_BUFFER_BIT) {
        MemoryFlag |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    
    VkBufferCreateInfo BufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = TrueSize,
        .usage = VK_Convert_To_Buffer_Usage_Flags(CreateInfo.UsageFlags),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if(vkCreateBuffer(Context->Device, &BufferCreateInfo, Context->VKAllocator, &Buffer->Buffer) != VK_SUCCESS) {
        //todo: Diagnostics
        return false;
    }

    VkMemoryRequirements MemoryRequirements;
    vkGetBufferMemoryRequirements(Context->Device, Buffer->Buffer, &MemoryRequirements);

    bool MemoryFailed = false;
    if(!VK_Memory_Allocate(&Context->MemoryManager, &MemoryRequirements, MemoryFlag, &Buffer->Allocation)) {
        MemoryFlag &= ~(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if(MemoryFlag != 0) {
            if(!VK_Memory_Allocate(&Context->MemoryManager, &MemoryRequirements, MemoryFlag, &Buffer->Allocation)) {
                MemoryFailed = true;
            }
        } else MemoryFailed = true;
    }

    if(MemoryFailed) {
        //todo: Diagnostics
        return false;
    }

    VkDeviceMemory Memory = VK_Get_Memory_Block(&Buffer->Allocation.Allocate)->Memory;
    if(vkBindBufferMemory(Context->Device, Buffer->Buffer, Memory, Buffer->Allocation.Allocate.Offset) != VK_SUCCESS) {
        //todo: Diagnostics
        return false;
    }

    Buffer->UsageFlags = CreateInfo.UsageFlags;
    Buffer->Size = CreateInfo.ByteSize;
    if(CreateInfo.UsageFlags & GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT) {
        buffer Data;
        if(!VK_Memory_Map(Context->Device, &Buffer->Allocation, TrueSize, &Data)) {
            //todo: Diagnostics 
            return false;
        }
        Buffer->Ptr = Data.Ptr;

    }

    return true;
}

internal void VK_Delete_Buffer(gdi_context* Context, vk_buffer* Buffer) {
    if(Buffer->UsageFlags & GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT && Buffer->Ptr) {
        VK_Memory_Unmap(Context->Device, &Buffer->Allocation);
        Buffer->Ptr = NULL;
    }

    VK_Memory_Free(&Context->MemoryManager, &Buffer->Allocation);
    if(Buffer->Buffer) {
        vkDestroyBuffer(Context->Device, Buffer->Buffer, Context->VKAllocator);
        Buffer->Buffer = VK_NULL_HANDLE;
    }
}

internal void VK_Buffer_Record_Frame(gdi_context* Context, async_handle<vk_buffer> Handle) {
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.BuffersInUse[Handle.Index()], true);
}