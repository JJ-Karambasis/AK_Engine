#ifndef FIXED_ARRAY_H
#define FIXED_ARRAY_H

template <typename type>
struct array;

struct allocator;

template <typename type>
struct span;

template <typename type>
struct fixed_array {
    type*      Ptr = nullptr;
    uptr       Count = 0;

    fixed_array() = default;
    fixed_array(type* Ptr, uptr Count);
    fixed_array(allocator* Allocator, uptr Count);
    fixed_array(allocator* Allocator, const array<type>& Array);
    fixed_array(allocator* Allocator, span<type> Span);
    inline type& operator[](uptr Index) {
        Assert(Index < Count);
        return Ptr[Index];
    }

    inline const type& operator[](uptr Index) const {
        Assert(Index < Count);
        return Ptr[Index];
    }

    inline type* operator+(uptr Index) {
        Assert(Index < Count);
        return Ptr + Index;
    }

    inline const type* operator+(uptr Index) const {
        Assert(Index < Count);
        return Ptr + Index;
    }

    inline type* begin() {
        return Ptr;
    }

    inline type* end() {
        return Ptr+Count;
    }

    inline const type* begin() const {
        return Ptr;
    }

    inline const type* end() const {
        return Ptr+Count;
    }
};

#endif