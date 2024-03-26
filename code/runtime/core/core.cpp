#include <math.h>
#include <stdlib.h>

void Allocator_Tracker_Manager__Create_Tracker(allocator_tracker_manager* TrackerManager, allocator* Allocator, string DebugName, allocator_type Type) {
    //Validation
    Assert(Allocator->Tracker == NULL);
    allocator_tracker* Tracker = NULL;
    
    //Grab a free tracker asynchronously
    AK_Mutex_Lock(&TrackerManager->TrackAllocLock);
    Tracker = TrackerManager->FirstFreeTracker;
    if(!Tracker) Tracker = Arena_Push_Struct(TrackerManager->TrackerArena, allocator_tracker);
    else SLL_Pop_Front_N(TrackerManager->FirstFreeTracker, NextSibling);
    AK_Mutex_Unlock(&TrackerManager->TrackAllocLock);

    //Clear and fill out tracker properties.
    //Base allocator is always thread safe
    allocator* BaseAllocator = Core_Get_Base_Allocator();
    Memory_Clear(Tracker, sizeof(allocator_tracker));
    Tracker->Name = string(BaseAllocator, DebugName);
    Tracker->Type = Type;
    Tracker->Allocator = Allocator;
    Allocator->Tracker = Tracker;
    allocator* ParentAllocator = Allocator->ParentAllocator;

    //Asynchronously add to tracking tree
    AK_Mutex_Lock(&TrackerManager->TrackTreeLock);
    if(ParentAllocator) {
        allocator_tracker* ParentTracker = ParentAllocator->Tracker;
        DLL_Push_Back_NP(ParentTracker->FirstChild, ParentTracker->LastChild, 
                         Tracker, NextSibling, PrevSibling);
    } else {
        Assert(!TrackerManager->RootTracker);
        TrackerManager->RootTracker = Tracker;
    }
    AK_Mutex_Unlock(&TrackerManager->TrackTreeLock);
}

void Allocator_Tracker_Manager__Delete_Tracker(allocator_tracker_manager* TrackerManager, allocator_tracker* Tracker) {
    //Validation
    Assert(Tracker->Allocator);
    allocator* Allocator = Tracker->Allocator;
    Assert(Allocator->Tracker == Tracker);

    //Remove allocator from the tracking tree asynchronously
    allocator* ParentAllocator = Allocator->ParentAllocator;
    AK_Mutex_Lock(&TrackerManager->TrackTreeLock);
    if(ParentAllocator) {
        allocator_tracker* ParentTracker = ParentAllocator->Tracker;
        DLL_Remove_NP(ParentTracker->FirstChild, ParentTracker->LastChild, 
                      Tracker, NextSibling, PrevSibling);
    } else {
        Assert(TrackerManager->RootTracker == Tracker);
        TrackerManager->RootTracker = NULL;
    }
    AK_Mutex_Unlock(&TrackerManager->TrackTreeLock);

    //Free memory from base allocator.
    //Base allocator is thread safe
    allocator* BaseAllocator = Core_Get_Base_Allocator();
    String_Free(BaseAllocator, Tracker->Name);

    //Asynchronously add tracker to the free list
    AK_Mutex_Lock(&TrackerManager->TrackAllocLock);
    SLL_Push_Front_N(TrackerManager->FirstFreeTracker, Tracker, NextSibling);
    AK_Mutex_Unlock(&TrackerManager->TrackAllocLock);
}

void Allocator_Delete__Tracker(allocator* Allocator) {
    if(Allocator->Tracker) {
        core* Core = Core_Get();
        allocator_tracker_manager* TrackerManager = &Core->AllocatorTrackerManager;
        Allocator_Tracker_Manager__Delete_Tracker(TrackerManager, Allocator->Tracker);
        Allocator->Tracker = NULL;
    }
}

void Allocator_Create__Tracker(allocator* Allocator, string DebugName, allocator_type Type) {
    core* Core = Core_Get();
    allocator_tracker_manager* TrackerManager = &Core->AllocatorTrackerManager;
    Allocator_Tracker_Manager__Create_Tracker(TrackerManager, Allocator, DebugName, Type);
}

internal void Allocator_Tracker_Manager__Init(allocator_tracker_manager* TrackerManager) {
    AK_Mutex_Create(&TrackerManager->TrackAllocLock);
    AK_Mutex_Create(&TrackerManager->TrackTreeLock);
    TrackerManager->TrackerArena = Arena_Create(Core_Get_Base_Allocator(), (uptr)-1);
    TrackerManager->FirstFreeTracker = NULL;
    TrackerManager->RootTracker = NULL;
}

internal void Allocator_Tracker_Manager__Shutdown(allocator_tracker_manager* TrackerManager) {
    TrackerManager->RootTracker = NULL;
    Arena_Delete(TrackerManager->TrackerArena);
    AK_Mutex_Delete(&TrackerManager->TrackTreeLock);
    AK_Mutex_Delete(&TrackerManager->TrackAllocLock);
}

