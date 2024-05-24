#include "im_stack.cpp"

#define IM_Current_Stack_Entry(constant, type) (type*)(Renderer->Stacks[constant].Last)


internal im_buffer_block* IM_Buffer_Get_Current_Block(im_buffer* Buffer, uptr Size, uptr Alignment) {
    im_buffer_block* Result = Buffer->Current != nullptr ? Buffer->Current : Buffer->First;
    while(Result && (Result->Size < (Align(Alignment, Result->Used)+Size))) {
        Result = Result->Next;
    }
    return Result;
}

internal im_buffer_block* IM_Buffer_Allocate_Block(im_renderer* IMRenderer, uptr BlockSize) {
    renderer* Renderer = IMRenderer->Renderer;
    im_buffer_block* Block = Arena_Push_Struct(IMRenderer->Arena, im_buffer_block);
    Block->Buffer = GDI_Context_Create_Buffer(Renderer->Context, {
        .ByteSize = BlockSize,
        .UsageFlags = GDI_BUFFER_USAGE_FLAG_VTX_BUFFER_BIT|GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT
    });
    Block->Size = BlockSize;
    return Block;
}

struct im_push_internal {
    void*            Data;
    uptr             DataOffset;
    im_buffer_block* VtxBlock;
};

template <typename type>
struct im_push : im_push_internal {
    inline u32 VtxOffset() {
        return Safe_U32(DataOffset/sizeof(type));
    } 

    inline void Write(u32 Index, const type& Vtx) {
        type* Array = (type*)Data;
        Array[Index] = Vtx;
    }
};

internal void IM_Buffer_Push(im_buffer* Buffer, uptr Size, uptr Alignment, im_push_internal* Push) {
    im_buffer_block* Block = IM_Buffer_Get_Current_Block(Buffer, Size, Alignment);
    if(!Block) {
        uptr BlockSize = 0;
        if(Buffer->Last) {
            BlockSize = Buffer->Last->Size*2;
        }

        if(BlockSize < Size+Alignment) {
            BlockSize = Ceil_Pow2(Size+Alignment);
        }

        //If the blocksize is less than the minimum block size we will round up
        BlockSize = Max(BlockSize, Buffer->MinimumBlockSize);

        Block = IM_Buffer_Allocate_Block(Buffer->Renderer, BlockSize);
        if(!Block) {
            Assert(false);
            return;
        }
        SLL_Push_Back(Buffer->First, Buffer->Last, Block);
    }

    if(Buffer->Current != Block) {
        Buffer->Renderer->Dirty = true;
    }

    Buffer->Current = Block;
    im_buffer_block* CurrentBlock = Buffer->Current;

    CurrentBlock->Used = Align(Alignment, CurrentBlock->Used);
    Assert(CurrentBlock->Used+Size <= CurrentBlock->Size);

    if(!CurrentBlock->Ptr) {
        CurrentBlock->Ptr = GDI_Context_Buffer_Map(Buffer->Renderer->Renderer->Context, CurrentBlock->Buffer);
        if(!CurrentBlock->Ptr) {
            Assert(false);
            return;
        }
    }

    Push->DataOffset = CurrentBlock->Used;
    Push->Data = CurrentBlock->Ptr + CurrentBlock->Used;
    Push->VtxBlock = CurrentBlock;

    CurrentBlock->Used += Size;
}

template <typename type>
internal inline im_push<type> IM_Buffer_Push(im_buffer* Buffer, uptr Count) {
    im_push<type> Result = {};
    IM_Buffer_Push(Buffer, sizeof(type)*Count, sizeof(type), &Result);
    return Result;
}

internal void IM_Buffer_Create(im_renderer* Renderer, im_buffer* Buffer, uptr InitialBlockSize) {
    Zero_Struct(Buffer);
    Buffer->Renderer = Renderer;
    Buffer->MinimumBlockSize = InitialBlockSize;
}


