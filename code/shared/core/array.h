#ifndef ARRAY_H
#define ARRAY_H

template <typename type, uptr N>
struct static_array {
    type Array[N];

    inline type& operator[](uptr Index) {
        Assert(Index < N);
        return Array[Index];
    }

    inline const type& operator[](uptr Index) const {
        Assert(Index < N);
        return Array[Index];
    }

    inline static constexpr uptr Count() {
        return N;
    }
};

template <typename type>
struct array {
    allocator* Allocator = nullptr;
    type*      Ptr = nullptr;
    uptr       Capacity = 0;
    uptr       Count = 0;

    array() = default;
    array(allocator* Allocator, uptr InitialCapacity=32);

    inline type& operator[](uptr Index) {
        Assert(Index < Count);
        return Ptr[Index];
    }

    inline type* begin() { return Ptr; }
    inline type* end() { return Ptr+Count; }

    inline const type* begin() const { return Ptr; }
    inline const type* end() const { return Ptr+Count; }
};

template <typename type>
inline void Array_Reserve(array<type>* Array, uptr Size) {
    uptr NewCapacity = Size;
    type* NewData = Allocator_Allocate_Array(Array->Allocator, NewCapacity, type);

    if(Array->Ptr) {
        uptr CopySize = Array->Capacity > NewCapacity ? NewCapacity : Array->Capacity;
        Array_Copy(NewData, Array->Ptr, CopySize);
        Allocator_Free_Memory(Array->Allocator, Array->Ptr);
    } 

    Array->Ptr = NewData;
    Array->Capacity = NewCapacity;

    if(Array->Capacity < Array->Count)
        Array->Count = Array->Capacity;    
}

template <typename type>
inline void Array_Resize(array<type>* Array, uptr NewSize) {
    //If the count is less we just take the new count
    if(NewSize < Array->Count) {
        Array->Count = NewSize;
    } else {
        //If the new count is greater, but less than the capacity we can safely take the new count
        if(NewSize <= Array->Capacity) {
            Array->Count = NewSize;
        } else {
            //Otherwise we need to resize the array to fix the new size
            uptr Diff = NewSize - Array->Capacity;
            Diff = Max(Diff, Array->Capacity);
            Array_Reserve(Array, Array->Capacity+Diff);
            Array->Count = NewSize;
        }
    }
}

template <typename type>
inline void Array_Clear(array<type>* Array) {
    Array->Count = 0;
}

template <typename type>
inline void Array_Init(array<type>* Array, allocator* Allocator, uptr InitialCapacity=32) {
    Array->Allocator = Allocator;
    Array_Reserve(Array, InitialCapacity);
}

template <typename type>
inline void Array_Free(array<type>* Array) {
    Array->Count = 0;
    Array->Capacity = 0;

    if(Array->Allocator && Array->Ptr) {
        Allocator_Free_Memory(Array->Allocator, Array->Ptr);
        Array->Ptr = NULL;
        Array->Allocator = NULL;
    }
}

template <typename type>
inline void Array_Push(array<type>* Array, const type& Entry) {
    if(Array->Count == Array->Capacity) {
        Array_Reserve(Array, Array->Capacity*2);
    }
    Array->Ptr[Array->Count++] = Entry;
}

template <typename type>
inline void Array_Push(array<type>* Array, type&& Entry) {
    if(Array->Count == Array->Capacity) {
        Array_Reserve(Array, Array->Capacity*2);
    }
    Array->Ptr[Array->Count++] = Entry;
}

template <typename type>
inline bool Array_Empty(array<type>* Array) {
    return Array->Count == 0 || !Array->Ptr;
}

template <typename type>
inline void Array_Push_Range(array<type>* Array, const type* Ptr, uptr Count) {
    if(Array->Count+Count > Array->Capacity) {
        uptr NewCapacity = Max(Array->Capacity*2, Array->Count+Count);
        Array_Reserve(Array, NewCapacity);
    }
    Memory_Copy(Array->Ptr+Array->Count, Ptr, Count*sizeof(type));
    Array->Count += Count;
}

