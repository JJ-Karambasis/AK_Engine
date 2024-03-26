struct thread_resource {
    resource_id   ID;
    ak_atomic_u32 Allocated;
    s32           Freed;
};

struct thread_resource_array {
    thread_resource* Ptr;
    u32              Count;
};

void Thread_Resource_Array_Alloc(thread_resource_array* Array, arena* Storage, u32 Count) {
    Array->Ptr = Arena_Push_Array(Storage, Count, thread_resource);
    Zero_Array(Array->Ptr, Count);
    Array->Count = Count;
}

struct thread_resource_data {
    resource_manager*     ResourceManager;
    thread_resource_array ResourceArray;
    u32 DuplicateCount;
};

struct resource_test_state {
    u32 AllocatedCounter;
    u32 FreeCounter;
    u32 Generation;
    s32 IsAllocated;
};

internal RESOURCE_ALLOCATE_CALLBACK_DEFINE(Resource_Allocate_Callback) {
    u32 Generation = Resource_Get_Key(ResourceID);
    resource_test_state* ResourceState = (resource_test_state*)ResourceData;
    
    //Make sure we are not already allocated
    Assert(!ResourceState->IsAllocated);
    //Make sure we are always allocated a newer resource
    Assert(ResourceState->Generation < Generation);
    
    //Accumulate counters for the final test and write resource state
    ResourceState->AllocatedCounter++;
    ResourceState->Generation = Generation;
    ResourceState->IsAllocated = true;

    return RESOURCE_RESULT_SUCCESS;
}

internal RESOURCE_FREE_CALLBACK_DEFINE(Resource_Free_Callback) {
    u32 Generation = Resource_Get_Key(ResourceID);
    resource_test_state* ResourceState = (resource_test_state*)ResourceData;
    bool IsAllocated = (bool)ResourceState->IsAllocated;

    //Make sure we are allocated
    Assert(IsAllocated);
    
    //Make sure generations match so allocations and frees match
    Assert(ResourceState->Generation == Generation);

    //Accumulate counters for the final test and write resource state
    ResourceState->FreeCounter++;
    ResourceState->IsAllocated = false;

    return RESOURCE_RESULT_SUCCESS;
}

//Thread to just create resources in a resource array
u32 Resource_Create_Thread(thread_context* ThreadContext, void* UserData) {
    
    thread_resource_data* ThreadData = (thread_resource_data*)UserData;
    resource_manager* ResourceManager = ThreadData->ResourceManager;
    thread_resource_array* ResourceArray = &ThreadData->ResourceArray;
    for(u32 i = 0; i < ResourceArray->Count; i++) {
        ResourceArray->Ptr[i].ID = Resource_Manager_Allocate(ResourceManager, NULL, 0);
        Assert(ResourceArray->Ptr[i].ID);
        AK_Atomic_Store_U32(&ResourceArray->Ptr[i].Allocated, true, AK_ATOMIC_MEMORY_ORDER_RELEASE);
    }

    return 0;
}

//Thread to just delete resources in a resource array
u32 Resource_Delete_Thread(thread_context* ThreadContext, void* UserData) {
    thread_resource_data* ThreadData = (thread_resource_data*)UserData;
    resource_manager* ResourceManager = ThreadData->ResourceManager;
    thread_resource_array* ResourceArray = &ThreadData->ResourceArray;
    
    bool IsDone;
    do {
        IsDone = true;
        for(u32 i = 0; i < ResourceArray->Count; i++) {
            if(AK_Atomic_Load_U32(&ResourceArray->Ptr[i].Allocated, AK_ATOMIC_MEMORY_ORDER_ACQUIRE) && !ResourceArray->Ptr[i].Freed) {
                Resource_Manager_Free(ResourceManager, ResourceArray->Ptr[i].ID, NULL, 0);
                ResourceArray->Ptr[i].ID = 0;
                ResourceArray->Ptr[i].Freed = true;
            } 
            
            if(!ResourceArray->Ptr[i].Freed) {
                IsDone = false;
            }
        }
    } while(!IsDone);

    return 0;
}

