static GLenum G_InternalFormatMap[GPU_TEXTURE_FORMAT_COUNT] = 
{
    0,
    GL_RGBA8
};

static GLenum G_FormatMap[GPU_TEXTURE_FORMAT_COUNT] = 
{
    0,
    GL_RGBA
};

static GLenum G_TypeMap[GPU_TEXTURE_FORMAT_COUNT] = 
{
    0,
    GL_UNSIGNED_BYTE
};

static uint32_t G_BytesPerPixel[GPU_TEXTURE_FORMAT_COUNT] = 
{
    0, 
    4
};

static gpu_resource_manager_vtable G_ResourceManagerVTable = 
{
    GL_Resource_Manager_Create_Texture2D,
    GL_Resource_Manager_Delete_Texture2D,
    GL_Resource_Manager_Create_Framebuffer,
    GL_Resource_Manager_Delete_Framebuffer
};

gl_resource_manager* GL_Resource_Manager_Create(allocator* Allocator, gl_device_context* DeviceContext)
{
    arena* Arena = Arena_Create(Allocator, Kilo(8));
    gl_resource_manager* ResourceManager = Arena_Push_Struct(Arena, gl_resource_manager);
    ResourceManager->Arena = Arena;
    ResourceManager->DeviceContext = DeviceContext;
    Set_VTable(&ResourceManager->ResourceManager, &G_ResourceManagerVTable);
    return ResourceManager;
}

void GL_Resource_Manager_Delete(gl_resource_manager* ResourceManager)
{
    Arena_Delete(ResourceManager->Arena);
}

GPU_RESOURCE_MANAGER_CREATE_TEXTURE2D(GL_Resource_Manager_Create_Texture2D)
{
    //TODO(JJ): We aren't taking into account usage at all here. If the usage is not sampled image we can 
    //probably just create a render buffer and attach that to framebuffers instead
    
    gl_resource_manager* ResourceManager = (gl_resource_manager*)_Manager;
    gl_texture2D* Result = ResourceManager->FreeTextures;
    if(!Result) Result = Arena_Push_Struct(ResourceManager->Arena, gl_texture2D);
    else SLL_Pop_Front(ResourceManager->FreeTextures);
    Zero_Struct(Result, gl_texture2D);
    Result->Format = Format;
    
    glGenTextures(1, &Result->Handle);
    
    glBindTexture(GL_TEXTURE_2D, Result->Handle);
    glTexImage2D(GL_TEXTURE_2D, 0, G_InternalFormatMap[Format], Width, Height, 0, G_FormatMap[Format], G_TypeMap[Format], NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return (gpu_texture2D*)Result;
}

#if 0 
GPU_RESOURCE_MANAGER_UPLOAD_TEXTURE2D(GL_Resource_Manager_Upload_Texture2D)
{
    gl_texture2D* DstTexture = (gl_texture2D*)_DstTexture;
    glBindTexture(GL_TEXTURE_2D, DstTexture->Handle);
    glTexSubImage2D(GL_TEXTURE_2D, 0, DstOffsetX, DstOffsetY, SrcWidth, SrcHeight, DstTexture->Format, DstTexture->Type, SrcTexels);
    glBindTexture(GL_TEXTURE_2D, 0);
}
#endif
GPU_RESOURCE_MANAGER_DELETE_TEXTURE2D(GL_Resource_Manager_Delete_Texture2D)
{
    gl_resource_manager* ResourceManager = (gl_resource_manager*)_Manager;
    gl_texture2D* Texture = (gl_texture2D*)_Texture;
    glDeleteTextures(1, &Texture->Handle);
    SLL_Push_Front(ResourceManager->FreeTextures, Texture);
}

GPU_RESOURCE_MANAGER_CREATE_FRAMEBUFFER(GL_Resource_Manager_Create_Framebuffer)
{
    GLuint Handle;
    glGenFramebuffers(1, &Handle);
    glBindFramebuffer(GL_FRAMEBUFFER, Handle);
    
    for(uint32_t ColorAttachmentIndex = 0; ColorAttachmentIndex < CreateInfo->ColorAttachmentCount; ColorAttachmentIndex++)
    {
        gl_texture2D* ColorAttachment = (gl_texture2D*)CreateInfo->ColorAttachments[ColorAttachmentIndex];
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+ColorAttachmentIndex, GL_TEXTURE_2D, ColorAttachment->Handle, 0);
    }
    
    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(Status != GL_FRAMEBUFFER_COMPLETE) return NULL;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    gl_resource_manager* ResourceManager = (gl_resource_manager*)_Manager;
    gl_framebuffer* Framebuffer = ResourceManager->FreeFramebuffers;
    if(!Framebuffer) Framebuffer = Arena_Push_Struct(ResourceManager->Arena, gl_framebuffer);
    else SLL_Pop_Front(ResourceManager->FreeFramebuffers);
    Zero_Struct(Framebuffer, gl_framebuffer);
    Framebuffer->Handle = Handle;
    
    return (gpu_framebuffer*)Framebuffer;
}

GPU_RESOURCE_MANAGER_DELETE_FRAMEBUFFER(GL_Resource_Manager_Delete_Framebuffer)
{
    gl_resource_manager* ResourceManager = (gl_resource_manager*)_Manager;
    gl_framebuffer* Framebuffer = (gl_framebuffer*)_Framebuffer;
    glDeleteFramebuffers(1, &Framebuffer->Handle);
    SLL_Push_Front(ResourceManager->FreeFramebuffers, Framebuffer);
}