template <typename type>
inline void Array_Insert_Range(array<type>* Array, uptr InsertIndex, const type* Entries, uptr Count) {
    if(InsertIndex > Array->Count) {
        Array_Resize(Array, InsertIndex);
    } else if(InsertIndex == Count) {
        Array_Push_Range(Array, Entries, Count);
        return;
    } 
    
    Assert(InsertIndex <= Array->Count);
    uptr ArrayToCopy = Array->Count-InsertIndex;
    Resize(Array, Array->Count+Count);

    type* DstEnd = Array->Ptr+(Array->Count-1);
    const type* SrcEnd = Array->Ptr+((InsertIndex+ArrayToCopy)-1);
    for(uptr EntryIndex = 0; EntryIndex < ArrayToCopy; EntryIndex++) {
        Assert(DstEnd != SrcEnd);
        *DstEnd-- = *SrcEnd--;
    }

    SrcEnd = Entries+(Count-1);
    for(uptr EntryIndex = 0; EntryIndex < Count; EntryIndex++) {
        *DstEnd-- = *SrcEnd--;
    }

    Assert(DstEnd == Array->Ptr+InsertIndex);
}

template <typename type>
inline void Array_Remove(array<type>* Array, uptr Index) {
    Assert(Count > 0);
    uptr IndexPlusOne = Index+1;
    for(; IndexPlusOne < Array->Count; IndexPlusOne++) {
        Index = IndexPlusOne-1;
        Array->Ptr[Index] = Array->Ptr[IndexPlusOne];
    }
    Array->Count--;
}

template <typename type>
inline void Array_Swap_Remove(array<type>* Array, uptr Index) {
    Assert(Array->Count > 0 && Index < Array->Count);
    if(Index != (Array->Count-1)) {
        //Last the element we want to remove with the last entry
        Array->Ptr[Index] = Array->Ptr[Array->Count-1];
    }
    Array->Count--;
}

template <typename type>
inline type& Array_Last(array<type>* Array) {
    Assert(Array->Count);
    return Array->Ptr[Array->Count-1];
}

template <typename type, uptr N>
inline type& Array_Last(static_array<type, N>* Array) {
    Assert(N > 0);
    return Array->Array[N-1];
}

template <typename type, uptr N>
inline const type& Array_Last(const static_array<type, N>* Array) {
    Assert(N > 0);
    return Array->Array[N-1];
}

template <typename type>
inline type& Array_Pop(array<type>* Array) {
    type& Result = Array_Last(Array);
    Array->Count--;
    return Result;
}

template <typename type>
inline uptr Array_Byte_Size(array<type>* Array) {
    return Array->Count*sizeof(type);
} 

template <typename type>
inline void Array_Destruct(array<type>* Array) {
    if(Array->Ptr) {
        for(u32 i = 0; i < Array->Count; i++) {
            Array->Ptr[i].~type();
        }
        Array_Free(Array);
    }
}

template <typename type>
inline array<type>::array(allocator* _Allocator, uptr InitialCapacity) {
    Array_Init(this, _Allocator, InitialCapacity);
}

template <typename type>
struct array_scoped {
    array<type> Array;
    inline array_scoped(allocator* Allocator) {
        Array_Init(&Array, Allocator);
    }

    inline ~array_scoped() { 
        Array_Destruct(&Array); 
    }

    inline type& operator[](uptr Index) {
        return Array[Index];
    }

    inline array<type>* operator*() {
        return &Array;
    }

    inline array<type>* operator->() {
        return &Array;
    }
};

template <typename type>
inline void Array_Push(array_scoped<type>* Arr, const type& Entry) {
    Array_Push(&Arr->Array, Entry);
}

template <typename type>
inline void Array_Push(array_scoped<type>* Arr, type&& Entry) {
    Array_Push(&Arr->Array, Entry);
}

#endif