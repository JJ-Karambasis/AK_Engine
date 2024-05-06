#ifndef DRAW_STREAM_H
#define DRAW_STREAM_H

#define RENDERER_MAX_BIND_GROUP 3
#define RENDERER_MAX_DYN_BIND_GROUP 1
#define RENDERER_MAX_VTX_BUFFERS 3
#define RENDERER_MAX_DRAW_STREAM_SIZE MB(1)

struct draw {
    gdi_handle<gdi_pipeline>   Pipeline;
    gdi_handle<gdi_bind_group> BindGroups[RENDERER_MAX_BIND_GROUP];
    gdi_handle<gdi_bind_group> DynBindGroup[RENDERER_MAX_DYN_BIND_GROUP];
    gdi_handle<gdi_buffer>     IdxBuffer;
    gdi_handle<gdi_buffer>     VtxBuffers[RENDERER_MAX_VTX_BUFFERS];
    u32                        IdxOffset;
    u32                        VtxOffset;
    u32                        InstOffset;
    u32                        PrimCount;
    u32                        InstCount;
    b32                        IsVtxDraw;
    u32                        DynOffsets[RENDERER_MAX_DYN_BIND_GROUP];
};

struct draw_stream {
    u8* Start;
    u8* End;
    u8* At;
};

void Draw_Stream_Set_Pipeline(draw_stream* Stream, gdi_handle<gdi_pipeline> Pipeline);
void Draw_Stream_Set_Bind_Groups(draw_stream* Stream, span<gdi_handle<gdi_bind_group>> BindGroups);
void Draw_Stream_Set_Dynamic_Bind_Groups(draw_stream* Stream, span<gdi_handle<gdi_bind_group>> BindGroups, span<u32> Offsets);
void Draw_Stream_Set_Vtx_Buffers(draw_stream* Stream, span<gdi_handle<gdi_buffer>> VtxBuffers);
void Draw_Stream_Set_Idx_Buffer(draw_stream* Stream, gdi_handle<gdi_buffer> IdxBuffer);
void Draw_Stream_Draw_Vtx(draw_stream* Stream, u32 VtxOffset, u32 InstOffset, u32 VtxCount, u32 InstCount);
void Draw_Stream_Draw_Idx(draw_stream* Stream, u32 IdxOffset, u32 VtxOffset, u32 InstOffset, u32 IdxCount, u32 InstCount);

#endif