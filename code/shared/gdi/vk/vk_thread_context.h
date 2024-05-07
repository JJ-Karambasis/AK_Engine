#ifndef VK_THREAD_CONTEXT_H
#define VK_THREAD_CONTEXT_H

struct vk_cmd_list {
    gdi_context*    Context;
    VkCommandBuffer CmdBuffer;
    vk_pipeline*    Pipeline;
    vk_cmd_list*    Next;
    vk_cmd_list*    Prev;
};

struct vk_cmd_storage_list {
    vk_cmd_list* Free;
    vk_cmd_list* Head;
    vk_cmd_list* Tail;
};

struct vk_cmd_pool {
    VkCommandPool       CommandPool;
    vk_cmd_storage_list PrimaryCmds;
    vk_cmd_storage_list SecondaryCmds;
};

template <typename type>
struct vk_delete_list_entry {
    u64  LastUsedFrameIndex;
    type Resource;
};

template <typename type>
struct vk_delete_list {
    array<vk_delete_list_entry<type>> List;

    vk_delete_list_entry<type>* begin() { return List.begin(); }
    vk_delete_list_entry<type>* end() { return List.end(); }
};

#define VK_UPLOAD_BUFFER_MINIMUM_BLOCK_SIZE MB(4)
struct vk_upload_buffer_block {
    VkBuffer                Buffer;
    vk_allocation           Allocation;
    buffer                  Data;
    uptr                    Used;
    vk_upload_buffer_block* Next;
};

struct vk_upload_buffer {
    arena*                  Arena;
    VkDevice                Device;
    VkAllocationCallbacks*  VKAllocator;
    vk_memory_manager*      MemoryManager;
    vk_upload_buffer_block* First;
    vk_upload_buffer_block* Last;
    vk_upload_buffer_block* Current;
};

struct vk_copy_upload_to_buffer {
    vk_upload            Upload;
    vk_handle<vk_buffer> Buffer;
    VkDeviceSize         Offset;
};

struct vk_copy_uploads_to_texture {
    vk_upload              Upload;
    fixed_array<uptr>      Offsets;
    fixed_array<vk_region> Regions;
    vk_handle<vk_texture>  Texture;
};

struct vk_copy_context {
    ak_rw_lock                        RWLock;
    u32                               CurrentListIndex;
    arena*                            Arenas[2];
    array<vk_copy_upload_to_buffer>   CopyUploadToBufferList[2];
    array<vk_copy_uploads_to_texture> CopyUploadsToTextureList[2];
};

struct vk_delete_context {
    ak_rw_lock                           RWLock;
    u32                                  CurrentListIndex;
    vk_delete_list<vk_pipeline>          PipelineList[2];
    vk_delete_list<vk_bind_group>        BindGroupList[2];
    vk_delete_list<vk_bind_group_layout> BindGroupLayoutList[2];
    vk_delete_list<vk_framebuffer>       FramebufferList[2];
    vk_delete_list<vk_render_pass>       RenderPassList[2];
    vk_delete_list<vk_sampler>           SamplerList[2];
    vk_delete_list<vk_texture_view>      TextureViewList[2];
    vk_delete_list<vk_texture>           TextureList[2];
    vk_delete_list<vk_buffer>            BufferList[2];
    vk_delete_list<vk_swapchain>         SwapchainList[2];
};

struct vk_thread_context {
    arena*                        Arena;
    vk_delete_context             DeleteContext;
    vk_copy_context               CopyContext;
    fixed_array<vk_upload_buffer> UploadBuffers; //One upload buffer per frame per thread
    fixed_array<vk_cmd_pool>      CmdPools; //One command per frame per thread
    vk_thread_context*            Next;
};

struct vk_thread_context_manager {
    gdi_context*  Context;
    ak_mutex      Lock;
    arena*        Arena;
    ak_atomic_ptr List;
    ak_tls        TLS;
};

void VK_Cmd_Storage_Free_All(vk_cmd_storage_list* StorageList);
vk_cmd_pool* VK_Get_Current_Cmd_Pool(vk_thread_context_manager* Manager, vk_thread_context* ThreadContext);
u8* VK_Upload_Buffer_Push(vk_upload_buffer* UploadBuffer, size_t Size, vk_upload* Upload);
vk_upload_buffer* VK_Get_Current_Upload_Buffer(vk_thread_context_manager* Manager, vk_thread_context* ThreadContext);
void VK_Copy_Context_Add_Upload_To_Buffer_Copy(vk_copy_context* CopyContext, const vk_copy_upload_to_buffer& CopyUploadToBuffer);
void VK_Copy_Context_Add_Uploads_To_Texture_Copy(vk_copy_context* CopyContext, vk_handle<vk_texture> Texture, vk_upload Upload, span<uptr> Offsets, span<vk_region> Regions);
vk_copy_context*   VK_Get_Copy_Context(vk_thread_context_manager* Manager);
vk_delete_context* VK_Get_Delete_Context(vk_thread_context_manager* Manager);
vk_thread_context* VK_Get_Thread_Context(vk_thread_context_manager* Manager);
void VK_Thread_Context_Manager_Create(gdi_context* Context, vk_thread_context_manager* Manager);
void VK_Thread_Context_Manager_Delete(vk_thread_context_manager* Manager);
void VK_Thread_Context_Manager_Copy_Data(vk_thread_context_manager* Manager);
void VK_Thread_Context_Manager_New_Frame(vk_thread_context_manager* Manager);


inline void VK_Copy_Context_Add_Upload_To_Texture_Copy(vk_copy_context* CopyContext, vk_handle<vk_texture> Texture, vk_upload Upload, vk_region Region) {
    VK_Copy_Context_Add_Uploads_To_Texture_Copy(CopyContext, Texture, Upload, {0}, {Region});
}

template <typename type>
inline internal void VK_Delete_List_Add(vk_delete_list<type>* List, type* Resource, u64 LastUsedFrameIndex) {
    Array_Push(&List->List, {
        .LastUsedFrameIndex = LastUsedFrameIndex,
        .Resource = *Resource
    });
}

template <typename type>
inline void VK_Delete_Context_Add(vk_delete_context* DeleteContext, vk_delete_list<type>* Lists, type* Resource, u64 LastUsedFrameIndex) {
    AK_RW_Lock_Reader(&DeleteContext->RWLock);
    u32 ListIndex = DeleteContext->CurrentListIndex;
    VK_Delete_List_Add(&Lists[ListIndex], Resource, LastUsedFrameIndex);
    AK_RW_Unlock_Reader(&DeleteContext->RWLock);
}

#endif