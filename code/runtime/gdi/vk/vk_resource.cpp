internal vk_resource_id VK_Resource_Make_ID(u32 Index, u32 Key) {
    vk_resource_internal_id Result;
    Result.Index = Index;
    Result.Generation = Key;
    return Result.ID;
}

internal ak_job_status VK_Resource_Allocate_Job_Internal(ak_job_system* JobSystem, ak_job_id JobID, vk_resource_manager* Manager, vk_resource_id _ResourceID, void* AllocateData) {
    vk_resource_internal_id ResourceID = {_ResourceID};
    vk_resource_state* ResourceState = &Manager->ResourceStates[ResourceID.Index];

    for(;;) {
        //First transition the resource state to process atomically
        if(AK_Atomic_Compare_Exchange_Bool_U32_Explicit(&ResourceState->State, VK_RESOURCE_STATE_NONE, VK_RESOURCE_STATE_PROCESSING, AK_ATOMIC_MEMORY_ORDER_ACQUIRE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
            //Check if this is a newer entry. If not this job can be removed without 
            //processing 
            //confirm: Not sure the above is the right behavior. This might be more performant
            //but we are losing data

            if(ResourceID.Generation >= AK_Atomic_Load_U32(&ResourceState->AllocatedGeneration, AK_ATOMIC_MEMORY_ORDER_ACQUIRE)) {
                void* ResourceData = (((u8*)Manager->ResourceData) + (ResourceID.Index*Manager->DataSize));

                vk_resource_result Result = Manager->AllocateCallback(Manager, ResourceID.ID, ResourceData, AllocateData);
                switch(Result) {
                    case VK_RESOURCE_RESULT_SUCCESS: {
                        AK_Atomic_Store_U32(&ResourceState->AllocatedGeneration, ResourceID.Generation, AK_ATOMIC_MEMORY_ORDER_RELEASE);
                        AK_Atomic_Store_U32(&ResourceState->State, VK_RESOURCE_STATE_ALLOCATED, AK_ATOMIC_MEMORY_ORDER_RELEASE);
                        AK_Auto_Reset_Event_Signal(&ResourceState->AllocatedStateTransitionEvent);
                        return AK_JOB_STATUS_COMPLETE;
                    } break;

                    case VK_RESOURCE_RESULT_FAILURE: {
                        //todo: Implement
                        Not_Implemented();
                    } break;
                }
            }

            //If we are unable to process the resource for whatever reason, reset the state back
            AK_Atomic_Store_U32(&ResourceState->State, VK_RESOURCE_STATE_NONE, AK_ATOMIC_MEMORY_ORDER_RELEASE);
            AK_Auto_Reset_Event_Signal(&ResourceState->NoneStateTransitionEvent);
        }

        if(ResourceID.Generation < AK_Atomic_Load_U32_Relaxed(&ResourceState->AllocatedGeneration)) {
            return AK_JOB_STATUS_COMPLETE;
        }

        //If the job cannot be transitioned to a processing state but 
        //its not older than the latest allocated state, we will wait on
        //an event to reprocess the job again once it has been transitioned back 
        //to a none state
        AK_Job_System_Begin_Job_Wait(JobSystem, JobID);
        AK_Auto_Reset_Event_Wait(&ResourceState->NoneStateTransitionEvent);
        AK_Job_System_End_Job_Wait(JobSystem, JobID);
    }
}

internal ak_job_status VK_Resource_Free_Job_Internal(ak_job_system* JobSystem, ak_job_id JobID, vk_resource_manager* Manager, vk_resource_id _ResourceID, void* FreeData) {
    vk_resource_internal_id ResourceID = {_ResourceID};
    vk_resource_state* ResourceState = &Manager->ResourceStates[ResourceID.Index];

    for(;;) {
        //First transition the resource state to process atomically
        if(AK_Atomic_Compare_Exchange_Bool_U32_Explicit(&ResourceState->State, VK_RESOURCE_STATE_NONE, VK_RESOURCE_STATE_PROCESSING, AK_ATOMIC_MEMORY_ORDER_ACQUIRE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
            //We need the proper index that is associated with the allocated index.
            //If our allocated index is 5 we need the resource id that is 5 to free
            if(ResourceID.Generation == AK_Atomic_Load_U32(&ResourceState->AllocatedGeneration, AK_ATOMIC_MEMORY_ORDER_ACQUIRE)) {
                //Ready to delete the resource so we can now check on the last fence. 
                //If the last time the fence was used was more than the context frame 
                //count, then we have already waited for this fence
                
                u64 FrameDifference = Manager->Context->CurrentFrameIndex-ResourceState->LastUsedFrameIndex;
                if(FrameDifference < Manager->Context->Frames.Count) {
                    if(vkGetFenceStatus(Manager->Context->Device, ResourceState->LastUsedFence) == VK_NOT_READY) {
                        AK_Job_System_Begin_Job_Wait(JobSystem, JobID);
                        vkWaitForFences(Manager->Context->Device, 1, &ResourceState->LastUsedFence, VK_TRUE, (u64)-1);
                        AK_Job_System_End_Job_Wait(JobSystem, JobID);
                    }
                }
                
                void* ResourceData = (((u8*)Manager->ResourceData) + (ResourceID.Index*Manager->DataSize));
                vk_resource_result Result = Manager->AllocateCallback(Manager, ResourceID.ID, ResourceData, FreeData);
                switch(Result) {
                    case VK_RESOURCE_RESULT_SUCCESS: {
                        AK_Atomic_Store_U32(&ResourceState->State, VK_RESOURCE_STATE_NONE, AK_ATOMIC_MEMORY_ORDER_RELEASE);
                        AK_Auto_Reset_Event_Signal(&ResourceState->NoneStateTransitionEvent);
                        return AK_JOB_STATUS_COMPLETE;
                    } break;

                    case VK_RESOURCE_RESULT_FAILURE: {
                        //todo: Implement
                        Not_Implemented();
                    } break;
                }
            }

            //If we are unable to process the resource for whatever reason, reset the state back
            AK_Atomic_Store_U32(&ResourceState->State, VK_RESOURCE_STATE_ALLOCATED, AK_ATOMIC_MEMORY_ORDER_RELEASE);
            AK_Auto_Reset_Event_Signal(&ResourceState->AllocatedStateTransitionEvent);
        }

        //If we find that we have an older free job, this can just be cleared
        if(ResourceID.Generation < AK_Atomic_Load_U32_Relaxed(&ResourceState->AllocatedGeneration)) {
            return AK_JOB_STATUS_COMPLETE;
        }
 
        //If the job cannot be transitioned to a processing state but 
        //its not older than the latest allocated state, we will wait on
        //an event to reprocess the job again once it has been transitioned back 
        //to a allocated state
        AK_Job_System_Begin_Job_Wait(JobSystem, JobID);
        AK_Auto_Reset_Event_Wait(&ResourceState->AllocatedStateTransitionEvent);
        AK_Job_System_End_Job_Wait(JobSystem, JobID);
    }
}

internal AK_JOB_CALLBACK_DEFINE(VK_Resource_Allocate_Job) {
    vk_resource_allocate_context* AllocateContext = (vk_resource_allocate_context*)JobUserData;
    return VK_Resource_Allocate_Job_Internal(JobSystem, JobID, AllocateContext->Manager, AllocateContext->ResourceID, AllocateContext+1);
}

internal AK_JOB_CALLBACK_DEFINE(VK_Resource_Free_Job) {
    vk_resource_free_context* FreeContext = (vk_resource_free_context*)JobUserData;
    return VK_Resource_Free_Job_Internal(JobSystem, JobID, FreeContext->Manager, FreeContext->ResourceID, FreeContext+1);
}

void VK_Resource_Manager_Create(vk_resource_manager* ResourceManager, gdi_context* Context, const vk_resource_manager_create_info& CreateInfo) {
    Zero_Struct(ResourceManager);

    uptr AllocateSize = (sizeof(uint32_t)+sizeof(vk_resource_state)+CreateInfo.ResourceSize)*CreateInfo.MaxCount;
    uint32_t* FreeIndices = (uint32_t*)Arena_Push(Context->Arena, AllocateSize);
    vk_resource_state* ResourceStates = (vk_resource_state*)(FreeIndices+CreateInfo.MaxCount);
    void* ResourceData = (void*)(ResourceStates+CreateInfo.MaxCount);

    ResourceManager->Context = Context;
    AK_Async_Stack_Index32_Init_Raw(&ResourceManager->FreeIndices, FreeIndices, CreateInfo.MaxCount);
    Array_Init(&ResourceManager->ResourceStates, ResourceStates, CreateInfo.MaxCount);
    ResourceManager->DataSize = CreateInfo.ResourceSize;
    ResourceManager->ResourceData = ResourceData;
    ResourceManager->AllocateCallback = CreateInfo.AllocateCallback;
    ResourceManager->FreeCallback = CreateInfo.FreeCallback;
    AK_Mutex_Create(&ResourceManager->Lock);

    for(u32 i = 0; i < CreateInfo.MaxCount; i++) {
        AK_Async_Stack_Index32_Push_Sync(&ResourceManager->FreeIndices, i);
        
        vk_resource_state* ResourceState = ResourceStates + i;
        //Generations always start at 1
        AK_Atomic_Store_U32_Relaxed(&ResourceState->IDGeneration,        1);
        AK_Atomic_Store_U32_Relaxed(&ResourceState->AllocatedGeneration, 1);
        AK_Auto_Reset_Event_Create(&ResourceState->AllocatedStateTransitionEvent, 0);
        AK_Auto_Reset_Event_Create(&ResourceState->NoneStateTransitionEvent, 0);
    }
}

vk_resource_id VK_Resource_Manager_Allocate(vk_resource_manager* Manager, void* AllocateUserData, uptr UserDataSize) {
    u32 Index = AK_Async_Stack_Index32_Pop(&Manager->FreeIndices);
    if(Index == AK_ASYNC_STACK_INDEX32_INVALID) {
        return 0;
    }

    const vk_resource_state* ResourceState = &Manager->ResourceStates[Index];
    vk_resource_id Result = VK_Resource_Make_ID(Index, AK_Atomic_Load_U32_Relaxed(&ResourceState->IDGeneration));

    uptr AllocationSize = sizeof(vk_resource_allocate_context)+UserDataSize;

    scratch Scratch = Scratch_Get();
    vk_resource_allocate_context* AllocateContext = (vk_resource_allocate_context*)Scratch_Push(&Scratch, AllocationSize);
    AllocateContext->Manager = Manager;
    AllocateContext->ResourceID = Result;

    if(AllocateUserData && UserDataSize) {
        void* AllocateData = (void*)(AllocateContext+1);
        Memory_Copy(AllocateData, AllocateUserData, UserDataSize);
    }

    ak_job_data AllocResourceJobData;
    AllocResourceJobData.JobCallback = VK_Resource_Allocate_Job;
    AllocResourceJobData.Data = AllocateContext;
    AllocResourceJobData.DataByteSize = AllocationSize;

    ak_job_id JobID = AK_Job_System_Alloc_Job(Manager->Context->JobSystem, AllocResourceJobData, 0, AK_JOB_FLAG_QUEUE_IMMEDIATELY_BIT|AK_JOB_FLAG_FREE_WHEN_DONE);
    Assert(JobID);

    return Result;
}

void VK_Resource_Manager_Free(vk_resource_manager* Manager, vk_resource_id _ResourceID, void* FreeUserData, uptr UserDataSize) {
    vk_resource_internal_id ResourceID = {_ResourceID};
    vk_resource_state* ResourceState = &Manager->ResourceStates[ResourceID.Index];
    u32 NextGenerationIndex = ResourceID.Generation+1;
    Assert(NextGenerationIndex != 0);
    if(AK_Atomic_Compare_Exchange_Bool_U32_Explicit(&ResourceState->IDGeneration, ResourceID.Generation, NextGenerationIndex, AK_ATOMIC_MEMORY_ORDER_ACQUIRE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
        AK_Async_Stack_Index32_Push(&Manager->FreeIndices, ResourceID.Index);

        scratch Scratch = Scratch_Get();

        uptr AllocationSize = UserDataSize + sizeof(vk_resource_free_context);
        vk_resource_free_context* FreeContext = (vk_resource_free_context*)Scratch_Push(&Scratch, AllocationSize);
        FreeContext->Manager = Manager;
        FreeContext->ResourceID = ResourceID.ID;

        if(FreeUserData && UserDataSize) {
            void* FreeData = (void*)(FreeContext+1);
            Memory_Copy(FreeData, FreeUserData, UserDataSize);
        }

        ak_job_data FreeResourceJobData;
        FreeResourceJobData.JobCallback = VK_Resource_Free_Job;
        FreeResourceJobData.Data = FreeContext;
        FreeResourceJobData.DataByteSize = AllocationSize;

        ak_job_id JobID = AK_Job_System_Alloc_Job(Manager->Context->JobSystem, FreeResourceJobData, 0, AK_JOB_FLAG_QUEUE_IMMEDIATELY_BIT|AK_JOB_FLAG_FREE_WHEN_DONE);
        Assert(JobID);
    } else {
        Invalid_Code();
    }
}

void* VK_Resource_Manager_Get_Resource(vk_resource_manager* Manager, vk_resource_id _ResourceID) {
    vk_resource_internal_id ResourceID = {_ResourceID};
    vk_resource_state* ResourceState = &Manager->ResourceStates[ResourceID.Index];

    //Check if the resource id is the latest resource id
    if(ResourceID.Generation == AK_Atomic_Load_U32_Relaxed(&ResourceState->IDGeneration)) {
        //Check to make sure the resource id has been allocated
        if(ResourceID.Generation == AK_Atomic_Load_U32_Relaxed(&ResourceState->AllocatedGeneration)) {
            vk_frame_context* FrameContext    = VK_Get_Current_Frame_Context(Manager->Context);
            ResourceState->LastUsedFence      = FrameContext->Fence;
            ResourceState->LastUsedFrameIndex = Manager->Context->CurrentFrameIndex;

            void* ResourceData = (((u8*)Manager->ResourceData) + (ResourceID.Index*Manager->DataSize));
            return ResourceData;
        }
    }

    return NULL;
}
