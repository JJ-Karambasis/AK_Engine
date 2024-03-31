internal os_event_buffer* OS_Event_Stream__Get_Current_Buffer(os_event_stream* Stream, os_event_type EventType) {
    os_event_buffer* Buffer = Stream->CurrentBuffer;
    while(Buffer && (OS_EVENT_BUFFER_SIZE < Buffer->Used+G_OSEventTypeSize[EventType])) {
        Buffer = Buffer->Next;
    }
    return Buffer;
}

internal os_event_buffer* OS_Event_Stream__Allocate_Buffer(os_event_stream* Stream) {
    arena* Arena = Stream->BufferArena;
    os_event_buffer* Result = (os_event_buffer*)Arena_Push_Struct(Arena, os_event_buffer);
    Memory_Clear(Result, sizeof(os_event_buffer));
    return Result;
}

internal void OS_Event_Stream__Pop(os_event_stream* Stream) {
    AK_Atomic_Increment_U32(&Stream->PopCount, AK_ATOMIC_MEMORY_ORDER_RELEASE);
}

internal void OS_Event_Stream__Clear(os_event_stream* Stream) {
    for(os_event_buffer* Buffer = Stream->FirstBuffer; Buffer; Buffer = Buffer->Next) {
        Buffer->Used = 0;
    }
    Stream->CurrentBuffer = Stream->FirstBuffer;
    Stream->Next = NULL;
}

void OS_Event_Manager_Create(os_event_manager* EventManager, u32 NumThreadReaders) {
    Zero_Struct(EventManager);
    EventManager->Arena = Arena_Create(Core_Get_Base_Allocator(), MB(1));
    Arena_Track(EventManager->Arena, String_Lit("OS Event Manager Arena"));
    os_event_queue_iter* Iterators = Arena_Push_Array(EventManager->Arena, NumThreadReaders, os_event_queue_iter);
    EventManager->IterArray.Ptr = Iterators;
    EventManager->IterArray.Count = NumThreadReaders;
    AK_Atomic_Store_U32_Relaxed(&EventManager->UsedThreadCount, 0);
}

void OS_Event_Manager_Delete(os_event_manager* EventManager) {
    Arena_Delete(EventManager->Arena);
    Zero_Struct(EventManager);
}

os_event_stream* OS_Event_Manager_Allocate_Stream(os_event_manager* EventManager) {
    os_event_stream_storage* EventStreamStorage = &EventManager->EventStreamStorage;
    
    //Before we allocate a stream. Check to see if there are any used streams in the queue
    //that can be cleaned up
    os_event_queue* EventQueue = &EventManager->EventQueue;

    os_event_stream* Stream = EventQueue->FirstStream;
    while(Stream && AK_Atomic_Compare_Exchange_Bool_U32_Relaxed(&Stream->PopCount, EventManager->IterArray.Count, 0)) {
        SLL_Pop_Front(EventQueue->FirstStream);
        SLL_Push_Front(EventStreamStorage->FirstFreeStream, Stream);
        Stream = EventQueue->FirstStream;

#ifdef DEBUG_BUILD
        EventManager->FreedStreamCount++;
#endif
    }

    if(!EventQueue->FirstStream) EventQueue->LastStream = NULL;
    
    os_event_stream* Result = EventStreamStorage->FirstFreeStream;
    if(!Result) {
        arena* StreamArena = Arena_Create(EventManager->Arena, KB(32));
        Result = Arena_Push_Struct(StreamArena, os_event_stream);
        Result->BufferArena = StreamArena;
    } else {
        SLL_Pop_Front(EventStreamStorage->FirstFreeStream);
    }
    OS_Event_Stream__Clear(Result);
#ifdef DEBUG_BUILD
    EventManager->AllocatedStreamCount++;
#endif
    return Result;
}

