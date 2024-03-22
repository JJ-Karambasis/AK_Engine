#ifndef OS_EVENT_H
#define OS_EVENT_H

struct os_event_manager;

typedef uint64_t os_window_id;

enum os_event_type {
    OS_EVENT_TYPE_NONE,
    OS_EVENT_TYPE_WINDOW_CLOSED,
#ifdef TEST_BUILD
    OS_EVENT_TYPE_TEST,
#endif
    OS_EVENT_TYPE_COUNT
};

struct os_event {
    os_event_type Type;
    os_window_id  WindowID;
};

#ifdef TEST_BUILD
struct os_event_test : public os_event {
    char TestData[5];
};
#endif

const os_event* OS_Next_Event();

#define OS_EVENT_BUFFER_SIZE KB(16)
struct os_event_buffer {
    u8               Buffer[OS_EVENT_BUFFER_SIZE];
    uptr             Used;
    os_event_buffer* Next;
};

struct os_event_stream {
    arena*           BufferArena;
    os_event_buffer* FirstBuffer;
    os_event_buffer* LastBuffer;
    os_event_buffer* CurrentBuffer;

    //When pop count is equal to os_manager_thread ThreadReaderCount
    //the stream is removed from the queue to be reused. This is so 
    //ThreadReaderCount of threads can access the event in the stream
    ak_atomic_u32 PopCount;    

    os_event_stream* Next;
};

struct os_event_queue {
    os_event_stream* FirstStream;
    os_event_stream* LastStream;
};

struct os_event_queue_iter {
    os_event_manager* EventManager;
    os_event_stream*  EventStream;
    os_event_buffer*  EventBuffer;
    uptr              BufferIndex;
};

struct os_event_queue_iter_array {
    os_event_queue_iter* Ptr;
    u32                  Count;
};

struct os_event_stream_storage {
    os_event_stream* FirstFreeStream;
};

struct os_event_manager {
    arena*                    Arena;
    os_event_queue            EventQueue;
    os_event_queue_iter_array IterArray;
    ak_atomic_u32             UsedThreadCount;
    ak_tls                    IterLocalStorage;
    os_event_stream_storage   EventStreamStorage;

#ifdef DEBUG_BUILD
    u32 FreedStreamCount;
    u32 AllocatedStreamCount;
#endif
};

static const uptr G_OSEventTypeSize[] = {
    0,
    sizeof(os_event),
#ifdef TEST_BUILD
    sizeof(os_event_test),
#endif
};

static_assert(Array_Count(G_OSEventTypeSize) == OS_EVENT_TYPE_COUNT);

void                 OS_Event_Manager_Create(os_event_manager* EventManager, u32 NumReaderThreads);
void                 OS_Event_Manager_Delete(os_event_manager* EventManager);
os_event_stream*     OS_Event_Manager_Allocate_Stream(os_event_manager* EventManager);
os_event*            OS_Event_Stream_Allocate_Event(os_event_stream* EventStream, os_event_type Type);
void                 OS_Event_Manager_Push_Back_Stream(os_event_manager* EventManager, os_event_stream* Stream);
os_event_queue_iter* OS_Event_Manager_Get_Iter(os_event_manager* EventManager);
const os_event*      OS_Event_Queue_Iter_Next(os_event_queue_iter* Iter);    

#endif