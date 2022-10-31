thread_manager Thread_Manager_Create()
{
    thread_manager Result;
    Zero_Struct(&Result, thread_manager);
    
    allocator* Allocator = OS_Get_Allocator();
    Result.ThreadPool = Pool_Create(Allocator, thread_context, 64);
    Result.ThreadMap = Hashmap_Create(Allocator, uint32_t, thread_context*, Hash_U32, Key_Compare_U32, 512, 64);
    Result.Lock = Async_Lock_Create(5000);
    return Result;
}

void Thread_Manager_Delete(thread_manager* ThreadManager)
{
    Thread_Manager_Wait_For_All_Threads(ThreadManager);
    
    for(pool_iter Iter = Pool_Begin_Iter(&ThreadManager->ThreadPool); Iter.IsValid; Pool_Next_Iter(&Iter))
    {
        thread_context* Thread = Pool_Get_Iter_Entry(&Iter, thread_context);
        Arena_Delete(Thread->Scratch);
        OS_Delete_Thread(Thread->Thread);
    }
    
    Hashmap_Delete(&ThreadManager->ThreadMap);
    Pool_Delete(&ThreadManager->ThreadPool);
}

OS_THREAD_CALLBACK(Thread_Manager__Thread_Callback)
{
    thread_context* ThreadContext = (thread_context*)UserData;
    thread_manager* ThreadManager = &Core_Get()->ThreadManager;
    
    uint32_t ThreadID = OS_Get_Current_Thread_ID();
    
    Async_Lock(&ThreadManager->Lock);
    Hashmap_Add(&ThreadManager->ThreadMap, &ThreadID, &ThreadContext);
    Async_Unlock(&ThreadManager->Lock);
    
    return ThreadContext->Callback(ThreadContext, ThreadContext->UserData);
}

thread_context* Thread_Manager_Create_Thread(thread_manager* ThreadManager, core_thread_callback* Callback, void* UserData)
{
    pool_handle ThreadHandle = Pool_Allocate(&ThreadManager->ThreadPool);
    thread_context* Result = Pool_Get(&ThreadManager->ThreadPool, thread_context, ThreadHandle);
    
    Result->Handle        = ThreadHandle;
    Result->MainAllocator = OS_Get_Allocator();
    Result->Scratch       = Arena_Create(Result->MainAllocator, Mega(1));
    Result->Callback      = Callback;
    Result->UserData      = UserData;
    Result->Thread        = OS_Create_Thread(Thread_Manager__Thread_Callback, Result);
    
    return Result;
}

void Thread_Manager_Delete_Thread(thread_manager* ThreadManager, thread_context* ThreadContext)
{
    uint32_t ThreadID = OS_Get_Thread_ID(ThreadContext->Thread);
    OS_Wait_Thread(ThreadContext->Thread);
    
    Async_Lock(&ThreadManager->Lock);
    Hashmap_Remove(&ThreadManager->ThreadMap, &ThreadID);
    Async_Unlock(&ThreadManager->Lock);
    
    OS_Delete_Thread(ThreadContext->Thread);
    Arena_Delete(ThreadContext->Scratch);
    Pool_Free(&ThreadManager->ThreadPool, ThreadContext->Handle);
}

thread_local global thread_context* G_ThreadContext;
thread_context* Thread_Manager_Get_Thread_Context(thread_manager* ThreadManager)
{
    if(!G_ThreadContext)
    {
        uint32_t ThreadID = OS_Get_Current_Thread_ID();
        
        Async_Lock(&ThreadManager->Lock);
        thread_context** PThreadContext = Hashmap_Find(&ThreadManager->ThreadMap, thread_context*, &ThreadID);
        if(PThreadContext) G_ThreadContext = *PThreadContext;
        Async_Unlock(&ThreadManager->Lock);
    }
    
    return NULL;
}

void            Thread_Manager_Wait_For_All_Threads(thread_manager* ThreadManager)
{
}