internal void IM_Reset_Buffer(im_renderer* Renderer, im_buffer* Buffer) {
    for(im_buffer_block* Block = Buffer->First; Block; Block = Block->Next) {
        Block->Used = 0;
        if(Block->Ptr) {
            GDI_Context_Buffer_Unmap(Renderer->Renderer->Context, Block->Buffer);
            Block->Ptr = nullptr;
        }
    }
    Buffer->Current = nullptr;
}

internal DRAW_CALLBACK(IM_Draw_Callback) {
    im_renderer* Renderer = (im_renderer*)UserData;
    Renderer->Callback(Renderer, Resolution, Renderer->UserData);

    for(const im_draw& Draw : Renderer->Draws) {
        Draw_Stream_Set_Pipeline(DrawStream, Draw.Pipeline);
        Draw_Stream_Set_Vtx_Buffers(DrawStream, {Draw.VtxBlock->Buffer});
        Draw_Stream_Set_Bind_Groups(DrawStream, Draw.BindGroups);
        Draw_Stream_Draw_Vtx(DrawStream, Draw.VtxOffset, 0, Draw.VtxCount, 1);
    }

    IM_Reset(Renderer);
}

im_renderer* IM_Create(const im_renderer_create_info& CreateInfo) {
    arena* Arena = Arena_Create(CreateInfo.Allocator);
    im_renderer* Renderer = Arena_Push_Struct(Arena, im_renderer);
    Renderer->Arena = Arena;
    Renderer->Renderer = CreateInfo.Renderer;
    IM_Buffer_Create(Renderer, &Renderer->VtxBuffer, KB(16));

    Renderer->UserData = CreateInfo.UserData;
    Renderer->Callback = CreateInfo.Callback;

    Renderer->RenderArena = Arena_Create(Renderer->Arena);
    Renderer->RenderTask = Renderer_Create_Draw_Task(Renderer->Renderer, IM_Draw_Callback, Renderer);
    Array_Init(&Renderer->Stacks, Renderer->Arena, im_stack_type::Count); 

    IM_Reset(Renderer);

    return Renderer;
}

void IM_Reset(im_renderer* Renderer) {
    IM_Reset_Buffer(Renderer, &Renderer->VtxBuffer);
    Arena_Clear(Renderer->RenderArena);
    Array_Init(&Renderer->Draws, Renderer->RenderArena);
    IM_Reset_Stacks(Renderer);
}

gdi_context* IM_Context(im_renderer* Renderer) {
    return Renderer_Get_Context(Renderer->Renderer);
}

internal void IM_Get_All_Bind_Groups(im_renderer* Renderer, gdi_handle<gdi_bind_group>* BindGroups) {
    for(uptr i = 0; i < RENDERER_MAX_BIND_GROUP; i++) {
        im_stack_bind_group* BindGroupStack = IM_Current_Stack_Entry(im_stack_type::BindGroups[i], im_stack_bind_group);
        if(BindGroupStack) {
            BindGroups[i] = BindGroupStack->Value;
        }
    }
}

void IM_Draw_Quad(im_renderer* Renderer, point2 P1, point2 P2, point2 UV1, point2 UV2, color4 Color) {
    const u32 VtxCount = 6;

    im_push<im_vertex_p2_uv2_c> Push = IM_Buffer_Push<im_vertex_p2_uv2_c>(&Renderer->VtxBuffer, VtxCount);

    if(Renderer->Dirty) {
        Renderer->Dirty = false;

        im_draw Draw = {
            .Pipeline = IM_Current_Pipeline(Renderer)->Value,
            .VtxBlock = Push.VtxBlock,
            .VtxOffset = Push.VtxOffset()
        };

        gdi_handle<gdi_bind_group> BindGroups[RENDERER_MAX_BIND_GROUP] = {};
        IM_Get_All_Bind_Groups(Renderer, BindGroups);

        Array_Copy(Draw.BindGroups, BindGroups, RENDERER_MAX_BIND_GROUP);
        Array_Push(&Renderer->Draws, Draw);
    }

    im_draw* Draw = &Array_Last(&Renderer->Draws);
    Draw->VtxCount += VtxCount;

    Push.Write(0, im_vertex_p2_uv2_c(P1, UV1, Color));
    Push.Write(1, im_vertex_p2_uv2_c(point2(P1.x, P2.y), point2(UV1.x, UV2.y), Color));
    Push.Write(2, im_vertex_p2_uv2_c(P2, UV2, Color));
    
    Push.Write(3, im_vertex_p2_uv2_c(P2, UV2, Color));
    Push.Write(4, im_vertex_p2_uv2_c(point2(P2.x, P1.y), point2(UV2.x, UV1.y), Color));
    Push.Write(5, im_vertex_p2_uv2_c(P1, UV1, Color));

    IM_Autopop_Stacks(Renderer);
}
 
