void QSBR_Update() {
    thread_context* ThreadContext = Thread_Context_Get();
    AK_QSBR_Update(Core_Get()->ThreadManager->QSBR, ThreadContext->QSBRContext);
}

internal void Thread_Pool__Init(thread_pool* ThreadPool) {
    Zero_Struct(ThreadPool);
    ThreadPool->FirstFreeIndex = (u32)-1;
    
    for(u32 i = 0; i < MAX_THREAD_COUNT; i++) {
        ThreadPool->Threads[i].PoolIndex = (u32)-1;
        ThreadPool->Threads[i].PoolNextIndex = (u32)-1;
    }
}

internal thread_context* Thread_Pool__Allocate_Context(thread_pool* Pool) {
    u32 Index;
    if(Pool->FirstFreeIndex != (u32)-1) {
        Index = Pool->FirstFreeIndex;
        Pool->FirstFreeIndex = Pool->Threads[Index].PoolNextIndex;
    } else {
        Assert(Pool->MaxUsed < MAX_THREAD_COUNT);
        Index = Pool->MaxUsed++;
    }

    thread_context* ThreadContext = Pool->Threads + Index;
    Zero_Struct(ThreadContext);
    
    ThreadContext->PoolNextIndex = (u32)-1;
    ThreadContext->PoolIndex = Index;

    
    
    return ThreadContext;
}

internal void Thread_Pool__Delete_Context(thread_pool* Pool, thread_context* ThreadContext) {
    ThreadContext->PoolNextIndex = Pool->FirstFreeIndex;
    Pool->FirstFreeIndex = ThreadContext->PoolIndex;
    ThreadContext->PoolIndex = (u32)-1;
}

internal void Thread_Map__Add(thread_map* ThreadMap, u64 ThreadID, thread_context* Context) {
    u32 Hash = Hash_U64(ThreadID);
    u32 SlotIndex = Hash % MAX_THREAD_COUNT;
    thread_slot* Slot = ThreadMap->Slots + SlotIndex;
    DLL_Push_Back_NP(Slot->SlotHead, Slot->SlotTail, Context, HashNext, HashPrev);
}

internal void Thread_Map__Remove(thread_map* ThreadMap, u64 ThreadID) {
    u32 Hash = Hash_U64(ThreadID);
    u32 SlotIndex = Hash % MAX_THREAD_COUNT;
    thread_slot* Slot = ThreadMap->Slots + SlotIndex;

    thread_context* TargetThread = NULL;
    for(thread_context* Thread = Slot->SlotHead; Thread; Thread = Thread->HashNext) {
        if(Thread->ThreadID == ThreadID) {
            TargetThread = Thread;
            break;
        }
    }

    if(TargetThread) {
        DLL_Remove_NP(Slot->SlotHead, Slot->SlotTail, TargetThread, HashNext, HashPrev);
    }
}

internal thread_context* Thread_Map__Get(thread_map* ThreadMap, u64 ThreadID) {
    u32 Hash = Hash_U64(ThreadID);
    u32 SlotIndex = Hash % MAX_THREAD_COUNT;
    thread_slot* Slot = ThreadMap->Slots + SlotIndex; 

    for(thread_context* Thread = Slot->SlotHead; Thread; Thread = Thread->HashNext) {
        if(Thread->ThreadID == ThreadID) {
            return Thread;
        }
    }

    return NULL;
}

thread_context* Thread_Manager__Create_Raw_Context(thread_manager* ThreadManager, u64 ThreadID) {
    AK_Mutex_Lock(&ThreadManager->AllocateLock);
    thread_context* Result = Thread_Pool__Allocate_Context(&ThreadManager->ThreadPool);
    AK_Mutex_Unlock(&ThreadManager->AllocateLock);
    
    Result->ThreadID = ThreadID;
    Result->QSBRContext = AK_QSBR_Create_Context(ThreadManager->QSBR);

    AK_Mutex_Lock(&ThreadManager->MapLock);
    Thread_Map__Add(&ThreadManager->ThreadMap, Result->ThreadID, Result);
    AK_Mutex_Unlock(&ThreadManager->MapLock);

    return Result;
}

