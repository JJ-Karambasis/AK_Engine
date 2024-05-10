#ifndef DYNAMIC_BUFFER_H
#define DYNAMIC_BUFFER_H

struct dynamic_buffer;
struct dynamic_binding {
    gdi_handle<gdi_bind_group> BindGroup;
    u32                        Offset;
    void*                      Data;
};

dynamic_binding Dynamic_Buffer_Push(dynamic_buffer* Buffer, uptr Size);

template <typename type>
inline dynamic_binding Dynamic_Buffer_Push(dynamic_buffer* Buffer) {
    return Dynamic_Buffer_Push(Buffer, sizeof(type));
}

#endif