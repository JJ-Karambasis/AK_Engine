internal GDI_HEAP_ALLOCATE_MEMORY_BLOCK_DEFINE(VK_Heap_Allocate_Memory_Block) {
    vk_heap* VKHeap = (vk_heap*)Heap;

    VkMemoryAllocateInfo MemoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = Size,
        .memoryTypeIndex = VKHeap->MemoryTypeIndex
    };

    gdi_context* Context = VKHeap->MemoryManager->Context; 
    
    VkDeviceMemory Memory;
    if(vkAllocateMemory(Context->Device, &MemoryAllocateInfo, Context->VKAllocator, &Memory) != VK_SUCCESS) {
        //todo: Logging
        return NULL;
    }

    vk_heap_memory_block* MemoryBlock = Arena_Push_Struct(Heap->Arena, vk_heap_memory_block);
    MemoryBlock->Memory = Memory;
    AK_Mutex_Create(&MemoryBlock->MapLock);
    return MemoryBlock;
}

internal GDI_HEAP_FREE_MEMORY_BLOCK_DEFINE(VK_Heap_Free_Memory_Block) {
    vk_heap* VKHeap = (vk_heap*)Heap;
    vk_heap_memory_block* Block = (vk_heap_memory_block*)MemoryBlock;
    if(Block->Memory) {
        gdi_context* Context = VKHeap->MemoryManager->Context;
        AK_Mutex_Delete(&Block->MapLock);
        vkFreeMemory(Context->Device, Block->Memory, Context->VKAllocator);
        Block->Memory = VK_NULL_HANDLE;
    }
}

global gdi_heap_vtable G_VKHeapVTable = {
    VK_Heap_Allocate_Memory_Block,
    VK_Heap_Free_Memory_Block
};

internal s32 VK_Memory_Find_Properties(vk_memory_manager* MemoryManager, u32 MemoryTypeRequirements, VkMemoryPropertyFlags MemoryFlags) {
    VkPhysicalDeviceMemoryProperties* MemoryProperties = &MemoryManager->Context->PhysicalDevice->MemoryProperties;
    for(u32 MemoryIndex = 0; MemoryIndex < MemoryProperties->memoryTypeCount; MemoryIndex++) {
        u32 MemoryTypeBits = (u32)(1 << MemoryIndex);
        if(MemoryTypeBits & MemoryTypeRequirements) {
            VkMemoryPropertyFlags Flags = MemoryProperties->memoryTypes[MemoryIndex].propertyFlags;
            if((Flags & MemoryFlags) == MemoryFlags) {
                return (s32)MemoryIndex;
            } 
        }
    }
    return -1;
}

internal void VK_Memory_Manager_Create(vk_memory_manager* Manager, gdi_context* Context) {
    Manager->Context = Context;
    VkPhysicalDeviceMemoryProperties* MemoryProperties = &Manager->Context->PhysicalDevice->MemoryProperties;
    for(u32 MemoryIndex = 0; MemoryIndex < MemoryProperties->memoryTypeCount; MemoryIndex++) {
        GDI_Heap_Create(&Manager->Heaps[MemoryIndex], Context->GDI->MainAllocator, MB(32), &G_VKHeapVTable);
        Manager->Heaps[MemoryIndex].MemoryTypeIndex = MemoryIndex;
        Manager->Heaps[MemoryIndex].MemoryManager = Manager;
    }
}

internal void VK_Memory_Manager_Delete(vk_memory_manager* Manager) {
    VkPhysicalDeviceMemoryProperties* MemoryProperties = &Manager->Context->PhysicalDevice->MemoryProperties;
    for(u32 MemoryIndex = 0; MemoryIndex < MemoryProperties->memoryTypeCount; MemoryIndex++) {
        GDI_Heap_Delete(&Manager->Heaps[MemoryIndex]);
    }
    Manager->Context = NULL;
}

internal vk_heap_memory_block* VK_Get_Memory_Block(gdi_allocate* Allocate) {
    return (vk_heap_memory_block*)Allocate->Block->Block;
}

internal bool VK_Memory_Allocate(vk_memory_manager* MemoryManager, const VkMemoryRequirements* MemoryRequirements, VkMemoryPropertyFlags MemoryFlags, vk_allocation* Allocation) {
    s32 MemoryIndex = VK_Memory_Find_Properties(MemoryManager, MemoryRequirements->memoryTypeBits, MemoryFlags);
    if(MemoryIndex == -1) {
        return false;
    }

    vk_heap* Heap = MemoryManager->Heaps + MemoryIndex;
    gdi_allocate Allocate = GDI_Heap_Allocate(Heap, MemoryRequirements->size, MemoryRequirements->alignment);
    if(!Allocate.Block) {
        return false;
    }

    Allocation->Heap     = Heap;
    Allocation->Allocate = Allocate;
    return true;
}

internal void VK_Memory_Free(vk_memory_manager* MemoryManager, vk_allocation* Allocation) {
    if(Allocation->Heap && Allocation->Allocate.Block) {
        GDI_Heap_Free(Allocation->Heap, &Allocation->Allocate);
        Allocation->Heap  = NULL;
    }
}

internal bool VK_Memory_Map(VkDevice Device, vk_allocation* Allocation, VkDeviceSize Size, buffer* Buffer) {
    vk_heap_memory_block* MemoryBlock = VK_Get_Memory_Block(&Allocation->Allocate);
    scoped_mutex Mutex(&MemoryBlock->MapLock);
    if(MemoryBlock->MappedReferenceCount == 0) {
        if(vkMapMemory(Device, MemoryBlock->Memory, 0, VK_WHOLE_SIZE, 0, (void**)&MemoryBlock->MappedMemory) != VK_SUCCESS) {
            //todo: logging
            return false;
        }
    }
    MemoryBlock->MappedReferenceCount++;
    *Buffer = buffer(MemoryBlock->MappedMemory+Allocation->Allocate.Offset, Size);
    return true;
}

internal void VK_Memory_Unmap(VkDevice Device, vk_allocation* Allocation) {
    vk_heap_memory_block* MemoryBlock = VK_Get_Memory_Block(&Allocation->Allocate);
    scoped_mutex Mutex(&MemoryBlock->MapLock);
    --MemoryBlock->MappedReferenceCount;
    if(MemoryBlock->MappedReferenceCount == 0) {
        vkUnmapMemory(Device, MemoryBlock->Memory);
        MemoryBlock->MappedMemory = NULL;
    }
}