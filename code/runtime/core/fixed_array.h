#ifndef FIXED_ARRAY_H
#define FIXED_ARRAY_H

template <typename type>
struct fixed_array {
    type*      Ptr = nullptr;
    uptr       Count = 0;

    fixed_array() = default;
    fixed_array(type* Ptr, uptr Count);
    fixed_array(allocator* Allocator, uptr Count);
    inline type& operator[](uptr Index) {
        Assert(Index < Count);
        return Ptr[Index];
    }

    inline type* begin() {
        return Ptr;
    }

    inline type* end() {
        return Ptr+Count;
    }
};

template <typename type>
void Array_Construct(fixed_array<type>* Array, allocator* Allocator, uptr Count) {
    Array->Ptr = new(Allocator) type[Count];
    Array->Count = Count;
}

template <typename type>
void Array_Destruct(fixed_array<type>* Array, allocator* Allocator) {
    if(Array->Ptr && Allocator) {
        for(u32 i = 0; i < Array->Count; i++) {
            Array->Ptr[i].~type();
        }
        operator delete[](Array->Ptr, Allocator);
        Array->Ptr = NULL;
        Array->Count = 0;
    }
}

template <typename type>
struct array_scoped {
    allocator*        Allocator;
    fixed_array<type> Array;
    inline array_scoped(allocator* _Allocator, uptr Count) {
        Allocator = _Allocator;
        Array_Construct(&Array, Allocator, Count);
    }

    inline ~array_scoped() { 
        if(Allocator) { 
            Array_Destruct(&Array, Allocator); 
            Allocator = NULL; 
        }
    }

    inline type& operator[](uptr Index) {
        return Array[Index];
    }

    inline fixed_array<type>* operator*() {
        return &Array;
    }

    inline fixed_array<type>* operator->() {
        return &Array;
    }
};

#endif