
vk_resource_base* vk_resource_ref::Get_Resource() {
    if(!Generation || !Resource) return nullptr;
    if(Generation != AK_Atomic_Load_U32_Relaxed(&Resource->Generation)) return nullptr;
    return Resource;
}

void vk_resource_base::Add_Reference(gdi_context* Context, vk_resource_ref Reference) {
    if(!References.Allocator) {
        References = array<vk_resource_ref>(Context->GDI->MainAllocator);
    }
    Array_Push(&References, Reference);
}

void vk_resource_base::Add_Reference(gdi_context* Context, vk_resource_base* Resource, vk_resource_type Type) {
    Add_Reference(Context, {Type, AK_Atomic_Load_U32_Relaxed(&Resource->Generation), Resource});
}

void VK_Resource_Record_Frame(vk_resource_base* Resource) {
    b32 LastState = (b32)AK_Atomic_Exchange_U32_Relaxed(&Resource->InUse, true);
    if(!LastState) {
        for(vk_resource_ref& Ref : Resource->References) {
            vk_resource_base* RefResource = Ref.Get_Resource();
            if(RefResource) {
                VK_Resource_Record_Frame(RefResource);
            }
        }
    }
}

internal inline void VK_Delete_Resource(gdi_context* Context, vk_pipeline* Pipeline) {
    VK_Delete_Pipeline(Context, Pipeline);
}

internal inline void VK_Delete_Resource(gdi_context* Context, vk_bind_group* BindGroup) {
    VK_Delete_Bind_Group(Context, BindGroup);
} 

internal inline void VK_Delete_Resource(gdi_context* Context, vk_bind_group_layout* BindGroupLayout) {
    VK_Delete_Bind_Group_Layout(Context, BindGroupLayout);
} 

internal inline void VK_Delete_Resource(gdi_context* Context, vk_framebuffer* Framebuffer) {
    VK_Delete_Framebuffer(Context, Framebuffer);
} 

internal inline void VK_Delete_Resource(gdi_context* Context, vk_render_pass* RenderPass) {
    VK_Delete_Render_Pass(Context, RenderPass);
} 

internal inline void VK_Delete_Resource(gdi_context* Context, vk_sampler* Sampler) {
    VK_Delete_Sampler(Context, Sampler);
}

internal inline void VK_Delete_Resource(gdi_context* Context, vk_texture_view* TextureView) {
    VK_Delete_Texture_View(Context, TextureView);
} 

internal inline void VK_Delete_Resource(gdi_context* Context, vk_texture* Texture) {
    VK_Delete_Texture(Context, Texture);
} 

internal inline void VK_Delete_Resource(gdi_context* Context, vk_buffer* Buffer) {
    VK_Delete_Buffer(Context, Buffer);
} 

internal inline void VK_Delete_Resource(gdi_context* Context, vk_swapchain* Swapchain) {
    VK_Delete_Swapchain(Context, Swapchain);
} 

template <typename type>
inline void VK_Resource_Update_Frame_Indices(gdi_context* Context, vk_resource_pool<type>& Pool) {
    for(u32 i = 0; i < Pool.FreeIndices.Capacity; i++) {
        type* Resource = Pool.Resources + i;
        if(AK_Atomic_Load_U32(&Resource->InUse, AK_ATOMIC_MEMORY_ORDER_ACQUIRE)) {
            Resource->LastUsedFrameIndex = Context->TotalFramesRendered;
            AK_Atomic_Store_U32(&Resource->InUse, false, AK_ATOMIC_MEMORY_ORDER_RELEASE);
        }
    }
}

template <typename type>
inline void VK_Create_Resource_Pool(vk_resource_pool<type>* Pool, arena* Arena, u32 MaxCount) {
    u32* FreeIndices = Arena_Push_Array(Arena, MaxCount, u32);
    AK_Async_Stack_Index32_Init_Raw(&Pool->FreeIndices, FreeIndices, MaxCount);
    Pool->Resources = Arena_Push_Array(Arena, MaxCount, type);
    for(u32 i = 0; i < MaxCount; i++) { 
        Pool->Resources[i].LastUsedFrameIndex = (u64)-1;
        Pool->Resources[i].Generation.Nonatomic = 1; 
    }
    for(u32 i = 0; i < MaxCount; i++) { AK_Async_Stack_Index32_Push_Sync(&Pool->FreeIndices, i); }
}

