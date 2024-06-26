#ifndef VK_RENDER_PASS_H
#define VK_RENDER_PASS_H

struct vk_render_pass : vk_resource_base {
    VkRenderPass Handle;
};

bool VK_Create_Render_Pass(gdi_context* Context, vk_render_pass* RenderPass, const gdi_render_pass_create_info& CreateInfo);
void VK_Delete_Render_Pass(gdi_context* Context, vk_render_pass* RenderPass);
internal void VK_Render_Pass_Record_Frame(gdi_context* Context, async_handle<vk_render_pass> Handle);

struct vk_framebuffer : vk_resource_base {
    VkFramebuffer Handle;
    dim2i         Size;
};

bool VK_Create_Framebuffer(gdi_context* Context, vk_framebuffer* Framebuffer, const gdi_framebuffer_create_info& CreateInfo);
void VK_Delete_Framebuffer(gdi_context* Context, vk_framebuffer* Framebuffer);
fixed_array<gdi_handle<gdi_texture_view>> VK_Framebuffer_Get_Attachments(gdi_context* Context, vk_framebuffer* Framebuffer, allocator* Allocator);
internal void VK_Framebuffer_Record_Frame(gdi_context* Context, async_handle<vk_framebuffer> Handle);


#endif