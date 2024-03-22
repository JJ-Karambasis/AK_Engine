#ifndef VK_RESOURCE_H
#define VK_RESOURCE_H

typedef u64 vk_resource_id;

union vk_resource_internal_id {
    vk_resource_id ID;
    struct {
        u32 Index;
        u32 Generation;
    };
};

enum {
    VK_RESOURCE_STATE_NONE,
    VK_RESOURCE_STATE_PROCESSING,
    VK_RESOURCE_STATE_ALLOCATED
};

struct vk_resource_state {
    ak_atomic_u32 IDGeneration; //Increments everytime Resource_Manager_Free_Resource() is called
    ak_atomic_u32 AllocatedGeneration; //Increments everytime the allocated job has completed
    ak_atomic_u32 State; //See the above enum for resource states, only used for the jobs
    
    ak_auto_reset_event AllocatedStateTransitionEvent;
    ak_auto_reset_event NoneStateTransitionEvent;
    
    VkFence LastUsedFence;
    u64     LastUsedFrameIndex;
};

struct vk_resource_manager;

enum vk_resource_result {
    VK_RESOURCE_RESULT_SUCCESS,
    VK_RESOURCE_RESULT_WAIT,
    VK_RESOURCE_RESULT_FAILURE
};

#define VK_RESOURCE_ALLOCATE_CALLBACK_DEFINE(name) vk_resource_result name(vk_resource_manager* Manager, vk_resource_id ResourceID, void* ResourceData, void* UserData)
#define VK_RESOURCE_FREE_CALLBACK_DEFINE(name) vk_resource_result name(vk_resource_manager* Manager, vk_resource_id ResourceID, void* ResourceData, void* UserData) 

typedef VK_RESOURCE_ALLOCATE_CALLBACK_DEFINE(vk_resource_allocate_callback_func);
typedef VK_RESOURCE_FREE_CALLBACK_DEFINE(vk_resource_free_callback_func);

struct vk_resource_manager {
    gdi_context*                        Context;
    ak_async_stack_index32              FreeIndices;   
    fixed_array<vk_resource_state>      ResourceStates;
    uptr                                DataSize;
    void*                               ResourceData;
    vk_resource_allocate_callback_func* AllocateCallback;
    vk_resource_free_callback_func*     FreeCallback;
    ak_mutex                            Lock;
};

struct vk_resource_allocate_context {
    vk_resource_manager* Manager;
    vk_resource_id       ResourceID;
};

struct vk_resource_free_context {
    vk_resource_manager* Manager;
    vk_resource_id       ResourceID;
};

struct vk_resource_manager_create_info {
    uptr                                ResourceSize;
    u32                                 MaxCount;
    vk_resource_allocate_callback_func* AllocateCallback;
    vk_resource_free_callback_func*     FreeCallback;
};

void           VK_Resource_Manager_Create(vk_resource_manager* ResourceManager, gdi_context* Context, const vk_resource_manager_create_info& CreateInfo);
vk_resource_id VK_Resource_Manager_Allocate(vk_resource_manager* Manager, void* AllocateUserData, uptr UserDataSize);
void           VK_Resource_Manager_Free(vk_resource_manager* Manager, vk_resource_id ResourceID, void* FreeUserData, uptr UserDataSize);
void*          VK_Resource_Manager_Get_Resource(vk_resource_manager* Manager, vk_resource_id ResourceID);

#define VK_Resource_Get_Key(id) (((resource_internal_id*)&id)->Generation)
#define VK_Resource_Get_Index(id) (((resource_internal_id*)&id)->Index)

#endif