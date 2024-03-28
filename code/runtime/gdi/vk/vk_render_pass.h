#ifndef VK_RENDER_PASS_H
#define VK_RENDER_PASS_H

struct vk_render_pass : public vk_resource {
    VkRenderPass RenderPass;
};

bool VK_Create_Render_Pass(gdi_context* Context, vk_render_pass* RenderPass, const gdi_render_pass_create_info& CreateInfo);
void VK_Delete_Render_Pass(gdi_context* Context, vk_render_pass* RenderPass);


struct vk_framebuffer : public vk_resource {
    fixed_array<gdi_handle<gdi_texture_view>> Attachments;
    VkFramebuffer                             Framebuffer;
};

bool VK_Create_Framebuffer(gdi_context* Context, vk_framebuffer* Framebuffer, const gdi_framebuffer_create_info& CreateInfo);
void VK_Delete_Framebuffer(gdi_context* Context, vk_framebuffer* Framebuffer);

#endif