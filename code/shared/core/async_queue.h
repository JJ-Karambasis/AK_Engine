#ifndef ASYNC_QUEUE_H
#define ASYNC_QUEUE_H

template <typename type>
struct async_spmc_queue {
    allocator*                  Allocator;
    type*                       Entries;
    ak_async_stack_index32      StackIndex; //todo: Change to spmc stack index when ak_atomic supports it
    ak_async_spmc_queue_index32 QueueIndex;
};

#define Async_SPMC_Queue_Capacity(queue) AK_Async_Queue_Index32_Capacity(&(queue)->QueueIndex.Capacity)

template <typename type>
inline void Async_SPMC_Queue_Create(async_spmc_queue<type>* Queue, allocator* Allocator, u32 Capacity) {
    uptr AllocationSize = (sizeof(type)+sizeof(u32)+sizeof(u32))*Capacity;
    Queue->Entries   = (type*)Allocator_Allocate_Memory(Allocator, AllocationSize);
    if(!Queue->Entries) return;

    Queue->Allocator = Allocator;
    u32* StackIndicesPtr = (u32*)(Queue->Entries+Capacity);
    u32* QueueIndicesPtr = (u32*)(StackIndicesPtr+Capacity);
    AK_Async_Stack_Index32_Init_Raw(&Queue->StackIndex, StackIndicesPtr, Capacity);
    AK_Async_SPMC_Queue_Index32_Init_Raw(&Queue->QueueIndex, QueueIndicesPtr, Capacity);
    for(u32 i = 0; i < Capacity; i++) {
        AK_Async_Stack_Index32_Push_Sync(&Queue->StackIndex, i);
    }
}

template <typename type>
inline void Async_SPMC_Queue_Delete(async_spmc_queue<type>* Queue) {
    if(Queue->Entries && Queue->Allocator) {
        Allocator_Free_Memory(Queue->Allocator, Queue->Entries);
        Queue->Allocator = NULL;
        Queue->Entries = NULL;
    }
}

template <typename type>
inline void Async_SPMC_Queue_Enqueue(async_spmc_queue<type>* Queue, const type& Entry) {
    u32 Index = AK_Async_Stack_Index32_Pop(&Queue->StackIndex);
    if(Index == AK_ASYNC_STACK_INDEX32_INVALID) {
        Assert(false);
        return;
    }

    Assert(Index < Async_SPMC_Queue_Capacity(Queue));
    Queue->Entries[Index] = Entry;

    AK_Async_SPMC_Queue_Index32_Enqueue(&Queue->QueueIndex, Index);
}

template <typename type>
inline bool Async_SPMC_Queue_Dequeue(async_spmc_queue<type>* Queue, type* Entry) {
    u32 Index = AK_Async_SPMC_Queue_Index32_Dequeue(&Queue->QueueIndex);
    if(Index == AK_ASYNC_SPMC_QUEUE_INDEX32_INVALID) {
        return false;
    }

    Assert(Index < Async_Queue_Capacity(Queue));
    *Entry = Queue->Entries[Index];
    AK_Async_Stack_Index32_Push(&Queue->StackIndex, Index);
    return true;
}

#endif