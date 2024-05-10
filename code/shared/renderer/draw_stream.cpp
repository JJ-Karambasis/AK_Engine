template <typename type>
internal inline void Draw_Stream_Write_Entry(draw_stream* DrawStream, const type* Entry) {
    Assert(DrawStream->At+sizeof(type) <= DrawStream->End);
    Memory_Copy(DrawStream->At, Entry, sizeof(type));
    DrawStream->At += sizeof(type);
}

template <typename type>
internal inline void Draw_Stream_Write_Handle(draw_stream* Stream, gdi_handle<type> Handle) {
    Draw_Stream_Write_Entry(Stream, &Handle.ID);
}

internal inline void Draw_Stream_Write_U32(draw_stream* Stream, u32 Value) {
    Draw_Stream_Write_Entry(Stream, &Value);
}

internal void Draw_Stream_Write(draw_stream* Stream) {
    //Always write the draw stream at the beginning of each entry
    Draw_Stream_Write_Entry(Stream, &Stream->DirtyFlag);
    draw* Draw = &Stream->Draw;

    //Then only write out the changes to the stream using the dirty flag
    if(Stream->DirtyFlag & draw_bits::Pipeline) {
        Draw_Stream_Write_Handle(Stream, Draw->Pipeline);
    }

    for(uptr i = 0; i < RENDERER_MAX_BIND_GROUP; i++) {
        if(Stream->DirtyFlag & draw_bits::BindGroups[i]) {
            Draw_Stream_Write_Handle(Stream, Draw->BindGroups[i]);
        }
    }

    for(uptr i = 0; i < RENDERER_MAX_DYN_BIND_GROUP; i++) {
        if(Stream->DirtyFlag & draw_bits::DynBindGroups[i]) {
            Draw_Stream_Write_Handle(Stream, Draw->DynBindGroup[i]);
        }
    }

    if(Stream->DirtyFlag & draw_bits::IdxBuffer) {
        Draw_Stream_Write_Handle(Stream, Draw->IdxBuffer);
    }

    for(uptr i = 0; i < RENDERER_MAX_VTX_BUFFERS; i++) {
        if(Stream->DirtyFlag & draw_bits::VtxBuffers[i]) {
            Draw_Stream_Write_Handle(Stream, Draw->VtxBuffers[i]);
        }
    }
    
    if(Stream->DirtyFlag & draw_bits::IdxFormat) {
        Draw_Stream_Write_U32(Stream, Draw->IdxFormat);
    }

    if(Stream->DirtyFlag & draw_bits::IdxOffset) {
        Draw_Stream_Write_U32(Stream, Draw->IdxOffset);
    }

    if(Stream->DirtyFlag & draw_bits::VtxOffset) {
        Draw_Stream_Write_U32(Stream, Draw->VtxOffset);
    }

    if(Stream->DirtyFlag & draw_bits::InstOffset) {
        Draw_Stream_Write_U32(Stream, Draw->InstOffset);
    }

    if(Stream->DirtyFlag & draw_bits::PrimCount) {
        Draw_Stream_Write_U32(Stream, Draw->PrimCount);
    }

    if(Stream->DirtyFlag & draw_bits::InstCount) {
        Draw_Stream_Write_U32(Stream, Draw->InstCount);
    }

    if(Stream->DirtyFlag & draw_bits::IsVtxDraw) {
        Draw_Stream_Write_U32(Stream, (u32)Draw->IsVtxDraw);
    }

    for(uptr i = 0; i < RENDERER_MAX_DYN_BIND_GROUP; i++) {
        if(Stream->DirtyFlag & draw_bits::DynOffsets[i]) {
            Draw_Stream_Write_U32(Stream, Draw->DynOffsets[i]);
        }
    }

    Stream->DirtyFlag = 0;
}

void Draw_Stream_Set_Pipeline(draw_stream* Stream, gdi_handle<gdi_pipeline> Pipeline) {
    if(Stream->Draw.Pipeline != Pipeline) {
        Stream->DirtyFlag |= draw_bits::Pipeline;
        Stream->Draw.Pipeline = Pipeline;
    }
}

void Draw_Stream_Set_Bind_Groups(draw_stream* Stream, span<gdi_handle<gdi_bind_group>> BindGroups) {
    Assert(BindGroups.Count <= RENDERER_MAX_BIND_GROUP);
    for(uptr i = 0; i < BindGroups.Count; i++) {
        if(Stream->Draw.BindGroups[i] != BindGroups[i]) {
            Stream->DirtyFlag |= draw_bits::BindGroups[i];
            Stream->Draw.BindGroups[i] = BindGroups[i];
        }
    }
}

