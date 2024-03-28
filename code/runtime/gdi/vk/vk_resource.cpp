void VK_Resource_Record_Frame(gdi_context* Context, vk_resource* Resource);

bool VK_Resource_Should_Queue(gdi_context* Context, const vk_resource* Resource) {
    if(Resource->LastUsedFence) {
        uptr Difference = Context->TotalFramesRendered - Resource->LastUsedFrameIndex;
        if(Difference && (Difference < Context->Frames.Count)) {
            VkResult FenceStatus = vkGetFenceStatus(Context->Device, Resource->LastUsedFence);
            return FenceStatus == VK_NOT_READY;
        }
    }
    return false;
}