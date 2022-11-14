#ifndef GL_RESOURCE_H
#define GL_RESOURCE_H

typedef struct gl_texture2D
{
    GLuint             Handle;
    gpu_texture_format Format;
    gl_texture2D*      Next;
} gl_texture2D;

typedef struct gl_framebuffer
{
    GLuint                 Handle;
    struct gl_framebuffer* Next;
} gl_framebuffer;

typedef struct gl_resource_manager
{
    gpu_resource_manager ResourceManager;
    gl_device_context*   DeviceContext;
    arena*               Arena;
    gl_texture2D*        FreeTextures;
    gl_framebuffer*      FreeFramebuffers;
} gl_resource_manager;

gl_resource_manager* GL_Resource_Manager_Create(allocator* Allocator, gl_device_context* DeviceContext);
void GL_Resource_Manager_Delete(gl_resource_manager* ResourceManager);
GPU_RESOURCE_MANAGER_CREATE_TEXTURE2D(GL_Resource_Manager_Create_Texture2D);
GPU_RESOURCE_MANAGER_DELETE_TEXTURE2D(GL_Resource_Manager_Delete_Texture2D);
GPU_RESOURCE_MANAGER_CREATE_FRAMEBUFFER(GL_Resource_Manager_Create_Framebuffer);
GPU_RESOURCE_MANAGER_DELETE_FRAMEBUFFER(GL_Resource_Manager_Delete_Framebuffer);

#endif