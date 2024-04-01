#ifndef VK_BUFFER_H
#define VK_BUFFER_H

struct vk_buffer {
    VkBuffer      Buffer;
    vk_allocation Allocation;
};

internal bool VK_Create_Buffer(gdi_context* Context, vk_buffer* Buffer, const gdi_buffer_create_info& CreateInfo);
internal void VK_Delete_Buffer(gdi_context* Context, vk_buffer* Buffer);
internal bool VK_Buffer_Upload(gdi_context* Context, async_handle<vk_buffer> Handle, const_buffer Data);
internal void VK_Buffer_Record_Frame(gdi_context* Context, async_handle<vk_buffer> Handle);

#endif