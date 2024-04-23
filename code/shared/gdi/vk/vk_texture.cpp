
internal bool VK_Create_Sampler(gdi_context* Context, vk_sampler* Sampler, const gdi_sampler_create_info& CreateInfo) {
     VkSamplerCreateInfo SamplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_Get_Filter(CreateInfo.Filter),
        .minFilter = VK_Get_Filter(CreateInfo.Filter),
        .addressModeU = VK_Get_Address_Mode(CreateInfo.AddressModeU),
        .addressModeV = VK_Get_Address_Mode(CreateInfo.AddressModeV)
     };

     if(vkCreateSampler(Context->Device, &SamplerInfo, Context->VKAllocator, &Sampler->Sampler) != VK_SUCCESS) {
        //todo: Logging
        return false;
     }
     return true;
}

internal void VK_Delete_Sampler(gdi_context* Context, vk_sampler* Sampler) {
    if(Sampler->Sampler) {
        vkDestroySampler(Context->Device, Sampler->Sampler, Context->VKAllocator);
        Sampler->Sampler = VK_NULL_HANDLE;
    }
}

internal void VK_Sampler_Record_Frame(gdi_context* Context, async_handle<vk_sampler> Handle) {
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.SamplersInUse[Handle.Index()], true);
}

internal bool VK_Create_Texture_View(gdi_context* Context, vk_texture_view* TextureView, const gdi_texture_view_create_info& CreateInfo) {

    async_handle<vk_texture> TextureHandle(CreateInfo.Texture.ID);
    vk_texture* Texture = Async_Pool_Get(&Context->ResourceContext.Textures, TextureHandle);
    if(!Texture) {
        return false;
    }

    gdi_format Format = CreateInfo.Format;
    if(Format == GDI_FORMAT_NONE) {
        Format = Texture->Format;
    }

    VkImageAspectFlags ImageAspect = GDI_Is_Depth_Format(Format) ? 
                                     VK_IMAGE_ASPECT_DEPTH_BIT : 
                                     VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageViewCreateInfo ImageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = Texture->Image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_Get_Format(Format),
        .subresourceRange = {ImageAspect, 0, 1, 0, 1}
    };

    if(vkCreateImageView(Context->Device, &ImageViewCreateInfo, Context->VKAllocator, &TextureView->ImageView) != VK_SUCCESS) {
        //todo: diagnostic
        return false;
    }

    TextureView->TextureHandle = TextureHandle;

    return true;
}

internal void VK_Delete_Texture_View(gdi_context* Context, vk_texture_view* TextureView) {
    if(TextureView->ImageView) {
        vkDestroyImageView(Context->Device, TextureView->ImageView, Context->VKAllocator);
        TextureView->ImageView = VK_NULL_HANDLE;
    }
}

internal void VK_Texture_View_Record_Frame(gdi_context* Context, async_handle<vk_texture_view> Handle) {
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.TextureViewsInUse[Handle.Index()], true);
}

internal bool VK_Create_Texture(gdi_context* Context, vk_texture* Texture, const gdi_texture_create_info& CreateInfo) {
    VkImageCreateInfo ImageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_Get_Format(CreateInfo.Format),
        .extent = {CreateInfo.Width, CreateInfo.Height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_Convert_To_Image_Usage_Flags(CreateInfo.UsageFlags),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    if(vkCreateImage(Context->Device, &ImageInfo, Context->VKAllocator, &Texture->Image) != VK_SUCCESS) {
        //todo: Logging
        return false;
    }

    VkMemoryRequirements MemoryRequirements;
    vkGetImageMemoryRequirements(Context->Device, Texture->Image, &MemoryRequirements);

    if(!VK_Memory_Allocate(&Context->MemoryManager, &MemoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &Texture->Allocation)) {
        //todo: Logging
        return false;
    }

    VkDeviceMemory Memory = VK_Get_Memory_Block(&Texture->Allocation.Allocate)->Memory;
    if(vkBindImageMemory(Context->Device, Texture->Image, Memory, Texture->Allocation.Allocate.Offset) != VK_SUCCESS) {
        //todo: Logging
        return false;
    }

    Texture->Width = CreateInfo.Width;
    Texture->Height = CreateInfo.Height;
    Texture->Format = CreateInfo.Format;
    Texture->IsSwapchain = false;

    return true;
}

internal void VK_Delete_Texture(gdi_context* Context, vk_texture* Texture) {
    //If texture is managed by the swapchain we do not free and destroy the image
    if(Texture->IsSwapchain) return;

    VK_Memory_Free(&Context->MemoryManager, &Texture->Allocation);
    if(Texture->Image) {
        vkDestroyImage(Context->Device, Texture->Image, Context->VKAllocator);
        Texture->Image = VK_NULL_HANDLE;
    }
}

internal void VK_Texture_Record_Frame(gdi_context* Context, async_handle<vk_texture> Handle) {
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.TexturesInUse[Handle.Index()], true);
}