//Thread to just handle testing creating and deleting a resource array,
//ending with the array having a single allocated entry
u32 Resource_Create_Delete_Thread(thread_context* ThreadContext, void* UserData) {
    thread_resource_data* ThreadData = (thread_resource_data*)UserData;
    resource_manager* ResourceManager = ThreadData->ResourceManager;
    thread_resource_array* ResourceArray = &ThreadData->ResourceArray;

    for(u32 i = 0; i < ResourceArray->Count; i++) {
        //Make sure duplicate checks are working. Make sure the sync changes are going to be tested at some point
        for(u32 j = 0; j < ThreadData->DuplicateCount; j++) {
            resource_id ResourceID = Resource_Manager_Allocate(ResourceManager, NULL, 0);
            assert(ResourceID);
            Resource_Manager_Free(ResourceManager, ResourceID, NULL, 0);
        }

        //Then make sure some normal allocations are happening
        //These will run on the delete thread at some point
        ResourceArray->Ptr[i].ID = Resource_Manager_Allocate(ResourceManager, NULL, 0);
        assert(ResourceArray->Ptr[i].ID);
        AK_Atomic_Store_U32(&ResourceArray->Ptr[i].Allocated, true, AK_ATOMIC_MEMORY_ORDER_RELEASE);
    }

    return 0;
}

//Thread that will create and delete a resource array synchronously
u32 Resource_Sync_Thread(thread_context* ThreadContext, void* UserData) {
    thread_resource_data* ThreadData = (thread_resource_data*)UserData;
    thread_resource_array* ResourceArray = &ThreadData->ResourceArray;
    resource_manager* ResourceManager = ThreadData->ResourceManager;
    scratch Scratch = Scratch_Get();
    resource_id* Resources = Scratch_Push_Array(&Scratch, ResourceArray->Count, resource_id);
    for(u32 i = 0; i < ResourceArray->Count; i++) {
        Resources[i] = Resource_Manager_Allocate(ResourceManager, NULL, 0);
    }

    for(u32 i = 0; i < ResourceArray->Count; i++) {
        assert(Resources[i]);
        Resource_Manager_Free(ResourceManager, Resources[i], NULL, 0);
    }

    //Once more to for duplicate checks just to be safe
    for(u32 i = 0; i < ResourceArray->Count; i++) {
        for(u32 j = 0; j < ThreadData->DuplicateCount; j++) {
            resource_id ResourceID = Resource_Manager_Allocate(ResourceManager, NULL, 0);
            assert(ResourceID);
            Resource_Manager_Free(ResourceManager, ResourceID, NULL, 0);
        }
    }

    return 0;
}

UTEST(AsyncResourceTest, SingleResourceTest) {
    const u32 Iterations = 100;
    const u32 ResourceCount = 10000;

    scratch Scratch = Scratch_Get();

    ak_job_queue* JobQueue = Core_Create_Job_Queue(Iterations*ResourceCount*2, AK_Get_Processor_Thread_Count(), 0);

    resource_manager ResourceManager;
    Resource_Manager_Create(&ResourceManager, Scratch.Arena, {
        .JobQueue = JobQueue,
        .ResourceSize = sizeof(resource_test_state),
        .MaxCount = ResourceCount,
        .AllocateCallback = Resource_Allocate_Callback,
        .FreeCallback = Resource_Free_Callback
    });

    for(u32 i = 0; i < Iterations; i++) {
        for(u32 j = 0; j < ResourceCount; j++) {
            resource_id ResourceID = Resource_Manager_Allocate(&ResourceManager, NULL, 0);
            assert(ResourceID);
            Resource_Manager_Free(&ResourceManager, ResourceID, NULL, 0);
        }
    }
    Core_Delete_Job_Queue(JobQueue);

    for(u32 i = 0; i < ResourceManager.ResourceStates.Count; i++) {
        resource_test_state* TestState = (resource_test_state*)(((u8*)ResourceManager.ResourceData) + (i*ResourceManager.DataSize));
        ASSERT_EQ(TestState->AllocatedCounter, TestState->FreeCounter);
        ASSERT_FALSE(TestState->IsAllocated);
    }
}

