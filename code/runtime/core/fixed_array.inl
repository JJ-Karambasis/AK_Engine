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