void IM_Draw_Quad(im_renderer* Renderer, rect2 PosRect, rect2 UVRect, color4 Color) {
    IM_Draw_Quad(Renderer, PosRect.P1, PosRect.P2, UVRect.P1, UVRect.P2, Color);
}

#define IM_Pop_Type(type) IM_Stack_List_Pop(&Renderer->Stacks[type])
#define IM_Push_Type_Dirty_Raw(constant, type, data_value, autopush) \
    im_stack_list* List = IM_Get_Stack_List(Renderer, constant); \
    if(List->Last) { \
        type* OldLast = (type*)List->Last; \
        if(OldLast->Value != data_value) \
            Renderer->Dirty = true; \
    } else { \
        Renderer->Dirty = true; \
    } \
    type* Entry = (type*)IM_Push_Stack(Renderer, constant, autopush); \
    Entry->Value = data_value

#define IM_Push_Type_Dirty(constant, type, data_value) \
    IM_Push_Type_Dirty_Raw(constant, type, data_value, false)
#define IM_Autopush_Type_Dirty(constant, type, data_value) \
    IM_Push_Type_Dirty_Raw(constant, type, data_value, true)

//IM push stack API
void IM_Push_Pipeline(im_renderer* Renderer, gdi_handle<gdi_pipeline> Pipeline) {
    IM_Push_Type_Dirty(im_stack_type::Pipeline, im_stack_pipeline, Pipeline);
}

void IM_Push_Bind_Group(im_renderer* Renderer, u32 Index, gdi_handle<gdi_bind_group> BindGroup) {
    IM_Push_Type_Dirty(im_stack_type::BindGroups[Index], im_stack_bind_group, BindGroup);
}

//IM pop stack API
void IM_Pop_Pipeline(im_renderer* Renderer) {
    IM_Pop_Type(im_stack_type::Pipeline);
}

void IM_Pop_Bind_Group(im_renderer* Renderer, u32 Index) {
    IM_Pop_Type(im_stack_type::BindGroups[Index]);
}

//IM autopop API
void IM_Set_Next_Pipeline(im_renderer* Renderer, gdi_handle<gdi_pipeline> Pipeline) {
    IM_Autopush_Type_Dirty(im_stack_type::Pipeline, im_stack_pipeline, Pipeline);
}

void IM_Set_Next_Bind_Group(im_renderer* Renderer, u32 Index, gdi_handle<gdi_bind_group> BindGroup) {
    IM_Autopush_Type_Dirty(im_stack_type::BindGroups[Index], im_stack_bind_group, BindGroup);
}

//IM get most recent stack api
im_stack_pipeline* IM_Current_Pipeline(im_renderer* Renderer) {
    return IM_Current_Stack_Entry(im_stack_type::Pipeline, im_stack_pipeline);
}

im_stack_bind_group* IM_Current_Bind_Group(im_renderer* Renderer, u32 Index) {
    return IM_Current_Stack_Entry(im_stack_type::BindGroups[Index], im_stack_bind_group);
}

fixed_array<im_stack_bind_group*> IM_Current_Bind_Groups(im_renderer* Renderer) {
    fixed_array<im_stack_bind_group*> BindGroups(Renderer->RenderArena, RENDERER_MAX_BIND_GROUP);
    for(uptr i = 0; i < RENDERER_MAX_BIND_GROUP; i++) {
        BindGroups[i] = IM_Current_Stack_Entry(im_stack_type::BindGroups[i], im_stack_bind_group);
    }
    return BindGroups;
}