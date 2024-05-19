#ifndef RENDERER_H
#define RENDERER_H

struct renderer;

#include <render_options.h>
#include "draw_stream.h"
#include "dynamic_buffer.h"
#include "textures.h"

struct renderer_create_info {
    allocator*              Allocator = Core_Get_Base_Allocator();
    ak_job_system*          JobSystem;
    gdi_context*            Context;
    gdi_sampler_create_info DefaultSamplerInfo;
};

typedef u64 render_task_id;

struct render_graph_id {
    u64                  ID;
    struct render_graph* Graph;
};

#define DRAW_CALLBACK(name) void name(ak_job_system* JobSystem, gdi_context* Context, draw_stream* DrawStream, dynamic_buffer* DynamicBuffer, dim2 Resolution, void* UserData)
typedef DRAW_CALLBACK(draw_callback_func);

struct draw_callback_data {
    draw_callback_func* Callback;
    void*               UserData;
};

renderer* Renderer_Create(const renderer_create_info& CreateInfo);
void      Renderer_Delete(renderer* Renderer);
bool      Renderer_Execute(renderer* Renderer, render_graph_id RenderGraph, span<gdi_handle<gdi_swapchain>> Swapchains);
gdi_context* Renderer_Get_Context(renderer* Renderer);
gdi_handle<gdi_bind_group_layout> Renderer_Get_Dynamic_Layout(renderer* Renderer);
gdi_handle<gdi_bind_group_layout> Renderer_Get_Texture_Layout(renderer* Renderer);

//todo: Draw indirect tasks for consoles and pcs (gpu driven)
//todo: Computer shaders to take advantage of async compute (skinning)
render_task_id Renderer_Create_Draw_Task(renderer* Renderer, draw_callback_func* Callback, void* UserData);
void           Renderer_Delete_Task(renderer* Renderer, render_task_id TaskID);

render_graph_id Renderer_Create_Graph(renderer* Renderer);
void            Renderer_Delete_Graph(renderer* Renderer, render_graph_id Graph);

void Render_Graph_Add_Task(render_graph_id RenderGraph, render_task_id Task, render_task_id ParentTask);
void Render_Graph_Clear(render_graph_id RenderGraph);

void Render_Task_Attach_Render_Pass(renderer* Renderer, render_task_id Task, const gdi_render_pass_begin_info& RenderPassInfo, span<gdi_resource_state> AttachmentStates);

#endif