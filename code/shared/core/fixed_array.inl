template <typename type>
inline void Array_Init(fixed_array<type>* Array, type* Ptr, uptr Count) {
    Array->Ptr = Ptr;
    Array->Count = Count;
}

template <typename type>
inline void Array_Init(fixed_array<type>* Array, allocator* Allocator, span<type> Span) {
    type* Ptr = Allocator_Allocate_Array(Allocator, Span.Count, type);
    Array_Copy(Ptr, Span.Ptr, Span.Count);
    Array_Init(Array, Ptr, Span.Count);
}

template <typename type>
inline void Array_Init(fixed_array<type>* Array, allocator* Allocator, uptr Count) {
    Array->Ptr = Allocator_Allocate_Array(Allocator, Count, type);
    Array->Count = Count;
}

template <typename type>
inline void Array_Init(fixed_array<type>* Array, allocator* Allocator, const array<type>& SrcArray) {
    Array_Init(Array, Allocator, SrcArray.Count);
    Array_Copy(Array->Ptr, SrcArray.Ptr, SrcArray.Count);
}

template <typename type>
inline void Array_Free(fixed_array<type>* Array, allocator* Allocator) {
    if(Array->Ptr) {
        Allocator_Free_Memory(Allocator, Array->Ptr);
        Array->Ptr = NULL;
        Array->Count = 0;
    }
}

template <typename type>
inline fixed_array<type>::fixed_array(type* _Ptr, uptr _Count) {
    Array_Init(this, _Ptr, _Count);
}

template <typename type>
inline fixed_array<type>::fixed_array(allocator* Allocator, uptr Count) {
    Array_Init(this, Allocator, Count);
}

template <typename type>
inline fixed_array<type>::fixed_array(allocator* Allocator, const array<type>& Array) {
    Array_Init(this, Allocator, Array);
}

template <typename type>
inline fixed_array<type>::fixed_array(allocator* Allocator, span<type> Span) {
    Array_Init(this, Allocator, Span);
}


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
struct fixed_array_scoped {
    allocator*        Allocator;
    fixed_array<type> Array;
    inline fixed_array_scoped(allocator* _Allocator, uptr Count) {
        Allocator = _Allocator;
        Array_Construct(&Array, Allocator, Count);
    }

    inline ~fixed_array_scoped() { 
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