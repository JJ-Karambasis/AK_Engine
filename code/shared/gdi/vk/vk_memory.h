#ifndef VK_MEMORY_H
#define VK_MEMORY_H

struct vk_memory_manager;

struct vk_heap_memory_block : public gdi_heap_memory_block {
    VkDeviceMemory Memory;
    ak_mutex       MapLock;
    u32            MappedReferenceCount;
    u8*            MappedMemory;
};

struct vk_heap : public gdi_heap {
    vk_memory_manager* MemoryManager;
    u32                MemoryTypeIndex;
};

struct vk_memory_manager {
    gdi_context* Context;
    vk_heap      Heaps[VK_MAX_MEMORY_TYPES];
};

struct vk_allocation {
    vk_heap*     Heap;
    gdi_allocate Allocate;
};

internal void VK_Memory_Manager_Create(vk_memory_manager* Manager, gdi_context* Context);
internal void VK_Memory_Manager_Delete(vk_memory_manager* Manager);
internal vk_heap_memory_block* VK_Get_Memory_Block(gdi_allocate* Allocate);
internal bool VK_Memory_Allocate(vk_memory_manager* MemoryManager, const VkMemoryRequirements* MemoryRequirements, VkMemoryPropertyFlags MemoryFlags, vk_allocation* Allocation);
internal void VK_Memory_Free(vk_memory_manager* MemoryManager, vk_allocation* Allocation);
internal bool VK_Memory_Map(VkDevice Device, vk_allocation* Allocation, VkDeviceSize Size, buffer* Buffer);
internal void VK_Memory_Unmap(VkDevice Device, vk_allocation* Allocation);

#endif