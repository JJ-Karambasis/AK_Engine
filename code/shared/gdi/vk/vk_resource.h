#ifndef VK_RESOURCE_H
#define VK_RESOURCE_H

enum vk_resource_type {
    VK_RESOURCE_TYPE_NONE,
    VK_RESOURCE_TYPE_PIPELINE,
    VK_RESOURCE_TYPE_BIND_GROUP,
    VK_RESOURCE_TYPE_BIND_GROUP_LAYOUT,
    VK_RESOURCE_TYPE_FRAMEBUFFER,
    VK_RESOURCE_TYPE_RENDER_PASS,
    VK_RESOURCE_TYPE_SAMPLER,
    VK_RESOURCE_TYPE_TEXTURE_VIEW,
    VK_RESOURCE_TYPE_TEXTURE,
    VK_RESOURCE_TYPE_BUFFER,
    VK_RESOURCE_TYPE_SWAPCHAIN
};

struct vk_pipeline;
struct vk_bind_group;
struct vk_bind_group_layout;
struct vk_framebuffer;
struct vk_render_pass;
struct vk_sampler;
struct vk_texture_view;
struct vk_texture;
struct vk_buffer;
struct vk_swapchain;

struct vk_resource_ref {
    vk_resource_type         Type;
    u32                      Generation;
    struct vk_resource_base* Resource;

    vk_resource_base* Get_Resource();

    inline vk_pipeline* Get_Pipeline() {
        Assert(Type == VK_RESOURCE_TYPE_PIPELINE);
        return (vk_pipeline*)Get_Resource();
    }

    inline vk_bind_group* Get_Bind_Group() {
        Assert(Type == VK_RESOURCE_TYPE_BIND_GROUP);
        return (vk_bind_group*)Get_Resource();
    }

    inline vk_bind_group_layout* Get_Bind_Group_Layout() {
        Assert(Type == VK_RESOURCE_TYPE_BIND_GROUP_LAYOUT);
        return (vk_bind_group_layout*)Get_Resource();
    }

    inline vk_framebuffer* Get_Framebuffer() {
        Assert(Type == VK_RESOURCE_TYPE_FRAMEBUFFER);
        return (vk_framebuffer*)Get_Resource();
    }

    inline vk_render_pass* Get_Render_Pass() {
        Assert(Type == VK_RESOURCE_TYPE_RENDER_PASS);
        return (vk_render_pass*)Get_Resource();
    }

    inline vk_sampler* Get_Sampler() {
        Assert(Type == VK_RESOURCE_TYPE_SAMPLER);
        return (vk_sampler*)Get_Resource();
    }

    inline vk_texture_view* Get_Texture_View() {
        Assert(Type == VK_RESOURCE_TYPE_TEXTURE_VIEW);
        return (vk_texture_view*)Get_Resource();
    }

    inline vk_texture* Get_Texture() {
        Assert(Type == VK_RESOURCE_TYPE_TEXTURE);
        return (vk_texture*)Get_Resource();
    }
    
    inline vk_buffer* Get_Buffer() {
        Assert(Type == VK_RESOURCE_TYPE_BUFFER);
        return (vk_buffer*)Get_Resource();
    }
    
    inline vk_swapchain* Get_Swapchain() {
        Assert(Type == VK_RESOURCE_TYPE_SWAPCHAIN);
        return (vk_swapchain*)Get_Resource();
    }
};

struct vk_resource_base {
    ak_atomic_u32          InUse;
    ak_atomic_u32          Generation;
    u64                    LastUsedFrameIndex;
    array<vk_resource_ref> References;

    void Add_Reference(gdi_context* Context, vk_resource_ref Reference);
    void Add_Reference(gdi_context* Context, vk_resource_base* Resource, vk_resource_type Type);
};

template <typename type>
union vk_handle {
    u64 ID;
    struct {
        u32 Index;
        u32 Generation;
    };

    vk_handle() = default;
    inline vk_handle(u64 _ID) {
        ID = _ID;
    }

    inline vk_handle(u32 _Index, u32 _Generation) {
        Index = _Index;
        Generation = _Generation;
    }
    inline bool Is_Null() { return ID == 0; }
};

