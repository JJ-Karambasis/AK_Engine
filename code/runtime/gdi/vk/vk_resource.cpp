internal RESOURCE_ALLOCATE_CALLBACK_DEFINE(VK_Resource_Allocate_Internal) {
    vk_resource_manager* ResourceManager = (vk_resource_manager*)Manager;
    return ResourceManager->AllocateCallback(JobQueue, JobID, ResourceManager, ResourceID, ResourceData, UserData);
}

internal RESOURCE_ALLOCATE_CALLBACK_DEFINE(VK_Resource_Free_Internal) {
    vk_resource_manager* ResourceManager = (vk_resource_manager*)Manager;
    vk_resource* Resource = (vk_resource*)ResourceData;

    //Ready to delete the resource so we can now check on the last fence. 
    //If the last time the fence was used was more than the context frame 
    //count, then we have already waited for this fence
    if(Resource->LastUsedFence != VK_NULL_HANDLE) {
        u64 FrameDifference = ResourceManager->Context->CurrentFrameIndex-Resource->LastUsedFrameIndex;
        if(FrameDifference < ResourceManager->Context->Frames.Count) {
            if(vkGetFenceStatus(ResourceManager->Context->Device, Resource->LastUsedFence) == VK_NOT_READY) {
                return RESOURCE_RESULT_WAIT;
            }
        }
    }

    return ResourceManager->FreeCallback(JobQueue, JobID, ResourceManager, ResourceID, ResourceData, UserData);
}

internal RESOURCE_GET_CALLBACK_DEFINE(VK_Resource_Get_Internal) {
    u32 ResourceIndex = Resource_Get_Index(ResourceID);
    vk_resource_manager* ResourceManager = (vk_resource_manager*)Manager;
    vk_resource* Resource = (vk_resource*)ResourceData;
    vk_frame_context* FrameContext    = VK_Get_Current_Frame_Context(ResourceManager->Context);
    Resource->LastUsedFence      = FrameContext->Fence;
    Resource->LastUsedFrameIndex = ResourceManager->Context->CurrentFrameIndex;
}

void VK_Resource_Manager_Create(vk_resource_manager* ResourceManager, gdi_context* Context, const vk_resource_manager_create_info& CreateInfo) {
    Resource_Manager_Create(ResourceManager, Context->Arena, {
        .JobQueue = Context->ResourceQueue,
        .ResourceSize = CreateInfo.ResourceSize,
        .MaxCount = CreateInfo.MaxCount,
        .AllocateCallback = VK_Resource_Allocate_Internal,
        .FreeCallback = VK_Resource_Free_Internal,
        .GetCallback = VK_Resource_Get_Internal
    });
    ResourceManager->Context = Context;
    ResourceManager->AllocateCallback = CreateInfo.AllocateCallback;
    ResourceManager->FreeCallback = CreateInfo.FreeCallback;
}