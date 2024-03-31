#ifndef VK_RENDER_PASS_H
#define VK_RENDER_PASS_H

struct vk_render_pass {
    VkRenderPass RenderPass;
};

bool VK_Create_Render_Pass(gdi_context* Context, vk_render_pass* RenderPass, const gdi_render_pass_create_info& CreateInfo);
void VK_Delete_Render_Pass(gdi_context* Context, vk_render_pass* RenderPass);
internal void VK_Render_Pass_Record_Frame(gdi_context* Context, async_handle<vk_render_pass> Handle);

struct vk_framebuffer {
    fixed_array<gdi_handle<gdi_texture_view>> Attachments;
    VkFramebuffer                             Framebuffer;
    u32                                       Width;
    u32                                       Height;
};

bool VK_Create_Framebuffer(gdi_context* Context, vk_framebuffer* Framebuffer, const gdi_framebuffer_create_info& CreateInfo);
void VK_Delete_Framebuffer(gdi_context* Context, vk_framebuffer* Framebuffer);
internal void VK_Framebuffer_Record_Frame(gdi_context* Context, async_handle<vk_framebuffer> Handle);


#endif