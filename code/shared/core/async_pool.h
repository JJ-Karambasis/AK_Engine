#ifndef ASYNC_POOL_H
#define ASYNC_POOL_H

template <typename type>
struct async_handle {
    u64 ID = 0;
    async_handle() = default;
    inline async_handle(u64 _ID) : ID(_ID) { } 
    inline bool Is_Null() { return ID == 0; }
    inline u32 Index() {
        return AK_Slot64_Index(ID);
    }
};

template <typename type>
struct async_pool {
    type*               Ptr;
    ak_async_slot_map64 SlotMap;
};

#define Async_Pool_Capacity(pool) AK_Async_Slot_Map64_Capacity(&(pool)->SlotMap)

template <typename type>
inline void Async_Pool_Create(async_pool<type>* Pool, allocator* Allocator, u32 Capacity) {
    uptr AllocationSize = (sizeof(ak_rw_lock)*sizeof(uint32_t)+sizeof(ak_slot64)+sizeof(type))*Capacity;
    type* Ptr = (type*)Allocator_Allocate_Memory(Allocator, AllocationSize);
    if(!Ptr) return;

    Pool->Ptr = Ptr;
    
    u32* FreeIndices = (u32*)(Ptr+Capacity);
    ak_slot64* Slots = (ak_slot64*)(FreeIndices+Capacity);
    AK_Async_Slot_Map64_Init_Raw(&Pool->SlotMap, FreeIndices, Slots, Capacity);
}

template <typename type>
inline void Async_Pool_Delete(async_pool<type>* Pool, allocator* Allocator) {
    if(Allocator && Pool->Ptr) {
        Allocator_Free_Memory(Allocator, Pool->Ptr);
        Pool->Ptr = NULL;
    }
}

template <typename type>
inline type* Async_Pool_Get(async_pool<type>* Pool, async_handle<type> Handle) {
    if(AK_Async_Slot_Map64_Is_Allocated(&Pool->SlotMap, Handle.ID)) {
        u32 Index = AK_Slot64_Index(Handle.ID);
        Assert(Index < Async_Pool_Capacity(Pool));
        return Pool->Ptr + Index;
    }
    return NULL;
}

template <typename type>
inline async_handle<type> Async_Pool_Allocate(async_pool<type>* Pool) {
    ak_slot64 SlotID = AK_Async_Slot_Map64_Alloc_Slot(&Pool->SlotMap);
    if(!SlotID) { 
        return {};
    }
    return async_handle<type>(SlotID);
}

template <typename type>
inline void Async_Pool_Free(async_pool<type>* Pool, async_handle<type> Handle) {
    if(AK_Async_Slot_Map64_Is_Allocated(&Pool->SlotMap, Handle.ID)) {
        u32 Index   = AK_Slot64_Index(Handle.ID);
        type* Entry = Pool->Ptr + Index;
        Zero_Struct(Entry);
        AK_Async_Slot_Map64_Free_Slot(&Pool->SlotMap, Handle.ID);
    }
}

#endif