#ifndef RESOURCE_H
#define RESOURCE_H

typedef u64 resource_id;

union resource_internal_id {
    resource_id ID;
    struct {
        u32 Index;
        u32 Generation;
    };
};

enum {
    RESOURCE_STATE_NONE,
    RESOURCE_STATE_PROCESSING,
    RESOURCE_STATE_ALLOCATED
};

struct resource_state {
    ak_atomic_u32 IDGeneration; //Increments everytime Resource_Manager_Free_Resource() is called
    ak_atomic_u32 AllocatedGeneration; //Increments everytime the allocated job has completed
    ak_atomic_u32 State; //See the above enum for resource states, only used for the jobs
    ak_atomic_u32 JobCount; //In order for the resource to be used, no jobs should be executing for that resource
};

struct resource_manager;

enum resource_result {
    RESOURCE_RESULT_SUCCESS,
    RESOURCE_RESULT_WAIT,
    RESOURCE_RESULT_FAILURE
};

#define RESOURCE_ALLOCATE_CALLBACK_DEFINE(name) resource_result name(ak_job_queue* JobQueue, ak_job_id JobID, resource_manager* Manager, resource_id ResourceID, void* ResourceData, void* UserData)
#define RESOURCE_FREE_CALLBACK_DEFINE(name) resource_result name(ak_job_queue* JobQueue, ak_job_id JobID, resource_manager* Manager, resource_id ResourceID, void* ResourceData, void* UserData) 
#define RESOURCE_GET_CALLBACK_DEFINE(name) void name(resource_manager* Manager, resource_id ResourceID, void* ResourceData)

typedef RESOURCE_ALLOCATE_CALLBACK_DEFINE(resource_allocate_callback_func);
typedef RESOURCE_FREE_CALLBACK_DEFINE(resource_free_callback_func);
typedef RESOURCE_GET_CALLBACK_DEFINE(resource_get_callback_func);

struct resource_manager {
    ak_job_queue*                    JobQueue;
    ak_async_stack_index32           FreeIndices;   
    fixed_array<resource_state>      ResourceStates;
    uptr                             DataSize;
    void*                            ResourceData;
    resource_allocate_callback_func* AllocateCallback;
    resource_free_callback_func*     FreeCallback;
    resource_get_callback_func*      GetCallback;
    ak_mutex                         Lock;
};

struct resource_allocate_context {
    resource_manager* Manager;
    resource_id       ResourceID;
};

struct resource_free_context {
    resource_manager* Manager;
    resource_id       ResourceID;
};

struct resource_manager_create_info {
    ak_job_queue*                    JobQueue;
    uptr                             ResourceSize;
    u32                              MaxCount;
    resource_allocate_callback_func* AllocateCallback;
    resource_free_callback_func*     FreeCallback;
    resource_get_callback_func*      GetCallback;
};

void        Resource_Manager_Create(resource_manager* ResourceManager, arena* Arena, const resource_manager_create_info& CreateInfo);
resource_id Resource_Manager_Allocate_ID(resource_manager* Manager);
void        Resource_Manager_Free_ID(resource_manager* Manager, resource_id ResourceID);
resource_id Resource_Manager_Allocate(resource_manager* Manager, const void* AllocateUserData, uptr UserDataSize);
void        Resource_Manager_Free(resource_manager* Manager, resource_id ResourceID, const void* FreeUserData, uptr UserDataSize);
void        Resource_Manager_Dispatch_Allocate(resource_manager* Manager, resource_id ResourceID, const void* AllocateUserData, uptr UserDataSize);
void        Resource_Manager_Dispatch_Free(resource_manager* Manager, resource_id ResourceID, const void* FreeUserData, uptr UserDataSize);
void*       Resource_Manager_Get_Resource(resource_manager* Manager, resource_id ResourceID);
void*       Resource_Manager_Get_Resource_No_Callback(resource_manager* Manager, resource_id ResourceID);
void*       Resource_Manager_Get_Resource_Unsafe(resource_manager* Manager, resource_id ResourceID);
void        Resource_Manager_Set_Allocated_State(resource_manager* Manager, resource_id ResourceID);

#define Resource_Get_Key(id) (((resource_internal_id*)&id)->Generation)
#define Resource_Get_Index(id) (((resource_internal_id*)&id)->Index)

#endif