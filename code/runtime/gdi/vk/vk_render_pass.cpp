bool VK_Create_Render_Pass(gdi_context* Context, vk_render_pass* RenderPass, const gdi_render_pass_create_info& CreateInfo) {
    scratch Scratch = Scratch_Get();
    array<VkAttachmentDescription> Attachments(&Scratch, CreateInfo.Attachments.Count);
    array<VkAttachmentReference>   ColorAttachments(&Scratch, CreateInfo.Attachments.Count);

    for(const gdi_render_pass_attachment& Attachment : CreateInfo.Attachments) {
        u32 AttachmentIndex = Safe_U32(Attachments.Count);

        VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
        switch(Attachment.Type) {
            case gdi_render_pass_attachment_type::Color: {
                Layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                Array_Push(&ColorAttachments, {
                    .attachment = AttachmentIndex,
                    .layout = Layout
                });
            } break;

            Invalid_Default_Case();
        }
        
        Array_Push(&Attachments, {
            .format = VK_Get_Format(Attachment.Format),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_Get_Load_Op(Attachment.LoadOp),
            .storeOp = VK_Get_Store_Op(Attachment.StoreOp),
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = Layout,
            .finalLayout = Layout
        });
    }

    VkSubpassDescription Subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = Safe_U32(ColorAttachments.Count),
        .pColorAttachments = ColorAttachments.Ptr
    };

    VkRenderPassCreateInfo RenderPassCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = Safe_U32(Attachments.Count),
        .pAttachments = Attachments.Ptr,
        .subpassCount = 1,
        .pSubpasses = &Subpass
    };

    if(vkCreateRenderPass(Context->Device, &RenderPassCreateInfo, Context->VKAllocator, &RenderPass->RenderPass) != VK_SUCCESS) {
        //todo: Diagnostic 
        return false;
    }

    return true;
}

void VK_Delete_Render_Pass(gdi_context* Context, vk_render_pass* RenderPass) {
    if(RenderPass->RenderPass) {
        vkDestroyRenderPass(Context->Device, RenderPass->RenderPass, Context->VKAllocator);
        RenderPass->RenderPass = VK_NULL_HANDLE;
    }
}

using texture_view_reader_lock = pool_reader_lock<vk_texture_view>;
using texture_reader_lock = pool_reader_lock<vk_texture>; 

bool VK_Create_Framebuffer(gdi_context* Context, vk_framebuffer* Framebuffer, const gdi_framebuffer_create_info& CreateInfo) {
    scratch Scratch = Scratch_Get();
    array<VkImageView> Attachments(&Scratch, CreateInfo.Attachments.Count);

    fixed_array<texture_view_reader_lock> TextureViews(&Scratch, CreateInfo.Attachments.Count);
    fixed_array<texture_reader_lock> Textures(&Scratch, CreateInfo.Attachments.Count);

    array_scoped<pool_scoped_lock<texture_view_reader_lock>> TextureViewsScoped(&Scratch, CreateInfo.Attachments.Count);
    array_scoped<pool_scoped_lock<texture_reader_lock>> TexturesScoped(&Scratch, CreateInfo.Attachments.Count);

    u32 Width  = (u32)-1;
    u32 Height = (u32)-1;

    //Make sure all attachments are loaded already
    for(uptr i = 0; i < CreateInfo.Attachments.Count; i++) {
        gdi_handle<gdi_texture_view> Handle = CreateInfo.Attachments[i];
        async_handle<vk_texture_view> TextureViewHandle(Handle.ID);

        TextureViews[i] = pool_reader_lock(&Context->TextureViewManager.Pool, TextureViewHandle);
        if(!TextureViews[i].Ptr) {
            //todo: diagnostic 
            return false;
        }
        TextureViewsScoped[i] = pool_scoped_lock(&TextureViews[i]);

        async_handle<vk_texture> TextureHandle(TextureViews[i]->TextureHandle.ID);
        Textures[i] = pool_reader_lock(&Context->TextureManager.Pool, TextureHandle);
        if(!Textures[i].Ptr) {
            //todo: diagnostic 
            return false;
        } 
        TexturesScoped[i] = pool_scoped_lock(&Textures[i]);

        if(Width == (u32)-1) {
            Width = Textures[i]->Width;
        } else {
            if(Width != Textures[i]->Width) {
                //todo: diagnostic
                return false;
            }
        }

        if(Height == (u32)-1) {
            Height = Textures[i]->Height;
        } else {
            if(Height != Textures[i]->Height) {
                //todo: diagnostic
                return false;
            }
        }

        Array_Push(&Attachments, TextureViews[i]->ImageView);
    }

    async_handle<vk_render_pass> RenderPassHandle(CreateInfo.RenderPass.ID);
    pool_reader_lock RenderPassReader(&Context->RenderPassManager.Pool, RenderPassHandle);
    if(!RenderPassReader.Ptr) {
        //todo: diagnostic
        return false;
    }
    pool_scoped_lock RenderPassLockScoped(&RenderPassReader);
    
    VkRenderPass RenderPass = RenderPassReader->RenderPass;
    VkFramebufferCreateInfo FramebufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = RenderPass,
        .attachmentCount = Safe_U32(Attachments.Count),
        .pAttachments = Attachments.Ptr,
        .width = Width,
        .height = Height,
        .layers = 1
    };

    if(vkCreateFramebuffer(Context->Device, &FramebufferCreateInfo, Context->VKAllocator, &Framebuffer->Framebuffer) != VK_SUCCESS) {
        //todo: diagnostic
        return false;
    }

    Array_Init(&Framebuffer->Attachments, Context->GDI->MainAllocator, CreateInfo.Attachments);
    return true;
}

void VK_Delete_Framebuffer(gdi_context* Context, vk_framebuffer* Framebuffer) {
    Array_Free(&Framebuffer->Attachments, Context->GDI->MainAllocator);
    if(Framebuffer->Framebuffer) {
        vkDestroyFramebuffer(Context->Device, Framebuffer->Framebuffer, Context->VKAllocator);
        Framebuffer->Framebuffer = VK_NULL_HANDLE;
    }
}