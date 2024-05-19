#ifndef VK_BUFFER_H
#define VK_BUFFER_H

struct vk_upload {
    VkBuffer     Buffer;
    VkDeviceSize Offset;
    VkDeviceSize Size;
};

struct vk_region {
    point2i Offset;
    dim2i   Size;
};

struct vk_buffer : vk_resource_base {
    VkBuffer               Handle;
    vk_allocation          Allocation;
    gdi_buffer_usage_flags UsageFlags;
    VkDeviceSize           Size;
    vk_upload              Upload;
    u8*                    Ptr;
};

internal bool VK_Create_Buffer(gdi_context* Context, vk_buffer* Buffer, const gdi_buffer_create_info& CreateInfo);
internal void VK_Delete_Buffer(gdi_context* Context, vk_buffer* Buffer);

#endif