UTEST(AsyncResourceTest, AsyncResourceTest) {
    const u32 Iterations = 2;
    const u32 ResourceCount = 100000;
    const u32 DuplicateCount = 10;

    scratch Scratch = Scratch_Get();

    ak_job_queue* JobQueue = Core_Create_Job_Queue(ResourceCount*DuplicateCount*5*Iterations, AK_Get_Processor_Thread_Count(), 0);

    resource_manager ResourceManager;
    Resource_Manager_Create(&ResourceManager, Scratch.Arena, {
        .JobQueue = JobQueue,
        .ResourceSize = sizeof(resource_test_state),
        .MaxCount = ResourceCount*Iterations*3,
        .AllocateCallback = Resource_Allocate_Callback,
        .FreeCallback = Resource_Free_Callback
    });

    thread_context** Threads = Scratch_Push_Array(&Scratch, Iterations*5, thread_context*);
    for(u32 i = 0; i < Iterations; i++) {
        thread_resource_array ResourceArray1;
        Thread_Resource_Array_Alloc(&ResourceArray1, Scratch.Arena, ResourceCount);

        thread_resource_array ResourceArray2;
        Thread_Resource_Array_Alloc(&ResourceArray2, Scratch.Arena, ResourceCount);

        thread_resource_data* ThreadData1 = Scratch_Push_Struct(&Scratch, thread_resource_data);
        ThreadData1->ResourceManager = &ResourceManager;
        ThreadData1->ResourceArray = ResourceArray1;

        thread_resource_data* ThreadData2 = Scratch_Push_Struct(&Scratch, thread_resource_data);
        ThreadData2->ResourceManager = &ResourceManager;
        ThreadData2->ResourceArray = ResourceArray2;
        ThreadData2->DuplicateCount = DuplicateCount;

        thread_resource_data* ThreadData3 = Scratch_Push_Struct(&Scratch, thread_resource_data);
        ThreadData3->ResourceManager = &ResourceManager;
        ThreadData3->ResourceArray.Count = ResourceCount;
        ThreadData3->DuplicateCount = DuplicateCount; 

        thread_context** ThreadBatch = Threads + i*5;
        ThreadBatch[0] = Thread_Manager_Create_Thread(Resource_Create_Thread, ThreadData1);
        ThreadBatch[1] = Thread_Manager_Create_Thread(Resource_Delete_Thread, ThreadData1);
        ThreadBatch[2] = Thread_Manager_Create_Thread(Resource_Create_Delete_Thread, ThreadData2);
        ThreadBatch[3] = Thread_Manager_Create_Thread(Resource_Delete_Thread, ThreadData2);
        ThreadBatch[4] = Thread_Manager_Create_Thread(Resource_Sync_Thread, ThreadData3);
    }

    for(u32 i = 0; i < Iterations*5; i++) {
        if(Threads[i]) {
            Thread_Context_Delete(Threads[i]);
        }
    }
    Core_Delete_Job_Queue(JobQueue);
    Thread_Manager_Wait_All();
    
    for(u32 i = 0; i < ResourceManager.ResourceStates.Count; i++) {
        resource_test_state* TestState = (resource_test_state*)(((u8*)ResourceManager.ResourceData) + (i*ResourceManager.DataSize));
        if(TestState->AllocatedCounter != TestState->FreeCounter) {
            int x = 0;
        }
        ASSERT_EQ(TestState->AllocatedCounter, TestState->FreeCounter);
        ASSERT_FALSE(TestState->IsAllocated);
    }
}