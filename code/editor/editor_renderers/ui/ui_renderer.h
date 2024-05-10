#ifndef UI_RENDERER_H
#define UI_RENDERER_H

struct ui_render_pass {
    gdi_handle<gdi_render_pass> RenderPass;
};

struct ui_pipeline {
    gdi_handle<gdi_pipeline> Pipeline;
};

struct ui_box_instance {
    gdi_handle<gdi_bind_group> BindGroup;
    vec2                       Dim;
    u32                        InstanceCount;
};

struct ui_renderer {
    //Globals that are set on creation
    ui_pipeline            Pipeline;
    ui*                    UI;
    gdi_handle<gdi_buffer> InstanceBuffer;
    uptr                   InstanceCount;
    render_task_id         RenderTask;
};

ui_render_pass UI_Render_Pass_Create(renderer* Renderer, gdi_format Format);
ui_pipeline    UI_Pipeline_Create(renderer* Renderer, packages* Packages, ui_render_pass* RenderPass);
void           UI_Renderer_Create(ui_renderer* UIRenderer, renderer* Renderer, ui_pipeline* Pipeline, ui* UI);

#endif