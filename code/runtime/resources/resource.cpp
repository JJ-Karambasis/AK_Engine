internal resource_id Resource_Make_ID(u32 Index, u32 Key) {
    resource_internal_id Result;
    Result.Index = Index;
    Result.Generation = Key;
    return Result.ID;
}

internal ak_job_status Resource_Allocate_Job_Internal(ak_job_queue* JobQueue, ak_job_id JobID, resource_manager* Manager, resource_id _ResourceID, void* AllocateData) {
    resource_internal_id ResourceID = {_ResourceID};
    resource_state* ResourceState = &Manager->ResourceStates[ResourceID.Index];

    //First transition the resource state to process atomically
    if(AK_Atomic_Compare_Exchange_Bool_U32_Explicit(&ResourceState->State, RESOURCE_STATE_NONE, RESOURCE_STATE_PROCESSING, AK_ATOMIC_MEMORY_ORDER_ACQUIRE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
        //Check if this is a newer entry. If not this job can be removed without 
        //processing 
        //confirm: Not sure the above is the right behavior. This might be more performant
        //but we are losing data

        if(ResourceID.Generation >= AK_Atomic_Load_U32(&ResourceState->AllocatedGeneration, AK_ATOMIC_MEMORY_ORDER_ACQUIRE)) {
            void* ResourceData = (((u8*)Manager->ResourceData) + (ResourceID.Index*Manager->DataSize));

            resource_result Result = Manager->AllocateCallback(JobQueue, JobID, Manager, ResourceID.ID, ResourceData, AllocateData);
            switch(Result) {
                case RESOURCE_RESULT_SUCCESS: {
                    Resource_Manager_Set_Allocated_State(Manager, _ResourceID);
                    return AK_JOB_STATUS_COMPLETE;
                } break;

                case RESOURCE_RESULT_FAILURE: {
                    //todo: Implement
                    Not_Implemented();
                } break;
            }
        }

        //If we are unable to process the resource for whatever reason, reset the state back
        AK_Atomic_Store_U32(&ResourceState->State, RESOURCE_STATE_NONE, AK_ATOMIC_MEMORY_ORDER_RELEASE);
    }

    if(ResourceID.Generation < AK_Atomic_Load_U32_Relaxed(&ResourceState->AllocatedGeneration)) {
        return AK_JOB_STATUS_COMPLETE;
    }

    return AK_JOB_STATUS_REQUEUE;
}

internal ak_job_status Resource_Free_Job_Internal(ak_job_queue* JobQueue, ak_job_id JobID, resource_manager* Manager, resource_id _ResourceID, void* FreeData) {
    resource_internal_id ResourceID = {_ResourceID};
    resource_state* ResourceState = &Manager->ResourceStates[ResourceID.Index];

    //First transition the resource state to process atomically
    if(AK_Atomic_Compare_Exchange_Bool_U32_Explicit(&ResourceState->State, RESOURCE_STATE_ALLOCATED, RESOURCE_STATE_PROCESSING, AK_ATOMIC_MEMORY_ORDER_ACQUIRE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
        //We need the proper index that is associated with the allocated index.
        //If our allocated index is 5 we need the resource id that is 5 to free
        if(ResourceID.Generation == AK_Atomic_Load_U32(&ResourceState->AllocatedGeneration, AK_ATOMIC_MEMORY_ORDER_ACQUIRE)) {
            
            void* ResourceData = (((u8*)Manager->ResourceData) + (ResourceID.Index*Manager->DataSize));
            resource_result Result = Manager->FreeCallback(JobQueue, JobID, Manager, ResourceID.ID, ResourceData, FreeData);
            switch(Result) {
                case RESOURCE_RESULT_SUCCESS: {
                    AK_Atomic_Store_U32(&ResourceState->State, RESOURCE_STATE_NONE, AK_ATOMIC_MEMORY_ORDER_RELEASE);
                    return AK_JOB_STATUS_COMPLETE;
                } break;

                case RESOURCE_RESULT_FAILURE: {
                    //todo: Implement
                    Not_Implemented();
                } break;
            }
        }

        //If we are unable to process the resource for whatever reason, reset the state back
        AK_Atomic_Store_U32(&ResourceState->State, RESOURCE_STATE_ALLOCATED, AK_ATOMIC_MEMORY_ORDER_RELEASE);
    }

    //If we find that we have an older free job, this can just be cleared
    if(ResourceID.Generation < AK_Atomic_Load_U32_Relaxed(&ResourceState->AllocatedGeneration)) {
        return AK_JOB_STATUS_COMPLETE;
    }

    return AK_JOB_STATUS_REQUEUE;
}

internal AK_JOB_QUEUE_CALLBACK_DEFINE(Resource_Allocate_Job) {
    resource_allocate_context* AllocateContext = (resource_allocate_context*)JobUserData;
    resource_state* ResourceState = &AllocateContext->Manager->ResourceStates[Resource_Get_Index(AllocateContext->ResourceID)];
    ak_job_status Result = Resource_Allocate_Job_Internal(JobQueue, JobID, AllocateContext->Manager, AllocateContext->ResourceID, AllocateContext+1);
    if(Result == AK_JOB_STATUS_COMPLETE) {
        AK_Atomic_Decrement_U32(&ResourceState->JobCount, AK_ATOMIC_MEMORY_ORDER_RELEASE);
    }
    return Result;
}

internal AK_JOB_QUEUE_CALLBACK_DEFINE(Resource_Free_Job) {
    resource_free_context* FreeContext = (resource_free_context*)JobUserData;
    resource_state* ResourceState = &FreeContext->Manager->ResourceStates[Resource_Get_Index(FreeContext->ResourceID)];
    ak_job_status Result = Resource_Free_Job_Internal(JobQueue, JobID, FreeContext->Manager, FreeContext->ResourceID, FreeContext+1);
    if(Result == AK_JOB_STATUS_COMPLETE) {
        AK_Atomic_Decrement_U32(&ResourceState->JobCount, AK_ATOMIC_MEMORY_ORDER_RELEASE);
    }
    return Result;
}

void Resource_Manager_Create(resource_manager* ResourceManager, arena* Arena, const resource_manager_create_info& CreateInfo) {
    Zero_Struct(ResourceManager);

    uptr AllocateSize = (sizeof(uint32_t)+sizeof(resource_state)+CreateInfo.ResourceSize)*CreateInfo.MaxCount;
    uint32_t* FreeIndices = (uint32_t*)Arena_Push(Arena, AllocateSize);
    resource_state* ResourceStates = (resource_state*)(FreeIndices+CreateInfo.MaxCount);
    void* ResourceData = (void*)(ResourceStates+CreateInfo.MaxCount);

    ResourceManager->JobQueue = CreateInfo.JobQueue;
    AK_Async_Stack_Index32_Init_Raw(&ResourceManager->FreeIndices, FreeIndices, CreateInfo.MaxCount);
    Array_Init(&ResourceManager->ResourceStates, ResourceStates, CreateInfo.MaxCount);
    ResourceManager->DataSize = CreateInfo.ResourceSize;
    ResourceManager->ResourceData = ResourceData;
    ResourceManager->AllocateCallback = CreateInfo.AllocateCallback;
    ResourceManager->FreeCallback = CreateInfo.FreeCallback;
    AK_Mutex_Create(&ResourceManager->Lock);

    for(u32 i = 0; i < CreateInfo.MaxCount; i++) {
        AK_Async_Stack_Index32_Push_Sync(&ResourceManager->FreeIndices, i);
        
        resource_state* ResourceState = ResourceStates + i;
        //Generations always start at 1
        AK_Atomic_Store_U32_Relaxed(&ResourceState->IDGeneration,        1);
        AK_Atomic_Store_U32_Relaxed(&ResourceState->AllocatedGeneration, 1);
    }
}

resource_id Resource_Manager_Allocate_ID(resource_manager* Manager) {
    u32 Index = AK_Async_Stack_Index32_Pop(&Manager->FreeIndices);
    if(Index == AK_ASYNC_STACK_INDEX32_INVALID) {
        return 0;
    }

    const resource_state* ResourceState = &Manager->ResourceStates[Index];
    resource_id Result = Resource_Make_ID(Index, AK_Atomic_Load_U32_Relaxed(&ResourceState->IDGeneration));
    return Result;
}

void Resource_Manager_Free_ID(resource_manager* Manager, resource_id _ResourceID) {
    resource_internal_id ResourceID = {_ResourceID};
    resource_state* ResourceState = &Manager->ResourceStates[ResourceID.Index];
    u32 NextGenerationIndex = ResourceID.Generation+1;
    Assert(NextGenerationIndex != 0);
    if(AK_Atomic_Compare_Exchange_Bool_U32_Explicit(&ResourceState->IDGeneration, ResourceID.Generation, NextGenerationIndex, AK_ATOMIC_MEMORY_ORDER_ACQUIRE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
        AK_Async_Stack_Index32_Push(&Manager->FreeIndices, ResourceID.Index);
    } else {
        Invalid_Code();
    }
}

resource_id Resource_Manager_Allocate(resource_manager* Manager, const void* AllocateUserData, uptr UserDataSize) {
    resource_id Result = Resource_Manager_Allocate_ID(Manager);
    if(!Result) {
        return 0;
    }

    Resource_Manager_Dispatch_Allocate(Manager, Result, AllocateUserData, UserDataSize);
    return Result;
}

void Resource_Manager_Free(resource_manager* Manager, resource_id ResourceID, const void* FreeUserData, uptr UserDataSize) {
    Resource_Manager_Free_ID(Manager, ResourceID);
    Resource_Manager_Dispatch_Free(Manager, ResourceID, FreeUserData, UserDataSize);
}

void Resource_Manager_Dispatch_Allocate(resource_manager* Manager, resource_id ResourceID, const void* AllocateUserData, uptr UserDataSize) {
    uptr AllocationSize = sizeof(resource_allocate_context)+UserDataSize;

    scratch Scratch = Scratch_Get();
    resource_allocate_context* AllocateContext = (resource_allocate_context*)Scratch_Push(&Scratch, AllocationSize);
    AllocateContext->Manager = Manager;
    AllocateContext->ResourceID = ResourceID;

    if(AllocateUserData && UserDataSize) {
        void* AllocateData = (void*)(AllocateContext+1);
        Memory_Copy(AllocateData, AllocateUserData, UserDataSize);
    }

    ak_job_queue_data AllocResourceJobData;
    AllocResourceJobData.JobCallback = Resource_Allocate_Job;
    AllocResourceJobData.Data = AllocateContext;
    AllocResourceJobData.DataByteSize = AllocationSize;
    
    resource_state* ResourceState = &Manager->ResourceStates[Resource_Get_Index(ResourceID)];
    AK_Atomic_Increment_U32(&ResourceState->JobCount, AK_ATOMIC_MEMORY_ORDER_RELEASE);

    ak_job_id JobID = AK_Job_Queue_Alloc_Job(Manager->JobQueue, AllocResourceJobData, 0, AK_JOB_FLAG_QUEUE_IMMEDIATELY_BIT|AK_JOB_FLAG_FREE_WHEN_DONE);
    Assert(JobID);
}

void Resource_Manager_Dispatch_Free(resource_manager* Manager, resource_id ResourceID, const void* FreeUserData, uptr UserDataSize) {
    scratch Scratch = Scratch_Get();

    resource_state* ResourceState = &Manager->ResourceStates[Resource_Get_Index(ResourceID)];

    uptr AllocationSize = UserDataSize + sizeof(resource_free_context);
    resource_free_context* FreeContext = (resource_free_context*)Scratch_Push(&Scratch, AllocationSize);
    FreeContext->Manager = Manager;
    FreeContext->ResourceID = ResourceID;

    if(FreeUserData && UserDataSize) {
        void* FreeData = (void*)(FreeContext+1);
        Memory_Copy(FreeData, FreeUserData, UserDataSize);
    }

    ak_job_queue_data FreeResourceJobData;
    FreeResourceJobData.JobCallback = Resource_Free_Job;
    FreeResourceJobData.Data = FreeContext;
    FreeResourceJobData.DataByteSize = AllocationSize;

    AK_Atomic_Increment_U32(&ResourceState->JobCount, AK_ATOMIC_MEMORY_ORDER_RELEASE);
    ak_job_id JobID = AK_Job_Queue_Alloc_Job(Manager->JobQueue, FreeResourceJobData, 0, AK_JOB_FLAG_QUEUE_IMMEDIATELY_BIT|AK_JOB_FLAG_FREE_WHEN_DONE);
    Assert(JobID);
}

void* Resource_Manager_Get_Resource(resource_manager* Manager, resource_id ResourceID) {
    void* ResourceData = Resource_Manager_Get_Resource_No_Callback(Manager, ResourceID);
    if(ResourceData) {
        Manager->GetCallback(Manager, ResourceID, ResourceData);
    }
    return ResourceData;
}

void* Resource_Manager_Get_Resource_No_Callback(resource_manager* Manager, resource_id _ResourceID) {
    resource_internal_id ResourceID = {_ResourceID};
    resource_state* ResourceState = &Manager->ResourceStates[ResourceID.Index];
    if(ResourceID.Generation == AK_Atomic_Load_U32_Relaxed(&ResourceState->AllocatedGeneration) &&
       AK_Atomic_Load_U32_Relaxed(&ResourceState->State) == RESOURCE_STATE_ALLOCATED &&
       AK_Atomic_Load_U32_Relaxed(&ResourceState->JobCount) == 0) {
        void* ResourceData = Resource_Manager_Get_Resource_Unsafe(Manager, _ResourceID);
        return ResourceData;
    }
    return NULL;
}

void* Resource_Manager_Get_Resource_Unsafe(resource_manager* Manager, resource_id _ResourceID) {
    resource_internal_id ResourceID = {_ResourceID};
    resource_state* ResourceState = &Manager->ResourceStates[ResourceID.Index];
    if(ResourceID.Generation == AK_Atomic_Load_U32_Relaxed(&ResourceState->IDGeneration)) {
        void* ResourceData = (((u8*)Manager->ResourceData) + (ResourceID.Index*Manager->DataSize));
        return ResourceData;
    }
    return NULL;
}

void Resource_Manager_Set_Allocated_State(resource_manager* Manager, resource_id ResourceID) {
    resource_state* ResourceState = &Manager->ResourceStates[Resource_Get_Index(ResourceID)];
    AK_Atomic_Store_U32(&ResourceState->AllocatedGeneration, Resource_Get_Key(ResourceID), AK_ATOMIC_MEMORY_ORDER_RELEASE);
    AK_Atomic_Store_U32(&ResourceState->State, RESOURCE_STATE_ALLOCATED, AK_ATOMIC_MEMORY_ORDER_RELEASE);
}