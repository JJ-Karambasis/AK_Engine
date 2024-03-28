#ifndef SPAN_H
#define SPAN_H

template <typename type>
struct span {
    const type* Ptr   = NULL;
    uptr        Count = 0;
    span() = default;
    span(const type* Ptr, uptr Count);
    span(const type* First, const type* Last);
    span(std::initializer_list<type> List);
    span(array<type> Array);

    inline const type& operator[](uptr Index) const {
        Assert(Index < Count);
        return Ptr[Index];
    }

    inline const type* begin() const {
        return Ptr;
    }

    inline const type* end() const {
        return Ptr+Count;
    }
};

template <typename type>
inline void Span_Init(span<type>* Span, const type* Ptr, uptr Count) {
    Span->Ptr   = Ptr;
    Span->Count = Count;
}

template <typename type>
inline void Span_Init(span<type>* Span, const type* First, const type* Last) {
    Span->Ptr   = First;
    Span->Count = (uptr)(Last-First);
}

template <typename type>
inline void Span_Init(span<type>* Span, std::initializer_list<type> List) {
    Span_Init(Span, List.begin(), List.end());
}

template <typename type>
inline void Span_Init(span<type>* Span, array<type> Array) {
    Span->Ptr = Array.Ptr;
    Span->Count = Array.Count;
}

template <typename type>
inline span<type>::span(const type* _Ptr, uptr _Count) {
    Span_Init(this, _Ptr, _Count);
}

template <typename type>
inline span<type>::span(const type* First, const type* Last) {
    Span_Init(this, First, Last);
}

template <typename type>
inline span<type>::span(std::initializer_list<type> List) {
    Span_Init(this, List);
}

template <typename type>
inline span<type>::span(array<type> Array) {
    Span_Init(this, Array);
}

#endif