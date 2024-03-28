#ifndef ASYNC_POOL_H
#define ASYNC_POOL_H

template <typename type>
struct async_handle {
    u64 ID = 0;
    async_handle() = default;
    inline async_handle(u64 _ID) : ID(_ID) { } 
    inline bool Is_Null() { return ID == 0; }
};

template <typename type>
struct async_pool {
    allocator*          Allocator;
    type*               Ptr;
    ak_rw_lock*         Locks;
    ak_async_slot_map64 SlotMap;
};

#define Async_Pool_Capacity(pool) AK_Async_Slot_Map64_Capacity(&(pool)->SlotMap)

template <typename type>
inline void Async_Pool_Create(async_pool<type>* Pool, allocator* Allocator, u32 Capacity) {
    uptr AllocationSize = (sizeof(ak_rw_lock)*sizeof(uint32_t)+sizeof(ak_slot64)+sizeof(type))*Capacity;
    type* Ptr = (type*)Allocator_Allocate_Memory(Allocator, AllocationSize);
    if(!Ptr) return;

    Pool->Allocator = Allocator;
    Pool->Ptr = Ptr;
    Pool->Locks = (ak_rw_lock*)(Ptr+Capacity);
    
    u32* FreeIndices = (u32*)(Pool->Locks+Capacity);
    ak_slot64* Slots = (ak_slot64*)(FreeIndices+Capacity);
    AK_Async_Slot_Map64_Init_Raw(&Pool->SlotMap, FreeIndices, Slots, Capacity);

    for(u32 i = 0; i < Capacity; i++) {
        AK_RW_Lock_Create(&Pool->Locks[i]);
    }
}

template <typename type>
inline void Async_Pool_Delete(async_pool<type>* Pool) {
    for(u32 i = 0; i < Async_Pool_Capacity(Pool); i++) {
        AK_RW_Lock_Delete(&Pool->Locks[i]);
    }
    
    if(Pool->Allocator && Pool->Ptr) {
        Allocator_Free_Memory(Pool->Ptr);
        Pool->Ptr = NULL;
        Pool->Allocator = NULL;
    }
}

template <typename type>
inline type* Async_Pool_Lock_Writer(async_pool<type>* Pool, async_handle<type> Handle) {
    if(AK_Async_Slot_Map64_Is_Allocated(&Pool->SlotMap, Handle.ID)) {
        u32 Index = AK_Slot64_Index(Handle.ID);
        Assert(Index < Async_Pool_Capacity(Pool));
        AK_RW_Lock_Writer(&Pool->Locks[Index]);
        return Pool->Ptr + Index;
    }
    return NULL;
}

template <typename type>
inline void Async_Pool_Unlock_Writer(async_pool<type>* Pool, async_handle<type> Handle) {
    if(AK_Async_Slot_Map64_Is_Allocated(&Pool->SlotMap, Handle.ID)) {
        u32 Index = AK_Slot64_Index(Handle.ID);
        Assert(Index < Async_Pool_Capacity(Pool));
        AK_RW_Unlock_Writer(&Pool->Locks[Index]);
    }
}

template <typename type>
inline const type* Async_Pool_Lock_Reader(const async_pool<type>* Pool, async_handle<type> Handle) {
    if(AK_Async_Slot_Map64_Is_Allocated(&Pool->SlotMap, Handle.ID)) {
        u32 Index = AK_Slot64_Index(Handle.ID);
        Assert(Index < Async_Pool_Capacity(Pool));
        AK_RW_Lock_Reader(&Pool->Locks[Index]);
        return Pool->Ptr + Index;
    }
    return NULL;
}

template <typename type>
inline void Async_Pool_Unlock_Reader(const async_pool<type>* Pool, async_handle<type> Handle) {
    if(AK_Async_Slot_Map64_Is_Allocated(&Pool->SlotMap, Handle.ID)) {
        u32 Index = AK_Slot64_Index(Handle.ID);
        Assert(Index < Async_Pool_Capacity(Pool));
        AK_RW_Unlock_Reader(&Pool->Locks[Index]);
    }
}

template <typename type>
inline async_handle<type> Async_Pool_Allocate(async_pool<type>* Pool) {
    ak_slot64 SlotID = AK_Async_Slot_Map64_Alloc_Slot(&Pool->SlotMap);
    if(!SlotID) { 
        return {};
    }

    u32 Index = AK_Slot64_Index(SlotID);
    type* Entry = Pool->Ptr + Index;
    Zero_Struct(Entry);
    return async_handle<type>(SlotID);
}

template <typename type>
inline void Async_Pool_Free(async_pool<type>* Pool, async_handle<type> Handle) {
    if(AK_Async_Slot_Map64_Is_Allocated(&Pool->SlotMap, Handle.ID)) {
        AK_Async_Slot_Map64_Free_Slot(&Pool->SlotMap, Handle.ID);
    }
}

template <typename type>
struct pool_writer_lock {
    type*              Ptr;
    async_handle<type> Handle;
    async_pool<type>*  Pool;

    inline pool_writer_lock(async_pool<type>* _Pool, async_handle<type> _Handle) : Handle(_Handle), Pool(_Pool) {
        Ptr = Async_Pool_Lock_Writer(Pool, Handle);
    } 

    inline type* operator->() {
        return Ptr;
    }

    inline void Unlock() {
        if(Ptr) {
            Async_Pool_Unlock_Writer(Pool, Handle);
            Ptr = NULL;
        }
    }
};

template <typename type>
struct pool_reader_lock {
    const type*             Ptr;
    async_handle<type>      Handle;
    const async_pool<type>* Pool;

    inline pool_reader_lock(const async_pool<type>* _Pool, async_handle<type> _Handle) : Handle(_Handle), Pool(_Pool) {
        Ptr = Async_Pool_Lock_Reader(Pool, Handle);
    } 

    inline const type* operator*() const {
        return Ptr;
    }

    inline const type* operator->() const {
        return Ptr;
    }

    inline void Unlock() {
        if(Ptr) {
            Async_Pool_Unlock_Reader(Pool, Handle);
            Ptr = NULL;
        }
    }
};

template <typename type>
struct pool_scoped_lock {
    type* Lock;
    inline pool_scoped_lock(type* _Lock=NULL) : Lock(_Lock) { }
    inline pool_scoped_lock& operator=(pool_scoped_lock&& Scope) {
        Lock = Scope.Lock;
        Scope.Lock = NULL;
        return *this;
    }
    inline ~pool_scoped_lock() { if(Lock) { Lock->Unlock(); Lock = NULL; }}
};

template <typename type>
inline pool_scoped_lock<type> Begin_Scoped_Lock(type* Type) {
    return pool_scoped_lock<type>(Type);
}

#endif