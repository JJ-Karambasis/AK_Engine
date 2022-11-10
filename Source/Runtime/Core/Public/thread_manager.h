#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

typedef struct thread_context thread_context;

#define CORE_THREAD_CALLBACK(name) int32_t name(thread_context* ThreadContext, void* UserData)
typedef CORE_THREAD_CALLBACK(core_thread_callback);

#define MAX_THREAD_CONTEXT_SLOT_COUNT 64

typedef struct thread_context
{
    uint32_t              ThreadID;
    allocator*            MainAllocator;
    arena*                Scratch;
    os_thread*            Thread;
    core_thread_callback* Callback;
    void*                 UserData;
    thread_context*       Next;
    thread_context*       Prev;
} thread_context;

typedef struct thread_slot
{
    thread_context* First;
    thread_context* Last;
    uint64_t        Count;
} thread_slot;

typedef struct thread_manager
{
    arena*          Arena;
    thread_slot     ThreadSlots[MAX_THREAD_CONTEXT_SLOT_COUNT];
    thread_context* FreeThreads;
    async_spin_lock Lock;
} thread_manager;

thread_manager Thread_Manager_Create();
void           Thread_Manager_Delete(thread_manager* ThreadManager);
thread_context* Thread_Manager_Create_Thread(thread_manager* ThreadManager, core_thread_callback* Callback, void* UserData);
void            Thread_Manager_Delete_Thread(thread_manager* ThreadManager, thread_context* ThreadContext);
thread_context* Thread_Manager_Get_Thread_Context(thread_manager* ThreadManager);
void            Thread_Manager_Wait_For_All_Threads(thread_manager* ThreadManager);

#endif