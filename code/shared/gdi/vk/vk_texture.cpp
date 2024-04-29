
internal bool VK_Create_Sampler(gdi_context* Context, vk_sampler* Sampler, const gdi_sampler_create_info& CreateInfo) {
     VkSamplerCreateInfo SamplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_Get_Filter(CreateInfo.Filter),
        .minFilter = VK_Get_Filter(CreateInfo.Filter),
        .addressModeU = VK_Get_Address_Mode(CreateInfo.AddressModeU),
        .addressModeV = VK_Get_Address_Mode(CreateInfo.AddressModeV)
     };

     if(vkCreateSampler(Context->Device, &SamplerInfo, Context->VKAllocator, &Sampler->Handle) != VK_SUCCESS) {
        //todo: Logging
        return false;
     }
     return true;
}

internal void VK_Delete_Sampler(gdi_context* Context, vk_sampler* Sampler) {
    if(Sampler->Handle) {
        vkDestroySampler(Context->Device, Sampler->Handle, Context->VKAllocator);
        Sampler->Handle = VK_NULL_HANDLE;
    }
}

internal bool VK_Create_Texture_View(gdi_context* Context, vk_texture_view* TextureView, const gdi_texture_view_create_info& CreateInfo) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;

    vk_handle<vk_texture> TextureHandle(CreateInfo.Texture.ID);
    vk_texture* Texture = VK_Resource_Get(ResourceContext->Textures, TextureHandle);
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
        .image = Texture->Handle,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_Get_Format(Format),
        .subresourceRange = {ImageAspect, 0, 1, 0, 1}
    };

    if(vkCreateImageView(Context->Device, &ImageViewCreateInfo, Context->VKAllocator, &TextureView->Handle) != VK_SUCCESS) {
        //todo: diagnostic
        return false;
    }

    TextureView->Add_Reference(Context, Texture, VK_RESOURCE_TYPE_TEXTURE);

    return true;
}

internal void VK_Delete_Texture_View(gdi_context* Context, vk_texture_view* TextureView) {
    if(TextureView->Handle) {
        vkDestroyImageView(Context->Device, TextureView->Handle, Context->VKAllocator);
        TextureView->Handle = VK_NULL_HANDLE;
    }
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

    if(vkCreateImage(Context->Device, &ImageInfo, Context->VKAllocator, &Texture->Handle) != VK_SUCCESS) {
        //todo: Logging
        return false;
    }

    VkMemoryRequirements MemoryRequirements;
    vkGetImageMemoryRequirements(Context->Device, Texture->Handle, &MemoryRequirements);

    if(!VK_Memory_Allocate(&Context->MemoryManager, &MemoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &Texture->Allocation)) {
        //todo: Logging
        return false;
    }

    VkDeviceMemory Memory = VK_Get_Memory_Block(&Texture->Allocation.Allocate)->Memory;
    if(vkBindImageMemory(Context->Device, Texture->Handle, Memory, Texture->Allocation.Allocate.Offset) != VK_SUCCESS) {
        //todo: Logging
        return false;
    }

    Texture->Width = CreateInfo.Width;
    Texture->Height = CreateInfo.Height;
    Texture->Format = CreateInfo.Format;
    Texture->IsSwapchain = false;
    Texture->JustAllocated = true;

    return true;
}

internal void VK_Delete_Texture(gdi_context* Context, vk_texture* Texture) {    
    //If texture is managed by the swapchain we do not free and destroy the image
    if(Texture->IsSwapchain) return;

    if(Texture->Handle) {
        VK_Memory_Free(&Context->MemoryManager, &Texture->Allocation);
        vkDestroyImage(Context->Device, Texture->Handle, Context->VKAllocator);
        Texture->Handle = VK_NULL_HANDLE;
    }
}