void Draw_Stream_Set_Dynamic_Bind_Groups(draw_stream* Stream, span<gdi_handle<gdi_bind_group>> BindGroups, span<u32> Offsets) {
    Assert(BindGroups.Count <= RENDERER_MAX_DYN_BIND_GROUP);
    Assert(Offsets.Count == BindGroups.Count);

    for(uptr i = 0; i < BindGroups.Count; i++) {
        if(Stream->Draw.DynBindGroup[i] != BindGroups[i]) {
            Stream->DirtyFlag |= draw_bits::DynBindGroups[i];
            Stream->Draw.DynBindGroup[i] = BindGroups[i];
        }
    }

    for(uptr i = 0; i < Offsets.Count; i++) {
        if(Stream->Draw.DynOffsets[i] != Offsets[i]) {
            Stream->DirtyFlag |= draw_bits::DynOffsets[i];
            Stream->Draw.DynOffsets[i] = Offsets[i];
        }
    }
}

void Draw_Stream_Set_Vtx_Buffers(draw_stream* Stream, span<gdi_handle<gdi_buffer>> VtxBuffers) {
    Assert(VtxBuffers.Count < RENDERER_MAX_VTX_BUFFERS);

    for(uptr i = 0; i < VtxBuffers.Count; i++) {
        if(Stream->Draw.VtxBuffers[i] != VtxBuffers[i]) {
            Stream->DirtyFlag |= draw_bits::VtxBuffers[i];
            Stream->Draw.VtxBuffers[i] = VtxBuffers[i];
        }
    }
}

void Draw_Stream_Set_Idx_Buffer(draw_stream* Stream, gdi_handle<gdi_buffer> IdxBuffer, gdi_format IdxFormat) {
    Assert(GDI_Is_Index_Format(IdxFormat));

    if(Stream->Draw.IdxBuffer != IdxBuffer) {
        Stream->DirtyFlag |= draw_bits::IdxBuffer;
        Stream->Draw.IdxBuffer = IdxBuffer;
    }

    if(Stream->Draw.IdxFormat != IdxFormat) {
        Stream->DirtyFlag |= draw_bits::IdxFormat;
        Stream->Draw.IdxFormat = IdxFormat;
    }
}

void Draw_Stream_Draw_Vtx(draw_stream* Stream, u32 VtxOffset, u32 InstOffset, u32 VtxCount, u32 InstCount) {
    if(Stream->Draw.VtxOffset != VtxOffset) {
        Stream->DirtyFlag |= draw_bits::VtxOffset;
        Stream->Draw.VtxOffset = VtxOffset;
    }

    if(Stream->Draw.InstOffset != InstOffset) {
        Stream->DirtyFlag |= draw_bits::InstOffset;
        Stream->Draw.InstOffset = InstOffset;
    }

    if(Stream->Draw.PrimCount != VtxCount) {
        Stream->DirtyFlag |= draw_bits::PrimCount;
        Stream->Draw.PrimCount = VtxCount;
    }

    if(Stream->Draw.InstCount != InstCount) {
        Stream->DirtyFlag |= draw_bits::InstCount;
        Stream->Draw.InstCount = InstCount;
    }

    if(Stream->Draw.IsVtxDraw != true32) {
        Stream->DirtyFlag |= draw_bits::IsVtxDraw;
        Stream->Draw.IsVtxDraw = true32;
    }

    Draw_Stream_Write(Stream);
}

void Draw_Stream_Draw_Idx(draw_stream* Stream, u32 IdxOffset, u32 VtxOffset, u32 InstOffset, u32 IdxCount, u32 InstCount) {
    if(Stream->Draw.IdxOffset != IdxOffset) {
        Stream->DirtyFlag |= draw_bits::IdxOffset;
        Stream->Draw.IdxOffset = IdxOffset;
    }

    if(Stream->Draw.VtxOffset != VtxOffset) {
        Stream->DirtyFlag |= draw_bits::VtxOffset;
        Stream->Draw.VtxOffset = VtxOffset;
    }

    if(Stream->Draw.InstOffset != InstOffset) {
        Stream->DirtyFlag |= draw_bits::InstOffset;
        Stream->Draw.InstOffset = InstOffset;
    }

    if(Stream->Draw.PrimCount != IdxCount) {
        Stream->DirtyFlag |= draw_bits::PrimCount;
        Stream->Draw.PrimCount = IdxCount;
    }

    if(Stream->Draw.InstCount != InstCount) {
        Stream->DirtyFlag |= draw_bits::InstCount;
        Stream->Draw.InstCount = InstCount;
    }

    if(Stream->Draw.IsVtxDraw != false32) {
        Stream->DirtyFlag |= draw_bits::IsVtxDraw;
        Stream->Draw.IsVtxDraw = false32;
    }

    Draw_Stream_Write(Stream);
}