os_event* OS_Event_Stream_Allocate_Event(os_event_stream* EventStream, os_event_type EventType) {
    //All events are in the structure
    //Event type: 4 bytes
    //Event data: N bytes (can be retrieved from G_OSEventTypeSize with event type)
    
    os_event_buffer* Buffer = OS_Event_Stream__Get_Current_Buffer(EventStream, EventType);
    if(!Buffer) {
        Buffer = OS_Event_Stream__Allocate_Buffer(EventStream);
        SLL_Push_Back(EventStream->FirstBuffer, EventStream->LastBuffer, Buffer);
    }

    EventStream->CurrentBuffer = Buffer;
    Assert(Buffer->Used+G_OSEventTypeSize[EventType] <= OS_EVENT_BUFFER_SIZE);
    os_event* Event = (os_event*)(((u8*)Buffer->Buffer) + Buffer->Used);
    Memory_Clear(Event, G_OSEventTypeSize[EventType]);
    Event->Type = EventType;
    Buffer->Used += G_OSEventTypeSize[EventType];
    return Event;
}

void OS_Event_Manager_Push_Back_Stream(os_event_manager* EventManager, os_event_stream* Stream) {
    if(!Stream->FirstBuffer || !Stream->FirstBuffer->Used) {
        //If the stream has no events written into it, we can add this stream
        //to the free list to be used later again
        os_event_stream_storage* EventStreamStorage = &EventManager->EventStreamStorage;
        SLL_Push_Front(EventStreamStorage->FirstFreeStream, Stream);
    } else {
        os_event_queue* EventQueue = &EventManager->EventQueue;
        SLL_Push_Back(EventQueue->FirstStream, EventQueue->LastStream, Stream);
    }
}

os_event_queue_iter* OS_Event_Manager_Get_Iter(os_event_manager* EventManager) {
    os_event_queue_iter* Result = (os_event_queue_iter*)AK_TLS_Get(&EventManager->IterLocalStorage);
    if(!Result) {
        uint32_t IterIndex = AK_Atomic_Fetch_Add_U32_Relaxed(&EventManager->UsedThreadCount, 1);
        Assert(IterIndex < EventManager->IterArray.Count);
        Result = &EventManager->IterArray.Ptr[IterIndex];
        Result->EventManager = EventManager;

        AK_TLS_Set(&EventManager->IterLocalStorage, Result);
    }
    return Result;
}

const os_event* OS_Event_Queue_Iter_Next(os_event_queue_iter* Iter) {
    os_event_manager* EventManager = Iter->EventManager;
    os_event_queue*   EventQueue   = &EventManager->EventQueue;

    //Check if the iterator has a valid stream, if not assign the stream to the
    //front of the queue
    if(!Iter->EventStream) {
        Iter->EventStream = EventQueue->FirstStream;
        
        //If queue has nothing we can return
        if(!Iter->EventStream) return NULL;

        Iter->EventBuffer = Iter->EventStream->FirstBuffer;
        Iter->BufferIndex = 0;
    }

    //Check if we exceed pass the initial event buffer size
    //If we do, we need to pull the next event buffer in the stream
    while(Iter->BufferIndex >= Iter->EventBuffer->Used) {
        Assert(Iter->BufferIndex == Iter->EventBuffer->Used);
        os_event_buffer* NewEventBuffer = Iter->EventBuffer->Next;

        //The stream is full if there are no more event buffers
        if(!NewEventBuffer) {
            //If queue has no more entries left we can return
            os_event_stream* NewEventStream = Iter->EventStream->Next;
            if(!NewEventStream) return NULL;

            //Stream is empty so we can pop the stream
            OS_Event_Stream__Pop(Iter->EventStream);
            Iter->EventStream = NewEventStream;
            Iter->EventBuffer = Iter->EventStream->FirstBuffer;
        } else {
            Iter->EventBuffer = NewEventBuffer;
        }
        Iter->BufferIndex = 0;
    }

    const os_event* Event = (const os_event*)(((u8*)Iter->EventBuffer->Buffer) + Iter->BufferIndex);
    Iter->BufferIndex += G_OSEventTypeSize[Event->Type];
    Assert(Iter->BufferIndex <= Iter->EventBuffer->Used);
    return Event;
}