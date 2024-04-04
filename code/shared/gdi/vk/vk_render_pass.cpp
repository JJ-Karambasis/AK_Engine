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

internal void VK_Render_Pass_Record_Frame(gdi_context* Context, async_handle<vk_render_pass> Handle) {
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.RenderPassesInUse[Handle.Index()], true);
}

bool VK_Create_Framebuffer(gdi_context* Context, vk_framebuffer* Framebuffer, const gdi_framebuffer_create_info& CreateInfo) {
    scratch Scratch = Scratch_Get();
    array<VkImageView> Attachments(&Scratch, CreateInfo.Attachments.Count);

    u32 Width  = (u32)-1;
    u32 Height = (u32)-1;

    //Make sure all attachments are loaded already
    for(uptr i = 0; i < CreateInfo.Attachments.Count; i++) {
        gdi_handle<gdi_texture_view> Handle = CreateInfo.Attachments[i];
        async_handle<vk_texture_view> TextureViewHandle(Handle.ID);

        vk_texture_view* TextureView = Async_Pool_Get(&Context->ResourceContext.TextureViews, TextureViewHandle);
        if(!TextureView) {
            //todo: diagnostic 
            return false;
        }

        async_handle<vk_texture> TextureHandle(TextureView->TextureHandle.ID);
        vk_texture* Texture = Async_Pool_Get(&Context->ResourceContext.Textures, TextureHandle);
        if(!Texture) {
            //todo: diagnostic 
            return false;
        } 

        if(Width == (u32)-1) {
            Width = Texture->Width;
        } else {
            if(Width != Texture->Width) {
                //todo: diagnostic
                return false;
            }
        }

        if(Height == (u32)-1) {
            Height = Texture->Height;
        } else {
            if(Height != Texture->Height) {
                //todo: diagnostic
                return false;
            }
        }

        Array_Push(&Attachments, TextureView->ImageView);
    }


    async_handle<vk_render_pass> RenderPassHandle(CreateInfo.RenderPass.ID);
    vk_render_pass* RenderPass = Async_Pool_Get(&Context->ResourceContext.RenderPasses, RenderPassHandle);
    if(!RenderPass) {
        //todo: diagnostic
        return false;
    }
    
    VkFramebufferCreateInfo FramebufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = RenderPass->RenderPass,
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

    Framebuffer->Width  = Width;
    Framebuffer->Height = Height;

    Array_Init(&Framebuffer->Attachments, Context->GDI->MainAllocator, CreateInfo.Attachments.Count);
    for(uptr i = 0; i < CreateInfo.Attachments.Count; i++) {
        Framebuffer->Attachments[i] = async_handle<vk_texture_view>(CreateInfo.Attachments[i].ID);
    }

    return true;
}

void VK_Delete_Framebuffer(gdi_context* Context, vk_framebuffer* Framebuffer) {
    Array_Free(&Framebuffer->Attachments, Context->GDI->MainAllocator);
    if(Framebuffer->Framebuffer) {
        vkDestroyFramebuffer(Context->Device, Framebuffer->Framebuffer, Context->VKAllocator);
        Framebuffer->Framebuffer = VK_NULL_HANDLE;
    }
} 

internal void VK_Framebuffer_Record_Frame(gdi_context* Context, async_handle<vk_framebuffer> Handle) {
    AK_Atomic_Store_U32_Relaxed(&Context->ResourceContext.FramebuffersInUse[Handle.Index()], true);
}