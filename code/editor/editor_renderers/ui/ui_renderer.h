#ifndef UI_RENDERER_H
#define UI_RENDERER_H

struct ui_render_pass {
    gdi_handle<gdi_render_pass> RenderPass;
};

struct ui_pipeline {
    gdi_handle<gdi_sampler>           Sampler;
    gdi_handle<gdi_bind_group_layout> BindGroupLayout;
    gdi_handle<gdi_pipeline>          Pipeline;
};

struct ui_renderer {
    //Globals that are set on creation
    gdi_context*   Context;
    ui_render_pass RenderPass;
    ui_pipeline    Pipeline;

    //Persistant updates per frame by the renderer
    gdi_handle<gdi_buffer>      GlobalBuffer;
    gdi_handle<gdi_bind_group>  GlobalBindGroup;
    gdi_handle<gdi_buffer>      InstanceBuffer;
    uptr                        InstanceCount;
    ui*                         UI;
    ui_font                     Font;

    //Per frame updates by the app
    uvec2                       FramebufferDim;
    gdi_handle<gdi_framebuffer> Framebuffer;
};

ui_render_pass UI_Render_Pass_Create(gdi_context* Context, gdi_format Format);
ui_pipeline    UI_Pipeline_Create(gdi_context* Context, packages* Packages, ui_render_pass* RenderPass);
void UI_Renderer_Create(ui_renderer* Renderer, gdi_context* Context, ui_render_pass* RenderPass, ui_pipeline* Pipeline, ui* UI, ui_font Font);
void UI_Renderer_Update(ui_renderer* Renderer, gdi_cmd_list* CmdList);
void UI_Renderer_Set_Framebuffer(ui_renderer* Renderer, gdi_handle<gdi_framebuffer> Framebuffer, uvec2 FramebufferDim);

#endif