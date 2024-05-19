bool VK_Create_Render_Pass(gdi_context* Context, vk_render_pass* RenderPass, const gdi_render_pass_create_info& CreateInfo) {
    scratch Scratch = Scratch_Get();
    array<VkAttachmentDescription> Attachments(&Scratch, CreateInfo.Attachments.Count);
    array<VkAttachmentReference>   ColorAttachments(&Scratch, CreateInfo.Attachments.Count);
    VkAttachmentReference* DepthAttachment = NULL;

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

            case gdi_render_pass_attachment_type::Depth: {
                Layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                Assert(!DepthAttachment);
                DepthAttachment = Scratch_Push_Struct(&Scratch, VkAttachmentReference);
                DepthAttachment->attachment = AttachmentIndex;
                DepthAttachment->layout = Layout;
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
        .pColorAttachments = ColorAttachments.Ptr,
        .pDepthStencilAttachment = DepthAttachment
    };

    VkRenderPassCreateInfo RenderPassCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = Safe_U32(Attachments.Count),
        .pAttachments = Attachments.Ptr,
        .subpassCount = 1,
        .pSubpasses = &Subpass
    };

    if(vkCreateRenderPass(Context->Device, &RenderPassCreateInfo, Context->VKAllocator, &RenderPass->Handle) != VK_SUCCESS) {
        //todo: Diagnostic 
        return false;
    }

    return true;
}

void VK_Delete_Render_Pass(gdi_context* Context, vk_render_pass* RenderPass) {
    if(RenderPass->Handle) {
        vkDestroyRenderPass(Context->Device, RenderPass->Handle, Context->VKAllocator);
        RenderPass->Handle = VK_NULL_HANDLE;
    }
}

bool VK_Create_Framebuffer(gdi_context* Context, vk_framebuffer* Framebuffer, const gdi_framebuffer_create_info& CreateInfo) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    
    scratch Scratch = Scratch_Get();
    array<VkImageView> Attachments(&Scratch, CreateInfo.Attachments.Count);

    s32 Width  = -1;
    s32 Height = -1;

    //Make sure all attachments are loaded already
    for(uptr i = 0; i < CreateInfo.Attachments.Count; i++) {
        vk_handle<vk_texture_view> TextureViewHandle(CreateInfo.Attachments[i].ID);
        vk_texture_view* TextureView = VK_Resource_Get(ResourceContext->TextureViews, TextureViewHandle);
        if(!TextureView) {
            //todo: diagnostic 
            return false;
        }

        vk_texture* Texture = TextureView->References[0].Get_Texture();
        if(!Texture) {
            //todo: diagnostic 
            return false;
        } 

        if(Width == -1) {
            Width = Texture->Size.width;
        } else {
            if(Width != Texture->Size.width) {
                //todo: diagnostic
                return false;
            }
        }

        if(Height == -1) {
            Height = Texture->Size.height;
        } else {
            if(Height != Texture->Size.height) {
                //todo: diagnostic
                return false;
            }
        }

        Array_Push(&Attachments, TextureView->Handle);
        Framebuffer->Add_Reference(Context, TextureView, VK_RESOURCE_TYPE_TEXTURE_VIEW);
    }

    vk_handle<vk_render_pass> RenderPassHandle(CreateInfo.RenderPass.ID);
    vk_render_pass* RenderPass = VK_Resource_Get(ResourceContext->RenderPasses, RenderPassHandle);
    if(!RenderPass) {
        //todo: diagnostic
        return false;
    }
    
    VkFramebufferCreateInfo FramebufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = RenderPass->Handle,
        .attachmentCount = Safe_U32(Attachments.Count),
        .pAttachments = Attachments.Ptr,
        .width = (u32)Width,
        .height = (u32)Height,
        .layers = 1
    };

    if(vkCreateFramebuffer(Context->Device, &FramebufferCreateInfo, Context->VKAllocator, &Framebuffer->Handle) != VK_SUCCESS) {
        //todo: diagnostic
        return false;
    }

    Framebuffer->Size.width  = Width;
    Framebuffer->Size.height = Height;

    return true;
}

void VK_Delete_Framebuffer(gdi_context* Context, vk_framebuffer* Framebuffer) {
    if(Framebuffer->Handle) {
        vkDestroyFramebuffer(Context->Device, Framebuffer->Handle, Context->VKAllocator);
        Framebuffer->Handle = VK_NULL_HANDLE;
    }
}

fixed_array<gdi_handle<gdi_texture_view>> VK_Framebuffer_Get_Attachments(gdi_context* Context, vk_framebuffer* Framebuffer, allocator* Allocator) {
    vk_resource_context* ResourceContext = &Context->ResourceContext;
    uptr AttachmentCount = Framebuffer->References.Count;
    if(!AttachmentCount) return {};

    fixed_array<gdi_handle<gdi_texture_view>> Result(Allocator, AttachmentCount);
    for(uptr i = 0; i < AttachmentCount; i++) {
        vk_texture_view* TextureView = Framebuffer->References[i].Get_Texture_View();
        vk_handle<vk_texture_view> Handle(Safe_U32(ResourceContext->TextureViews.Get_Index(TextureView)), 
                                          Framebuffer->References[i].Generation);
        Result[i] = gdi_handle<gdi_texture_view>(Handle.ID);
    }
    return Result;
}