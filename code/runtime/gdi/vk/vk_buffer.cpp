internal bool VK_Create_Buffer(gdi_context* Context, vk_buffer* Buffer, const gdi_buffer_create_info& CreateInfo) {
    VkBufferCreateInfo BufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = CreateInfo.ByteSize,
        .usage = VK_Convert_To_Buffer_Usage_Flags(CreateInfo.UsageFlags),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if(vkCreateBuffer(Context->Device, &BufferCreateInfo, Context->VKAllocator, &Buffer->Buffer) != VK_SUCCESS) {
        //todo: Diagnostics
        return false;
    }

    VkMemoryRequirements MemoryRequirements;
    vkGetBufferMemoryRequirements(Context->Device, Buffer->Buffer, &MemoryRequirements);

    if(!VK_Memory_Allocate(&Context->MemoryManager, &MemoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &Buffer->Allocation)) {
        //todo: Diagnostics
        return false;
    }

    VkDeviceMemory Memory = VK_Get_Memory_Block(&Buffer->Allocation.Allocate)->Memory;
    if(vkBindBufferMemory(Context->Device, Buffer->Buffer, Memory, Buffer->Allocation.Allocate.Offset) != VK_SUCCESS) {
        //todo: Diagnostics
        return false;
    }

    return true;
}

internal void VK_Delete_Buffer(gdi_context* Context, vk_buffer* Buffer) {
    VK_Memory_Free(&Context->MemoryManager, &Buffer->Allocation);
    if(Buffer->Buffer) {
        vkDestroyBuffer(Context->Device, Buffer->Buffer, Context->VKAllocator);
        Buffer->Buffer = VK_NULL_HANDLE;
    }
}

internal bool VK_Buffer_Upload(gdi_context* Context, async_handle<vk_buffer> Handle, const_buffer Data) {
    vk_thread_context* ThreadContext = VK_Get_Thread_Context(Context);
    vk_upload_buffer* UploadBuffer = VK_Get_Current_Upload_Buffer(Context, ThreadContext);
    
    vk_upload Upload;
    if(!VK_Upload_Buffer_Push(UploadBuffer, Data, &Upload)) {
        //todo: Diagnostics
        return false;
    }

    VK_Copy_Context_Add_Upload_To_Buffer_Copy(&ThreadContext->CopyContext, {
        .Upload = Upload,
        .Buffer = Handle
    });

    return true;
}

internal void VK_Buffer_Record_Frame(gdi_context* Context, async_handle<vk_buffer> Handle) {

}