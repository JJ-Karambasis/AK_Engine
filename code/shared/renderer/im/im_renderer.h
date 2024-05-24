#ifndef IM_RENDERER_H
#define IM_RENDERER_H

#include "im_stack.h"

struct im_renderer;
#define IM_MIN_BLOCK_SIZE KB(16)

struct im_vertex_p2_uv2_c {
    point2 P;
    point2 UV;
    color4 C;

    im_vertex_p2_uv2_c() = default;
    inline im_vertex_p2_uv2_c(const point2& _P, const point2& _UV, const color4& _C) : 
        P(_P), UV(_UV), C(_C) { }
};

struct im_buffer_block {
    gdi_handle<gdi_buffer> Buffer;
    u8*                    Ptr;
    uptr                   Size;
    uptr                   Used;
    im_buffer_block*       Next;
};

struct im_buffer {
    im_renderer*     Renderer;
    im_buffer_block* First;
    im_buffer_block* Last;
    im_buffer_block* Current;
    uptr             MinimumBlockSize;
};

#define IM_CALLBACK_DEFINE(name) void name(im_renderer* Renderer, dim2 Resolution, void* UserData)
typedef IM_CALLBACK_DEFINE(im_callback_func);

struct im_draw {
    gdi_handle<gdi_pipeline>   Pipeline;
    gdi_handle<gdi_bind_group> BindGroups[RENDERER_MAX_BIND_GROUP];
    im_buffer_block*           VtxBlock;
    u32                        VtxCount;
    u32                        VtxOffset;      
};

struct im_renderer {
    arena*    Arena;
    renderer* Renderer;
    im_buffer VtxBuffer;
    b32       Dirty;
    
    void*             UserData;
    im_callback_func* Callback;

    arena*         RenderArena;
    array<im_draw> Draws;
    render_task_id RenderTask;

    fixed_array<im_stack_list> Stacks;
};

struct im_renderer_create_info {
    allocator*        Allocator;
    renderer*         Renderer;
    im_callback_func* Callback;
    void*             UserData;
};

im_renderer* IM_Create(const im_renderer_create_info& CreateInfo);
void         IM_Reset(im_renderer* Renderer);
gdi_context* IM_Context(im_renderer* Renderer);

//Draw API
void IM_Draw_Quad(im_renderer* Renderer, point2 P1, point2 P2, point2 UV1, point2 UV2, color4 Color);
void IM_Draw_Quad(im_renderer* Renderer, rect2 Rect, rect2 UVRect, color4 Color);

//IM push stack API
void IM_Push_Pipeline(im_renderer* Renderer, gdi_handle<gdi_pipeline> Pipeline);
void IM_Push_Bind_Group(im_renderer* Renderer, u32 Index, gdi_handle<gdi_bind_group> BindGroup);

//IM pop stack API
void IM_Pop_Pipeline(im_renderer* Renderer);
void IM_Pop_Bind_Group(im_renderer* Renderer, u32 BindGroup);

//IM autopop API
void IM_Set_Next_Pipeline(im_renderer* Renderer, gdi_handle<gdi_pipeline> Pipeline);
void IM_Set_Next_Bind_Group(im_renderer* Renderer, u32 Index, gdi_handle<gdi_bind_group> BindGroup);

//IM get most recent stack api
im_stack_pipeline* IM_Current_Pipeline(im_renderer* Renderer);
im_stack_bind_group* IM_Current_Bind_Group(im_renderer* Renderer, u32 Index);
fixed_array<im_stack_bind_group*> IM_Current_Bind_Groups(im_renderer* Renderer);


#endif