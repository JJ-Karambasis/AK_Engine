#ifndef FIXED_ARRAY_H
#define FIXED_ARRAY_H

template <typename type>
struct fixed_array {
    type*      Ptr = nullptr;
    uptr       Count = 0;

    fixed_array() = default;
    fixed_array(type* Ptr, uptr Count);
    inline type& operator[](uptr Index) {
        Assert(Index < Count);
        return Ptr[Index];
    }
};

template <typename type>
inline void Array_Init(fixed_array<type>* Array, type* Ptr, uptr Count) {
    Array->Ptr = Ptr;
    Array->Count = Count;
}

template <typename type>
inline fixed_array<type>::fixed_array(type* _Ptr, uptr _Count) {
    Array_Init(this, _Ptr, _Count);
}

#endif