thread_context* Thread_Manager__Get_Thread_Context() {
    thread_manager* ThreadManager = Thread_Manager_Get();
    if(!ThreadManager) return NULL;

    thread_context* ThreadContext = (thread_context*)AK_TLS_Get(&ThreadManager->ThreadContextLocalStorage);
    if(ThreadContext) return ThreadContext;

    AK_Mutex_Lock(&ThreadManager->MapLock);
    ThreadContext = Thread_Map__Get(&ThreadManager->ThreadMap, AK_Thread_Get_Current_ID());
    AK_Mutex_Unlock(&ThreadManager->MapLock);

    if(!ThreadContext) {
        ThreadContext = Thread_Manager__Create_Raw_Context(ThreadManager, AK_Thread_Get_Current_ID());
    }

    AK_TLS_Set(&ThreadManager->ThreadContextLocalStorage, ThreadContext);

    return ThreadContext;
}

void Thread_Context_Delete(thread_context* ThreadContext) {
    thread_manager* ThreadManager = Thread_Manager_Get();

    AK_Thread_Wait(&ThreadContext->Thread);
    AK_Thread_Delete(&ThreadContext->Thread);

    for(u32 ScratchIndex = 0; ScratchIndex < MAX_SCRATCH_COUNT; ScratchIndex++) {
        if(ThreadContext->ScratchPool[ScratchIndex]) {
            Arena_Delete(ThreadContext->ScratchPool[ScratchIndex]);
            ThreadContext->ScratchPool[ScratchIndex] = NULL;
        }
    }

    AK_QSBR_Delete_Context(ThreadManager->QSBR, ThreadContext->QSBRContext);
    ThreadContext->QSBRContext = 0;

    AK_Mutex_Lock(&ThreadManager->MapLock);
    Thread_Map__Remove(&ThreadManager->ThreadMap, ThreadContext->ThreadID);
    AK_Mutex_Unlock(&ThreadManager->MapLock);

    AK_Mutex_Lock(&ThreadManager->AllocateLock);
    Thread_Pool__Delete_Context(&ThreadManager->ThreadPool, ThreadContext);
    AK_Mutex_Unlock(&ThreadManager->AllocateLock);
}

void Thread_Context_Wait(thread_context* ThreadContext) {
    AK_Thread_Wait(&ThreadContext->Thread);
}

thread_context* Thread_Context_Get() {
    return Thread_Manager__Get_Thread_Context();
}

internal ALLOCATOR_ALLOCATE_MEMORY_DEFINE(Scratch_Allocate_Memory) {
    scratch* Scratch = (scratch*)Allocator;
    return Scratch_Push(Scratch, Size, 1, ALLOCATOR_CLEAR_FLAG_NO_CLEAR);
}

internal ALLOCATOR_FREE_MEMORY_DEFINE(Scratch_Free_Memory) {
    //noop
}

internal ALLOCATOR_GET_STATS_DEFINE(Scratch_Get_Stats) {
    Not_Implemented();
    return {};
}

global allocator_vtable G_ScratchVTable = {
    Scratch_Allocate_Memory,
    Scratch_Free_Memory,
    Scratch_Get_Stats
};

scratch::scratch(arena* _Arena) : Arena(_Arena) { 
    VTable = &G_ScratchVTable;
}

scratch::~scratch() {
    if(VTable) {
        Scratch_Release();
        VTable = NULL;
    }
}


scratch Scratch_Get() {
    thread_context* ThreadContext = Thread_Context_Get();
    Assert(ThreadContext->CurrentScratchIndex < MAX_SCRATCH_COUNT);
    u32 ScratchIndex = ThreadContext->CurrentScratchIndex++;
    if(!ThreadContext->ScratchPool[ScratchIndex]) {
        allocator* VirtualAllocator = (allocator*)Core_Get()->VirtualAllocator;
        string ArenaName(VirtualAllocator, "Thread %llu Scratch Index %d", ThreadContext->ThreadID, ScratchIndex+1);
        ThreadContext->ScratchPool[ScratchIndex] = Arena_Create(VirtualAllocator, (uptr)-1);
        Arena_Track(ThreadContext->ScratchPool[ScratchIndex], ArenaName);
        String_Free(VirtualAllocator, ArenaName);
    }
    ThreadContext->ScratchMarkers[ScratchIndex] = Arena_Get_Marker(ThreadContext->ScratchPool[ScratchIndex]);
    return scratch(ThreadContext->ScratchPool[ScratchIndex]);
}

void Scratch_Release() {
    thread_context* ThreadContext = Thread_Context_Get();
    Assert(ThreadContext->CurrentScratchIndex > 0);
    u32 ScratchIndex = --ThreadContext->CurrentScratchIndex;
    Arena_Set_Marker(ThreadContext->ScratchPool[ScratchIndex], &ThreadContext->ScratchMarkers[ScratchIndex]);
}