template <typename type>
struct vk_resource_pool {
    ak_async_stack_index32 FreeIndices;
    type*                  Resources;

    inline uptr Get_Index(type* Entry) {
        //Make sure the entry is in range of the resource pool. Otherwise we 
        //can't grab the index
        Assert(Entry >= Resources && Entry <= Resources+FreeIndices.Capacity);
        return ((uptr)Entry-(uptr)Resources)/sizeof(type);
    }
};

#include "vk_pipeline.h"
#include "vk_bind_groups.h"
#include "vk_render_pass.h"
#include "vk_texture.h"
#include "vk_buffer.h"
#include "vk_swapchain.h"

struct vk_resource_context {
    gdi_context* Context;
    vk_resource_pool<vk_pipeline>          Pipelines;
    vk_resource_pool<vk_bind_group>        BindGroups;
    vk_resource_pool<vk_bind_group_layout> BindGroupLayouts;
    vk_resource_pool<vk_framebuffer>       Framebuffers;
    vk_resource_pool<vk_render_pass>       RenderPasses;
    vk_resource_pool<vk_sampler>           Samplers;
    vk_resource_pool<vk_texture_view>      TextureViews;
    vk_resource_pool<vk_texture>           Textures;
    vk_resource_pool<vk_buffer>            Buffers;
    vk_resource_pool<vk_swapchain>         Swapchains;
};

void VK_Resource_Record_Frame(vk_resource_base* Resource);
void VK_Resource_Update_Last_Frame_Indices(vk_resource_context* ResourceContext);
bool VK_Resource_Should_Delete(gdi_context* Context, vk_resource_base* Resource, u64* OutDeleteFrameIndex);
void VK_Resource_Context_Create(gdi_context* Context, vk_resource_context* ResourceContext, const gdi_context_create_info& CreateInfo);
void VK_Resource_Context_Delete(vk_resource_context* ResourceContext);

template <typename type>
inline vk_handle<type> VK_Resource_Alloc(vk_resource_pool<type>& Pool) {
    u32 FreeIndex = AK_Async_Stack_Index32_Pop(&Pool.FreeIndices);
    if(FreeIndex == AK_ASYNC_STACK_INDEX32_INVALID) {
        return {};
    }

    type* Resource = Pool.Resources + FreeIndex;
    Assert(Resource->LastUsedFrameIndex == (u64)-1);
    Assert(AK_Atomic_Load_U32_Relaxed(&Resource->InUse) == false);
    
    return vk_handle<type>(FreeIndex, AK_Atomic_Load_U32_Relaxed(&Resource->Generation));
}

template <typename type>
inline type* VK_Resource_Get(vk_resource_pool<type>& Pool, vk_handle<type> Handle) {
    if(!Handle.Generation) return nullptr;
    Assert(Handle.Index < Pool.FreeIndices.Capacity);
    type* Resource = Pool.Resources + Handle.Index;
    if(AK_Atomic_Load_U32_Relaxed(&Resource->Generation) != Handle.Generation) return nullptr;
    return Resource;
}

template <typename type>
inline void VK_Resource_Free(vk_resource_pool<type>& Pool, vk_handle<type> Handle) {
    if(!Handle.Generation) return;
    u32 NextGeneration = Handle.Generation+1;
    if(!NextGeneration) NextGeneration = 1;
    Assert(Handle.Index < Pool.FreeIndices.Capacity);
    type* Resource = Pool.Resources + Handle.Index;

    if(AK_Atomic_Compare_Exchange_Bool_U32(&Resource->Generation, Handle.Generation, NextGeneration, AK_ATOMIC_MEMORY_ORDER_ACQ_REL)) {
        Resource->LastUsedFrameIndex = (u64)-1;
        AK_Atomic_Store_U32_Relaxed(&Resource->InUse, false);
        Array_Free(&Resource->References);
        Resource->Handle = VK_NULL_HANDLE;
        AK_Async_Stack_Index32_Push(&Pool.FreeIndices, Handle.Index);
    } else {
        Assert(false); //Tried to delete an invalid slot!
    }
}

#endif