template <typename type>
const_buffer::const_buffer(const span<type>& Span) {
    Ptr = (const u8*)Span.Ptr;
    Size = Span.Count*sizeof(type);
}