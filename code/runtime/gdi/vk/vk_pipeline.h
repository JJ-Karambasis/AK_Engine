#ifndef VK_PIPELINE_H
#define VK_PIPELINE_H

struct vk_pipeline {
    VkPipelineBindPoint BindPoint;
    VkPipelineLayout    Layout;
    VkPipeline          Pipeline;
};

internal bool VK_Create_Pipeline(gdi_context* Context, vk_pipeline* Pipeline, const gdi_graphics_pipeline_create_info& CreateInfo);
internal void VK_Delete_Pipeline(gdi_context* Context, vk_pipeline* Pipeline);
internal void VK_Pipeline_Record_Frame(gdi_context* Context, async_handle<vk_pipeline> Handle);

#endif