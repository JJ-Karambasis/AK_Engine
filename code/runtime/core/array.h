#ifndef ARRAY_H
#define ARRAY_H

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
inline array<type>::array(allocator* _Allocator, uptr InitialCapacity) {
    Array_Init(this, _Allocator, InitialCapacity);
}

#endif