template <typename type>
inline void VK_Delete_Resource_Pool(gdi_context* Context, vk_resource_pool<type>* Pool) {
    for(u32 i = 0; i < Pool->FreeIndices.Capacity; i++) {
        type* Resource = Pool->Resources + i;
        VK_Delete_Resource(Context, Resource);
    }
}

void VK_Resource_Update_Last_Frame_Indices(vk_resource_context* ResourceContext) {
    gdi_context* Context = ResourceContext->Context;
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->Pipelines);
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->BindGroups);
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->BindGroupLayouts);
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->Framebuffers);
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->RenderPasses);
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->Samplers);
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->TextureViews);
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->Textures);
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->Buffers);
    VK_Resource_Update_Frame_Indices(Context, ResourceContext->Swapchains);
}

bool VK_Resource_Should_Delete(gdi_context* Context, vk_resource_base* Resource, u64* OutDeleteFrameIndex) {
    u64 LastUsedFrameIndex = Resource->LastUsedFrameIndex;
    u64 Difference = Context->TotalFramesRendered - LastUsedFrameIndex;
    
    //If the resource is not in use or if the resource has not been used within the last 
    //frames, then we can safely delete the resource.
    if(LastUsedFrameIndex == (u64)-1 || 
       (!AK_Atomic_Load_U32_Relaxed(&Resource->InUse) &&
       (!Difference || Difference > Context->Frames.Count))) {
        return true;
    } else {
        *OutDeleteFrameIndex = AK_Atomic_Load_U32_Relaxed(&Resource->InUse) ? Context->TotalFramesRendered : LastUsedFrameIndex;
        return false;
    }
}

void VK_Resource_Context_Create(gdi_context* Context, vk_resource_context* ResourceContext, const gdi_context_create_info& CreateInfo) {
    ResourceContext->Context = Context;
    
    VK_Create_Resource_Pool(&ResourceContext->Pipelines, Context->Arena, CreateInfo.PipelineCount);
    VK_Create_Resource_Pool(&ResourceContext->BindGroups, Context->Arena, CreateInfo.BindGroupCount);
    VK_Create_Resource_Pool(&ResourceContext->BindGroupLayouts, Context->Arena, CreateInfo.BindGroupLayoutCount);
    VK_Create_Resource_Pool(&ResourceContext->Framebuffers, Context->Arena, CreateInfo.FramebufferCount);
    VK_Create_Resource_Pool(&ResourceContext->RenderPasses, Context->Arena, CreateInfo.RenderPassCount);
    VK_Create_Resource_Pool(&ResourceContext->Samplers, Context->Arena, CreateInfo.SamplerCount);
    VK_Create_Resource_Pool(&ResourceContext->TextureViews, Context->Arena, CreateInfo.TextureViewCount);
    VK_Create_Resource_Pool(&ResourceContext->Textures, Context->Arena, CreateInfo.TextureCount);
    VK_Create_Resource_Pool(&ResourceContext->Buffers, Context->Arena, CreateInfo.BufferCount);
    VK_Create_Resource_Pool(&ResourceContext->Swapchains, Context->Arena, CreateInfo.SwapchainCount);
}

void VK_Resource_Context_Delete(vk_resource_context* ResourceContext) {
    gdi_context* Context = ResourceContext->Context;

    VK_Delete_Resource_Pool(Context, &ResourceContext->Pipelines);
    VK_Delete_Resource_Pool(Context, &ResourceContext->BindGroups);
    VK_Delete_Resource_Pool(Context, &ResourceContext->BindGroupLayouts);
    VK_Delete_Resource_Pool(Context, &ResourceContext->Framebuffers);
    VK_Delete_Resource_Pool(Context, &ResourceContext->RenderPasses);
    VK_Delete_Resource_Pool(Context, &ResourceContext->Samplers);
    VK_Delete_Resource_Pool(Context, &ResourceContext->TextureViews);
    VK_Delete_Resource_Pool(Context, &ResourceContext->Textures);
    VK_Delete_Resource_Pool(Context, &ResourceContext->Buffers);

    for(u32 i = 0; i < ResourceContext->Swapchains.FreeIndices.Capacity; i++) {
        vk_swapchain* Swapchain = ResourceContext->Swapchains.Resources + i;
        VK_Delete_Swapchain_Full(Context, Swapchain);
    }
}

#include "vk_swapchain.cpp"
#include "vk_buffer.cpp"
#include "vk_texture.cpp"
#include "vk_render_pass.cpp"
#include "vk_bind_groups.cpp"
#include "vk_pipeline.cpp"