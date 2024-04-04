template <typename type>
inline void Array_Push_Range(array<type>* Array, span<type> Span) {
    Array_Push_Range(Array, Span.Ptr, Span.Count);
}