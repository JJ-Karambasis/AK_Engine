#ifndef VK_BUFFER_H
#define VK_BUFFER_H

struct vk_upload {
    VkBuffer     Buffer;
    VkDeviceSize Offset;
    VkDeviceSize Size;
};

struct vk_buffer {
    VkBuffer               Buffer;
    vk_allocation          Allocation;
    gdi_buffer_usage_flags UsageFlags;
    VkDeviceSize           Size;
    vk_upload              Upload;
    u8*                    Ptr;
};

internal bool VK_Create_Buffer(gdi_context* Context, vk_buffer* Buffer, const gdi_buffer_create_info& CreateInfo);
internal void VK_Delete_Buffer(gdi_context* Context, vk_buffer* Buffer);
internal void VK_Buffer_Record_Frame(gdi_context* Context, async_handle<vk_buffer> Handle);

#endif