core* Core_Create() {
    virtual_allocator* VirtualAllocator = Virtual_Allocator_Create();
    heap* Heap                          = Heap_Create(VirtualAllocator, MB(64));
    lock_allocator* LockAllocator       = Lock_Allocator_Create(Heap);

    core* Core = Allocator_Allocate_Struct(Heap, core);
    Zero_Struct(Core);
    Core->VirtualAllocator = VirtualAllocator;
    Core->MainAllocator    = LockAllocator;
    Core_Set(Core);

    Core->ThreadManager = Thread_Manager_Create();

    Allocator_Tracker_Manager__Init(&Core->AllocatorTrackerManager);
    Virtual_Allocator_Track(VirtualAllocator, String_Lit("Base Virtual Allocator"));
    Heap_Track(Heap, String_Lit("Base Heap Allocator"));
    Lock_Allocator_Track(LockAllocator, String_Lit("Base Allocator"));

    Core->LogManager = Log_Manager_Create();

    return Core;
}

void Core_Delete() {
    core* Core = Core_Get();
    if(Core) {
        Log_Manager_Delete();
        Thread_Manager_Delete();
        
        allocator_tracker_manager AllocatorTrackerManager = Core->AllocatorTrackerManager;
        heap* Heap = (heap*)Core->MainAllocator->ParentAllocator;

        virtual_allocator* VirtualAllocator = Core->VirtualAllocator;

        Core->MainAllocator->Tracker = NULL;
        Heap->Tracker = NULL;
        Heap->FreeBlockTree.NodeArena->Tracker = NULL;
        VirtualAllocator->Tracker = NULL;
        Allocator_Tracker_Manager__Shutdown(&AllocatorTrackerManager);

        Lock_Allocator_Delete(Core->MainAllocator);
        Heap_Delete(Heap);
        Virtual_Allocator_Delete(VirtualAllocator);

        Core_Set(NULL);
    }
}

allocator* Core_Get_Base_Allocator() {
    core* Core = Core_Get();
    return Core->MainAllocator;
}

static AK_JOB_THREAD_UPDATE_DEFINE(Core_Job_System_Thread_Update) {
    thread_context* ThreadContext = Thread_Context_Get();
    Assert(ThreadContext->CurrentScratchIndex == 0);
}

static AK_JOB_THREAD_END_DEFINE(Core_Job_System_Thread_End) {
    Thread_Context_Delete(Thread_Context_Get());
    Logger_Delete(Logger_Get());
}

ak_job_system* Core_Create_Job_System(uint32_t MaxJobCount, uint32_t NumThreads, uint32_t NumDependencies) {
    ak_job_thread_callbacks ThreadCallbacks = {0};
    ThreadCallbacks.JobThreadUpdate = Core_Job_System_Thread_Update;
    ThreadCallbacks.JobThreadEnd = Core_Job_System_Thread_End;
    return AK_Job_System_Create(MaxJobCount, NumThreads, NumDependencies, &ThreadCallbacks, NULL);
}

void Core_Delete_Job_System(ak_job_system* JobSystem) {
    AK_Job_System_Delete(JobSystem);
}

ak_job_queue* Core_Create_Job_Queue(uint32_t MaxJobCount, uint32_t NumThreads, uint32_t NumDependencies) {
    ak_job_thread_callbacks ThreadCallbacks = {0};
    ThreadCallbacks.JobThreadUpdate = Core_Job_System_Thread_Update;
    ThreadCallbacks.JobThreadEnd = Core_Job_System_Thread_End;
    return AK_Job_Queue_Create(MaxJobCount, NumThreads, NumDependencies, &ThreadCallbacks, NULL);
}

void Core_Delete_Job_Queue(ak_job_queue* JobQueue) {
    AK_Job_Queue_Delete(JobQueue);
}

global core* G_Core;
core* Core_Get() {
    Assert(G_Core);
    return G_Core;
}

void Core_Set(core* Core) {
    G_Core = Core;
    if(Core) {
        Thread_Manager_Set(Core->ThreadManager);
        Log_Manager_Set(Core->LogManager);
    } else {
        Thread_Manager_Set(NULL);
        Log_Manager_Set(NULL);
    }
}

#include "log.cpp"
#include "datetime.cpp"
#include "threads.cpp"
#include "memory.cpp"
#include "hash.cpp"
#include "strings.cpp"
#include "utility.cpp"

#define AK_JOB_MALLOC(size, user_data) Allocator_Allocate_Memory(Core_Get_Base_Allocator(), size)
#define AK_JOB_FREE(memory, user_data) Allocator_Free_Memory(Core_Get_Base_Allocator(), memory)
#define AK_ATOMIC_IMPLEMENTATION
#include <ak_atomic.h>

#ifdef COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4365)
#endif

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#ifdef COMPILER_MSVC
#pragma warning(pop)
#endif