void* Scratch_Push(scratch* Scratch, uptr Size, uptr Alignment, allocator_clear_flag ClearFlag) {
    void* Result = Arena_Push(Scratch->Arena, Size, Alignment, ALLOCATOR_CLEAR_FLAG_NO_CLEAR);
    if(ClearFlag == ALLOCATOR_CLEAR_FLAG_CLEAR) {
        Memory_Clear(Result, Size);
    }
    return Result;
}

thread_manager* Thread_Manager_Create() {
    allocator* Allocator = Core_Get_Base_Allocator();
    thread_manager* ThreadManager = Allocator_Allocate_Struct(Allocator, thread_manager);
    Zero_Struct(ThreadManager);

    AK_Mutex_Create(&ThreadManager->AllocateLock);
    AK_Mutex_Create(&ThreadManager->MapLock);
    Thread_Pool__Init(&ThreadManager->ThreadPool);
    AK_TLS_Create(&ThreadManager->ThreadContextLocalStorage);
    ThreadManager->QSBR = AK_QSBR_Create(ThreadManager->QSBR);

    ThreadManager->MainThreadContext = Thread_Manager__Create_Raw_Context(ThreadManager, AK_Thread_Get_Current_ID());

    Thread_Manager_Set(ThreadManager);
    return ThreadManager;
}

void Thread_Manager_Delete() {
    thread_manager* ThreadManager = Thread_Manager_Get();
    if(ThreadManager) {
        AK_TLS_Delete(&ThreadManager->ThreadContextLocalStorage);

        for(u32 ThreadIndex = 0; ThreadIndex < MAX_THREAD_COUNT; ThreadIndex++) {
            thread_context* ThreadContext = ThreadManager->ThreadPool.Threads + ThreadIndex;
            if(ThreadContext->PoolIndex != (u32)-1) {
                Thread_Context_Delete(ThreadContext);
            }
        }

        AK_QSBR_Delete(ThreadManager->QSBR);
        AK_Mutex_Delete(&ThreadManager->AllocateLock);
        AK_Mutex_Delete(&ThreadManager->MapLock);

        allocator* Allocator = Core_Get_Base_Allocator();
        Allocator_Free_Memory(Allocator, ThreadManager);
        Thread_Manager_Set(NULL);
    }
}

void Thread_Manager_Wait_All() {
    thread_manager* ThreadManager = Thread_Manager_Get();
    if(ThreadManager) {
        for(u32 ThreadIndex = 0; ThreadIndex < MAX_THREAD_COUNT; ThreadIndex++) {
            thread_context* ThreadContext = ThreadManager->ThreadPool.Threads + ThreadIndex;
            if(ThreadContext->PoolIndex != (u32)-1) {
                Thread_Context_Wait(ThreadContext);
            }
        }
    }
}

internal AK_THREAD_CALLBACK_DEFINE(Thread_Manager__Thread_Callback) {
    thread_context* ThreadContext = (thread_context*)UserData;
    thread_manager* ThreadManager = Thread_Manager_Get();

    ThreadContext->ThreadID = AK_Thread_Get_Current_ID();

    AK_Mutex_Lock(&ThreadManager->MapLock);
    Thread_Map__Add(&ThreadManager->ThreadMap, ThreadContext->ThreadID, ThreadContext);
    AK_Mutex_Unlock(&ThreadManager->MapLock);

    u32 Result = ThreadContext->Callback(ThreadContext, ThreadContext->CallbackUserData);

    AK_Mutex_Lock(&ThreadManager->MapLock);
    Thread_Map__Remove(&ThreadManager->ThreadMap, ThreadContext->ThreadID);
    AK_Mutex_Unlock(&ThreadManager->MapLock);

    return (s32)Result;
}

thread_context* Thread_Manager_Create_Thread(thread_context_callback* Callback, void* UserData) {
    thread_manager* ThreadManager = Thread_Manager_Get();
    if(!ThreadManager) return NULL;

    AK_Mutex_Lock(&ThreadManager->AllocateLock);
    thread_context* Result = Thread_Pool__Allocate_Context(&ThreadManager->ThreadPool);
    AK_Mutex_Unlock(&ThreadManager->AllocateLock);

    Result->Callback = Callback;
    Result->CallbackUserData = UserData; 
    AK_Thread_Create(&Result->Thread, Thread_Manager__Thread_Callback, Result);
    return Result;
}

global thread_manager* G_ThreadManager;
thread_manager* Thread_Manager_Get() {
    Assert(G_ThreadManager);
    return G_ThreadManager;
}

void Thread_Manager_Set(thread_manager* Manager) {
    G_ThreadManager = Manager;
}