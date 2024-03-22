#define MAX_ITERATIONS 50
#define MAX_STREAM_PUSHES 10
#define MAX_COUNT 1000

typedef struct {
    char TestData[sizeof(((os_event_test*)0)->TestData)];
} os_event_sync_thread_data_entry;

typedef struct {
    u32                             Count;
    os_event_sync_thread_data_entry Entries[MAX_STREAM_PUSHES*MAX_COUNT];
} os_event_sync_thread_data;

os_event_sync_thread_data_entry* OS_Event_Sync_Thread_Data_Get_Next_Entry(os_event_sync_thread_data* ThreadData) {
    Assert(ThreadData->Count < Array_Count(ThreadData->Entries));
    os_event_sync_thread_data_entry* Result = ThreadData->Entries + ThreadData->Count++;
    return Result;
}

os_event_sync_thread_data_entry* OS_Event_Sync_Thread_Data_Get(os_event_sync_thread_data* ThreadData, u32 Index) {
    Assert(Index < ThreadData->Count);
    return ThreadData->Entries + Index;
}

typedef struct {
    char TestData[sizeof(((os_event_test*)0)->TestData)];
} os_event_async_thread_data_entry;

typedef struct {
    os_event_manager*                EventManager;
    u32                              Count;
    os_event_async_thread_data_entry Entries[MAX_ITERATIONS*MAX_STREAM_PUSHES*MAX_COUNT];
} os_event_async_thread_data;

os_event_async_thread_data_entry* OS_Event_Async_Thread_Data_Get_Next_Entry(os_event_async_thread_data* ThreadData) {
    Assert(ThreadData->Count < Array_Count(ThreadData->Entries));
    os_event_async_thread_data_entry* Result = ThreadData->Entries + ThreadData->Count++;
    return Result;
}

os_event_async_thread_data_entry* OS_Event_Async_Thread_Data_Get(os_event_async_thread_data* ThreadData, u32 Index) {
    Assert(Index < ThreadData->Count);
    return ThreadData->Entries + Index;
}

u32 OS_Event_Thread(thread_context* ThreadContext, void* UserData) {
    os_event_async_thread_data* ThreadData = (os_event_async_thread_data*)UserData;
    os_event_queue_iter* Iter = OS_Event_Manager_Get_Iter(ThreadData->EventManager);

    while(ThreadData->Count < (MAX_ITERATIONS*MAX_STREAM_PUSHES*MAX_COUNT)) {
        for(;;) {
            const os_event_test* Event = (const os_event_test*)OS_Event_Queue_Iter_Next(Iter);
            if(!Event) break;

            os_event_async_thread_data_entry* Entry = OS_Event_Async_Thread_Data_Get_Next_Entry(ThreadData);
            Memory_Copy(Entry->TestData, Event->TestData, sizeof(Event->TestData));
        }
    }

    return 0;
}

UTEST(OS_Event, Test_Async) {
    //u32 ThreadCount = AK_Get_Processor_Thread_Count();
    u32 ThreadCount = AK_Get_Processor_Thread_Count();
    os_event_manager EventManager = {};
    OS_Event_Manager_Create(&EventManager, ThreadCount);
    os_event_async_thread_data* ThreadData = Allocator_Allocate_Array(Core_Get_Base_Allocator(), ThreadCount, os_event_async_thread_data);
    Zero_Array(ThreadData, ThreadCount);
    for(u32 ThreadIndex = 0; ThreadIndex < ThreadCount; ThreadIndex++) {
        ThreadData[ThreadIndex].EventManager = &EventManager;
        Thread_Manager_Create_Thread(OS_Event_Thread, &ThreadData[ThreadIndex]);
    }

    for(u32 i = 0; i < MAX_ITERATIONS; i++) {
        for(u32 j = 0; j < MAX_STREAM_PUSHES; j++) {
            os_event_stream* Stream = OS_Event_Manager_Allocate_Stream(&EventManager);
            for(u32 k = 0; k < MAX_COUNT; k++) {
                os_event_test* Event = (os_event_test*)OS_Event_Stream_Allocate_Event(Stream, OS_EVENT_TYPE_TEST);
                stbsp_sprintf(Event->TestData, "%u", k);
            }
            OS_Event_Manager_Push_Back_Stream(&EventManager, Stream);
        }
        AK_Sleep(1);
    }

    Thread_Manager_Wait_All();

    printf("Async Final: Allocated(%d) Freed(%d)\n", EventManager.AllocatedStreamCount, EventManager.FreedStreamCount);

    u32 ActualIndex = 0;
    for(u32 i = 0; i < MAX_ITERATIONS; i++) {
        for(u32 j = 0; j < MAX_STREAM_PUSHES; j++) {
            for(u32 k = 0; k < MAX_COUNT; k++) {
                char Buffer[sizeof(((os_event_test*)0)->TestData)] = {0};
                stbsp_sprintf(Buffer, "%u", k);
                for(u32 ThreadIndex = 0; ThreadIndex < ThreadCount; ThreadIndex++) {
                    os_event_async_thread_data_entry* ThreadEntry = OS_Event_Async_Thread_Data_Get(&ThreadData[ThreadIndex], ActualIndex);
                    ASSERT_TRUE(strcmp(ThreadEntry->TestData, Buffer) == 0);
                }

                ActualIndex++;
            }
        }   
    }

    OS_Event_Manager_Delete(&EventManager);
}

#undef MAX_ITERATIONS
#undef MAX_STREAM_PUSHES
#undef MAX_COUNT