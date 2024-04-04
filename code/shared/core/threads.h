#ifndef THREADS_H
#define THREADS_H

#define MAX_SCRATCH_COUNT 128
#define MAX_THREAD_COUNT  128

void QSBR_Update();

struct scratch : public allocator {
    arena* Arena;
    scratch(arena* Arena);
    ~scratch();
};

scratch Scratch_Get();
void    Scratch_Release();
void*   Scratch_Push(scratch* Scratch, uptr Size, uptr Alignment = DEFAULT_ALIGNMENT, allocator_clear_flag ClearFlag = DEFAULT_CLEAR_FLAG);

#define Scratch_Push_Struct(scratch, type) (type*)Scratch_Push(scratch, sizeof(type), alignof(type))
#define Scratch_Push_Array(scratch, count, type) (type*)Scratch_Push(scratch, sizeof(type)*count, alignof(type))

struct thread_context;
typedef u32 thread_context_callback(thread_context* Context, void* UserData);

struct thread_context {
	arena* 		    ScratchPool[MAX_SCRATCH_COUNT];
	arena_marker    ScratchMarkers[MAX_SCRATCH_COUNT];
	u32    		    CurrentScratchIndex;
	ak_qsbr_context QSBRContext;
	
    u64    	  ThreadID;
	ak_thread Thread;
	
    thread_context_callback* Callback;
	void*            		 CallbackUserData;

	u32 PoolIndex;
	u32 PoolNextIndex;

	thread_context* HashNext;
	thread_context* HashPrev;
};

void 			Thread_Context_Delete(thread_context* ThreadContext);
void 			Thread_Context_Wait(thread_context* ThreadContext);
thread_context* Thread_Context_Get();

struct thread_pool {
	thread_context Threads[MAX_THREAD_COUNT];
	u32            FirstFreeIndex;
	u32            MaxUsed;
};

struct thread_slot {
	thread_context* SlotHead;
	thread_context* SlotTail;
};

struct thread_map {
	thread_slot Slots[MAX_THREAD_COUNT];
};

struct thread_manager {
	ak_mutex  		AllocateLock;
	ak_mutex        MapLock;
	thread_map 		ThreadMap;
	thread_pool     ThreadPool;
	thread_context* MainThreadContext;
	ak_tls          ThreadContextLocalStorage;
	ak_qsbr*        QSBR;
};

thread_manager* Thread_Manager_Create();
void            Thread_Manager_Delete();
void            Thread_Manager_Wait_All();
thread_context* Thread_Manager_Create_Thread(thread_context_callback* Callback, void* UserData);

thread_manager* Thread_Manager_Get();
void            Thread_Manager_Set(thread_manager* Manager);

#endif