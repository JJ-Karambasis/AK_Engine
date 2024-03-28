template <typename type>
inline void VK_Resource_Manager_Create(vk_resource_manager<type>* ResourceManager, gdi_context* Context, u32 Capacity) {
    ResourceManager->Context = Context;
    Async_Pool_Create(&ResourceManager->Pool, Context->Arena, Capacity);
    Async_Queue_Create(&ResourceManager->DeleteQueue, Context->Arena, Capacity);
}

template <typename type>
inline async_handle<type> VK_Resource_Manager_Allocate(vk_resource_manager<type>* ResourceManager) {
    return Async_Pool_Allocate(&ResourceManager->Pool);
}

template <typename type>
inline void VK_Resource_Manager_Free(vk_resource_manager<type>* ResourceManager, async_handle<type> Handle) {
    Async_Pool_Free(&ResourceManager->Pool, Handle);
}

template <typename type>
inline bool VK_Resource_Manager_Try_Delete_Queue(vk_resource_manager<type>* ResourceManager, const type* Resource) {
    //First check if resource needs to be queued
    bool ShouldQueueResource = VK_Resource_Should_Queue(ResourceManager->Context, Resource);
    if(ShouldQueueResource) {
        Async_Queue_Enqueue(&ResourceManager->DeleteQueue, *Resource);
    }
    return ShouldQueueResource;
}