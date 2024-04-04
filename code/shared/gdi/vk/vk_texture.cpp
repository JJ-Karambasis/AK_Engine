void VK_Texture_Record_Frame(gdi_context* Context, async_handle<vk_texture> Handle) {
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.TexturesInUse[Handle.Index()], true);
}

bool VK_Create_Texture_View(gdi_context* Context, vk_texture_view* TextureView, const gdi_texture_view_create_info& CreateInfo) {

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

void VK_Delete_Texture_View(gdi_context* Context, vk_texture_view* TextureView) {
    if(TextureView->ImageView) {
        vkDestroyImageView(Context->Device, TextureView->ImageView, Context->VKAllocator);
        TextureView->ImageView = VK_NULL_HANDLE;
    }
}

void VK_Texture_View_Record_Frame(gdi_context* Context, async_handle<vk_texture_view> Handle) {
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.TextureViewsInUse[Handle.Index()], true);
}