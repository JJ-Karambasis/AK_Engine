inline uint32_t Thread_Manager__Hash_Thread_ID(uint32_t Key)
{
    Key = (Key+0x7ed55d16) + (Key<<12);
    Key = (Key^0xc761c23c) ^ (Key>>19);
    Key = (Key+0x165667b1) + (Key<<5);
    Key = (Key+0xd3a2646c) ^ (Key<<9);
    Key = (Key+0xfd7046c5) + (Key<<3);
    Key = (Key^0xb55a4f09) ^ (Key>>16);
    return Key;
}

thread_context* Thread_Manager__Allocate_Context(thread_manager* ThreadManager)
{
    thread_context* Result = ThreadManager->FreeThreads;
    if(!Result) Result = Arena_Push_Struct(ThreadManager->Arena, thread_context);
    else ThreadManager->FreeThreads = ThreadManager->FreeThreads->Next;
    Zero_Struct(Result, thread_context);
    
    Result->MainAllocator = OS_Get_Allocator();
    Result->Scratch       = Arena_Create(Result->MainAllocator, Mega(1));
    return Result;
}

void Thread_Manager__Map_Context_To_ThreadID(thread_manager* ThreadManager, thread_context* ThreadContext)
{
    Async_Lock(&ThreadManager->Lock);
    uint32_t SlotMask  = MAX_THREAD_CONTEXT_SLOT_COUNT-1;
    uint32_t Hash      = Thread_Manager__Hash_Thread_ID(ThreadContext->ThreadID);
    uint32_t SlotIndex = Hash & SlotMask;
    thread_slot* Slot  = ThreadManager->ThreadSlots + SlotIndex;
    if(!Slot->First) Slot->First = Slot->Last = ThreadContext;
    else
    {
        ThreadContext->Prev = Slot->Last;
        Slot->Last->Next = ThreadContext;
        Slot->Last = ThreadContext;
    }
    Slot->Count++;
    Async_Unlock(&ThreadManager->Lock);
}

thread_manager Thread_Manager_Create()
{
    thread_manager Result;
    Zero_Struct(&Result, thread_manager);
    
    allocator* Allocator = OS_Get_Allocator();
    Result.Arena = Arena_Create(Allocator, Kilo(16));
    
    thread_context* MainThreadContext = Thread_Manager__Allocate_Context(&Result);
    MainThreadContext->ThreadID = OS_Get_Current_Thread_ID();
    Thread_Manager__Map_Context_To_ThreadID(&Result, MainThreadContext);
    
    return Result;
}

void Thread_Manager_Delete(thread_manager* ThreadManager)
{
    Thread_Manager_Wait_For_All_Threads(ThreadManager);
    
    for(uint32_t SlotIndex = 0; SlotIndex < MAX_THREAD_CONTEXT_SLOT_COUNT; SlotIndex++)
    {
        thread_slot* Slot = ThreadManager->ThreadSlots + SlotIndex;
        for(thread_context* Thread = Slot->First; Thread; Thread = Thread->Next)
        {
            Arena_Delete(Thread->Scratch);
            if(Thread->Thread) OS_Delete_Thread(Thread->Thread);
        }
    }
    Arena_Delete(ThreadManager->Arena);
}

OS_THREAD_CALLBACK(Thread_Manager__Thread_Callback)
{
    thread_context* ThreadContext = (thread_context*)UserData;
    thread_manager* ThreadManager = &Core_Get()->ThreadManager;
    ThreadContext->ThreadID = OS_Get_Current_Thread_ID();
    Thread_Manager__Map_Context_To_ThreadID(ThreadManager, ThreadContext);
    return ThreadContext->Callback(ThreadContext, ThreadContext->UserData);
}

thread_context* Thread_Manager_Create_Thread(thread_manager* ThreadManager, core_thread_callback* Callback, void* UserData)
{
    thread_context* Result = Thread_Manager__Allocate_Context(ThreadManager);
    Result->Callback      = Callback;
    Result->UserData      = UserData;
    Result->Thread        = OS_Create_Thread(Thread_Manager__Thread_Callback, Result);
    return Result;
}

void Thread_Manager_Delete_Thread(thread_manager* ThreadManager, thread_context* ThreadContext)
{
    OS_Wait_Thread(ThreadContext->Thread);
    
    Async_Lock(&ThreadManager->Lock);
    
    uint32_t SlotMask  = MAX_THREAD_CONTEXT_SLOT_COUNT-1;
    uint32_t Hash      = Thread_Manager__Hash_Thread_ID(ThreadContext->ThreadID);
    uint32_t SlotIndex = Hash & SlotMask;
    thread_slot* Slot  = ThreadManager->ThreadSlots + SlotIndex;
    if(Slot->First == ThreadContext)
    {
        Slot->First = Slot->First->Next;
        if(Slot->First) Slot->First->Prev = NULL;
    }
    
    if(Slot->Last == ThreadContext)
    {
        Slot->Last = Slot->Last->Prev;
        if(Slot->Last) Slot->Last->Next = NULL;
    }
    
    if(ThreadContext->Prev) ThreadContext->Prev->Next = ThreadContext->Next;
    if(ThreadContext->Next) ThreadContext->Next->Prev = ThreadContext->Prev;
    Slot->Count--;
    Async_Unlock(&ThreadManager->Lock);
    
    OS_Delete_Thread(ThreadContext->Thread);
    Arena_Delete(ThreadContext->Scratch);
    ThreadContext->Next = ThreadManager->FreeThreads;
    ThreadManager->FreeThreads = ThreadContext;
}

thread_local global thread_context* G_ThreadContext;
thread_context* Thread_Manager_Get_Thread_Context(thread_manager* ThreadManager)
{
    if(!G_ThreadContext)
    {
        uint32_t ThreadID = OS_Get_Current_Thread_ID();
        
        Async_Lock(&ThreadManager->Lock);
        uint32_t SlotMask  = MAX_THREAD_CONTEXT_SLOT_COUNT-1;
        uint32_t Hash      = Thread_Manager__Hash_Thread_ID(ThreadID);
        uint32_t SlotIndex = Hash & SlotMask;
        thread_slot* Slot  = ThreadManager->ThreadSlots + SlotIndex;
        
        for(thread_context* Context = Slot->First; Context; Context = Context->Next)
        {
            if(Context->ThreadID == ThreadID) 
            {
                G_ThreadContext = Context;
                break;
            }
        }
        
        Async_Unlock(&ThreadManager->Lock);
    }
    
    return G_ThreadContext;
}

void Thread_Manager_Wait_For_All_Threads(thread_manager* ThreadManager)
{
}