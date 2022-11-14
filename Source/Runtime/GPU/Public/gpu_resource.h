#ifndef GPU_RESOURCE_H
#define GPU_RESOURCE_H

typedef struct gpu_resource_manager gpu_resource_manager;
typedef struct gpu_texture2D   gpu_texture2D;
typedef struct gpu_framebuffer gpu_framebuffer;
typedef struct gpu_framebuffer_create_info gpu_framebuffer_create_info;

typedef enum gpu_texture_format
{
    GPU_TEXTURE_FORMAT_UNKNOWN,
    GPU_TEXTURE_FORMAT_R8G8B8A8_UNORM,
    GPU_TEXTURE_FORMAT_COUNT
} gpu_texture_format;

#define GPU_TEXTURE_USAGE_RENDER_TARGET (1 << 0)
#define GPU_TEXTURE_USAGE_SAMPLED       (1 << 1)

#define GPU_RESOURCE_MANAGER_CREATE_TEXTURE2D(name) gpu_texture2D* name(gpu_resource_manager* _Manager, uint32_t Width, uint32_t Height, gpu_texture_format Format, uint32_t UsageFlags)
#define GPU_RESOURCE_MANAGER_DELETE_TEXTURE2D(name) void name(gpu_resource_manager* _Manager, gpu_texture2D* _Texture)

#define GPU_RESOURCE_MANAGER_CREATE_FRAMEBUFFER(name) gpu_framebuffer* name(gpu_resource_manager* _Manager, gpu_framebuffer_create_info* CreateInfo)
#define GPU_RESOURCE_MANAGER_DELETE_FRAMEBUFFER(name) void name(gpu_resource_manager* _Manager, gpu_framebuffer* _Framebuffer)

typedef GPU_RESOURCE_MANAGER_CREATE_TEXTURE2D(gpu_resource_manager_create_texture2d);
typedef GPU_RESOURCE_MANAGER_DELETE_TEXTURE2D(gpu_resource_manager_delete_texture2d);
typedef GPU_RESOURCE_MANAGER_CREATE_FRAMEBUFFER(gpu_resource_manager_create_framebuffer);
typedef GPU_RESOURCE_MANAGER_DELETE_FRAMEBUFFER(gpu_resource_manager_delete_framebuffer);

#define GPU_Resource_Manager_Create_Texture2D(ResourceManager, Width, Height, Format, Flags) (ResourceManager)->_VTable->Create_Texture2D(ResourceManager, Width, Height, Format, Flags)
#define GPU_Resource_Manager_Delete_Texture2D(ResourceManager, Texture) (ResourceManager)->_VTable->Delete_Texture2D(ResourceManager, Texture)
#define GPU_Resource_Manager_Create_Framebuffer(ResourceManager, CreateInfo) (ResourceManager)->_VTable->Create_Framebuffer(ResourceManager, CreateInfo)
#define GPU_Resource_Manager_Delete_Framebuffer(ResourceManager, Framebuffer) (ResourceManager)->_VTable->Delete_Framebuffer(ResourceManager, Framebuffer)

typedef struct gpu_resource_manager_vtable
{
    gpu_resource_manager_create_texture2d*   Create_Texture2D;
    gpu_resource_manager_delete_texture2d*   Delete_Texture2D;
    gpu_resource_manager_create_framebuffer* Create_Framebuffer;
    gpu_resource_manager_delete_framebuffer* Delete_Framebuffer;
} gpu_resource_manager_vtable;

typedef struct gpu_resource_manager
{
    gpu_resource_manager_vtable* _VTable;
} gpu_resource_manager;

typedef struct gpu_framebuffer_create_info
{
    v2i Dim;
    uint32_t        ColorAttachmentCount;
    gpu_texture2D** ColorAttachments;
    gpu_texture2D*  DepthAttachment;
}  gpu_framebuffer_create_info;

#endif