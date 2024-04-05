#ifndef VK_BIND_GROUPS_H
#define VK_BIND_GROUPS_H

struct vk_bind_group_layout {
    array<async_handle<vk_sampler>> Samplers;
    VkDescriptorSetLayout           SetLayout;
};

internal bool VK_Create_Bind_Group_Layout(gdi_context* Context, vk_bind_group_layout* Layout, const gdi_bind_group_layout_create_info& CreateInfo);
internal void VK_Delete_Bind_Group_Layout(gdi_context* Context, vk_bind_group_layout* Layout);
internal void VK_Bind_Group_Layout_Record_Frame(gdi_context* Context, async_handle<vk_bind_group_layout> Handle);

struct vk_bind_group {
    async_handle<vk_bind_group_layout> BindGroupLayout;
    VkDescriptorSet                    DescriptorSet;
    array<uptr>                        DynamicOffsets;
};

internal bool VK_Create_Bind_Group(gdi_context* Context, vk_bind_group* BindGroup, gdi_handle<gdi_bind_group_layout> Layout);
internal void VK_Delete_Bind_Group(gdi_context* Context, vk_bind_group* BindGroup);
internal bool VK_Bind_Group_Write(gdi_context* Context, vk_bind_group* BindGroup, const gdi_bind_group_write_info& WriteInfo);
internal void VK_Bind_Group_Record_Frame(gdi_context* Context, async_handle<vk_bind_group> Handle);

struct vk_descriptor_pool {
    ak_mutex               Lock;
    VkDevice               Device;
    VkAllocationCallbacks* VKAllocator;
    VkDescriptorPool       Pool;
};

internal bool VK_Descriptor_Pool_Create(gdi_context* Context, vk_descriptor_pool* Pool);
internal void VK_Descriptor_Pool_Delete(vk_descriptor_pool* Pool);
internal bool VK_Descriptor_Pool_Allocate(vk_descriptor_pool* Pool, VkDescriptorSetLayout SetLayout, VkDescriptorSet* OutDescriptorSet);
internal void VK_Descriptor_Pool_Free(vk_descriptor_pool* Pool, VkDescriptorSet DescriptorSet);

#endif