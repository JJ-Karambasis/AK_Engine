#ifndef DRAW_STREAM_H
#define DRAW_STREAM_H

#define RENDERER_DIRTY_FLAG_TYPE u32
#define RENDERER_MAX_DRAW_STREAM_SIZE MB(1)

using bind_group_indices = static_array<u32, RENDERER_MAX_BIND_GROUP>;
using dyn_bind_group_indices = static_array<u32, RENDERER_MAX_DYN_BIND_GROUP>;
using vtx_buffer_indices = static_array<u32, RENDERER_MAX_VTX_BUFFERS>;

namespace draw_bit_index {

    template <typename array>
    static constexpr inline array Compute_Indices_Offset(u32 LastIndex) {
        u32 CurrentIndex = LastIndex+1;
        array Result = {};
        for(uptr i = 0; i < Result.Count(); i++) {
            Result[i] = CurrentIndex++;
        }
        return Result;
    }

    static constexpr inline u32 Compute_Index_Offset(u32 LastIndex) {
        return LastIndex+1;
    }

    static const u32 PipelineIndex = 0;
    static const bind_group_indices BindGroupIndices = Compute_Indices_Offset<bind_group_indices>(PipelineIndex);
    static const dyn_bind_group_indices DynBindGroupIndices = Compute_Indices_Offset<dyn_bind_group_indices>(Array_Last(&BindGroupIndices));
    static const u32 IdxBufferIndex = Compute_Index_Offset(Array_Last(&DynBindGroupIndices));
    static const vtx_buffer_indices VtxBufferIndices = Compute_Indices_Offset<vtx_buffer_indices>(IdxBufferIndex);
    static const u32 IdxFormatIndex = Compute_Index_Offset(Array_Last(&VtxBufferIndices));
    static const u32 IdxOffsetIndex = Compute_Index_Offset(IdxFormatIndex);
    static const u32 VtxOffsetIndex = Compute_Index_Offset(IdxOffsetIndex);
    static const u32 InstOffsetIndex = Compute_Index_Offset(VtxOffsetIndex);
    static const u32 PrimCountIndex = Compute_Index_Offset(InstOffsetIndex);
    static const u32 InstCountIndex = Compute_Index_Offset(PrimCountIndex);
    static const u32 IsVtxDrawIndex = Compute_Index_Offset(InstCountIndex);
    static const dyn_bind_group_indices DynOffsetIndices = Compute_Indices_Offset<dyn_bind_group_indices>(IsVtxDrawIndex);
    static const u32 Count = Compute_Index_Offset(Array_Last(&DynOffsetIndices));
}

using bind_group_bits = static_array<RENDERER_DIRTY_FLAG_TYPE, RENDERER_MAX_BIND_GROUP>;
using dyn_bind_group_bits = static_array<RENDERER_DIRTY_FLAG_TYPE, RENDERER_MAX_DYN_BIND_GROUP>;
using vtx_buffer_bits = static_array<RENDERER_DIRTY_FLAG_TYPE, RENDERER_MAX_VTX_BUFFERS>;
namespace draw_bits {
    template <typename dst_array, typename src_array>
    static constexpr inline dst_array Compute_Bits(src_array Src) {
        static_assert(dst_array::Count() == src_array::Count());
        dst_array Dst = {};
        for(uptr i = 0; i < Dst.Count(); i++) {
            Dst[i] = (1u << Src[i]);
        }
        return Dst;
    }

    static const RENDERER_DIRTY_FLAG_TYPE Pipeline = (1 << draw_bit_index::PipelineIndex);
    static const bind_group_bits BindGroups = Compute_Bits<bind_group_bits>(draw_bit_index::BindGroupIndices);
    static const dyn_bind_group_bits DynBindGroups = Compute_Bits<dyn_bind_group_bits>(draw_bit_index::DynBindGroupIndices);
    static const RENDERER_DIRTY_FLAG_TYPE IdxBuffer = (1u << draw_bit_index::IdxBufferIndex);
    static const vtx_buffer_bits VtxBuffers = Compute_Bits<vtx_buffer_bits>(draw_bit_index::VtxBufferIndices);
    static const RENDERER_DIRTY_FLAG_TYPE IdxFormat = (1u << draw_bit_index::IdxFormatIndex);
    static const RENDERER_DIRTY_FLAG_TYPE IdxOffset = (1u << draw_bit_index::IdxOffsetIndex);
    static const RENDERER_DIRTY_FLAG_TYPE VtxOffset = (1u << draw_bit_index::VtxOffsetIndex);
    static const RENDERER_DIRTY_FLAG_TYPE InstOffset = (1u << draw_bit_index::InstOffsetIndex);
    static const RENDERER_DIRTY_FLAG_TYPE PrimCount = (1u << draw_bit_index::PrimCountIndex);
    static const RENDERER_DIRTY_FLAG_TYPE InstCount = (1u << draw_bit_index::InstCountIndex);
    static const RENDERER_DIRTY_FLAG_TYPE IsVtxDraw = (1u << draw_bit_index::IsVtxDrawIndex);
    static const dyn_bind_group_indices DynOffsets = Compute_Bits<dyn_bind_group_indices>(draw_bit_index::DynOffsetIndices);
    static const RENDERER_DIRTY_FLAG_TYPE Count = (1u << draw_bit_index::Count); //Hopefully this protects against overflow by compiler
};

struct draw {
    gdi_handle<gdi_pipeline>   Pipeline;
    gdi_handle<gdi_bind_group> BindGroups[RENDERER_MAX_BIND_GROUP];
    gdi_handle<gdi_bind_group> DynBindGroup[RENDERER_MAX_DYN_BIND_GROUP];
    gdi_handle<gdi_buffer>     IdxBuffer;
    gdi_handle<gdi_buffer>     VtxBuffers[RENDERER_MAX_VTX_BUFFERS];
    gdi_format                 IdxFormat;
    u32                        IdxOffset;
    u32                        VtxOffset;
    u32                        InstOffset;
    u32                        PrimCount;
    u32                        InstCount;
    b32                        IsVtxDraw;
    u32                        DynOffsets[RENDERER_MAX_DYN_BIND_GROUP];
};

struct draw_stream {
    //Stream data
    u8* Start;
    u8* End;
    u8* At;

    //Draw state
    RENDERER_DIRTY_FLAG_TYPE DirtyFlag;
    draw                     Draw;
};

void Draw_Stream_Set_Pipeline(draw_stream* Stream, gdi_handle<gdi_pipeline> Pipeline);
void Draw_Stream_Set_Bind_Groups(draw_stream* Stream, span<gdi_handle<gdi_bind_group>> BindGroups);
void Draw_Stream_Set_Dynamic_Bind_Groups(draw_stream* Stream, span<gdi_handle<gdi_bind_group>> BindGroups, span<u32> Offsets);
void Draw_Stream_Set_Vtx_Buffers(draw_stream* Stream, span<gdi_handle<gdi_buffer>> VtxBuffers);
void Draw_Stream_Set_Idx_Buffer(draw_stream* Stream, gdi_handle<gdi_buffer> IdxBuffer, gdi_format IdxFormat);
void Draw_Stream_Draw_Vtx(draw_stream* Stream, u32 VtxOffset, u32 InstOffset, u32 VtxCount, u32 InstCount);
void Draw_Stream_Draw_Idx(draw_stream* Stream, u32 IdxOffset, u32 VtxOffset, u32 InstOffset, u32 IdxCount, u32 InstCount);

#endif