struct dynamic_buffer_block {
    gdi_handle<gdi_buffer>     Buffer;
    u8*                        Ptr;
    uptr                       Size;
    uptr                       Used;
    gdi_handle<gdi_bind_group> BindGroup;
    dynamic_buffer_block*      Next;
};

struct dynamic_buffer {
    arena*                            Arena;
    renderer*                         Renderer;
    dynamic_buffer_block*             First;
    dynamic_buffer_block*             Last;
    dynamic_buffer_block*             Current;
    uptr                              MinimumBlockSize;
    uptr                              Alignment;
};

internal dynamic_buffer_block* Dynamic_Buffer_Get_Current_Block(dynamic_buffer* Buffer, uptr Size) {
    dynamic_buffer_block* Result = Buffer->Current;
    while(Result && (Result->Size < (Align_Pow2(Result->Used, Buffer->Alignment)+Size))) {
        Result = Result->Next;
    }
    return Result;
}

internal dynamic_buffer_block* Dynamic_Buffer_Allocate_Block(dynamic_buffer* Buffer, uptr BlockSize) {
    renderer* Renderer = Buffer->Renderer;
    dynamic_buffer_block* Block = Arena_Push_Struct(Buffer->Arena, dynamic_buffer_block);
    Block->Buffer = GDI_Context_Create_Buffer(Renderer->Context, {
        .ByteSize = BlockSize,
        .UsageFlags = GDI_BUFFER_USAGE_FLAG_CONSTANT_BUFFER_BIT|GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT
    });
    Block->Size = BlockSize;

    Block->BindGroup = GDI_Context_Create_Bind_Group(Renderer->Context, {
        .Layout = Renderer->DynamicBufferLayout,
        .WriteInfo = {
            .Bindings = { {
                    .Type = GDI_BIND_GROUP_TYPE_CONSTANT_DYNAMIC,
                    .BufferBinding = {
                        .Buffer = Block->Buffer,
                        .Size   = BlockSize
                    }
                }
            }
        }
    });

    return Block;
}

dynamic_binding Dynamic_Buffer_Push(dynamic_buffer* Buffer, uptr Size) {
    Assert(Is_Pow2(Buffer->Alignment));

    dynamic_buffer_block* Block = Dynamic_Buffer_Get_Current_Block(Buffer, Size);
    if(!Block) {
        uptr BlockSize = 0;
        if(!Buffer->Last) {
            BlockSize = Buffer->Last->Size*2;
        }

        uptr Mask = Buffer->Alignment-1;
        
        //If the block size still doesn't handle the allocation, round 
        //the block up to the nearest power of two that handles the 
        //allocation
        if(BlockSize < (Size+Mask)) {
            BlockSize = Ceil_Pow2(Size+Mask);
        }

        //If the blocksize is less than the minimum block size we will round up
        BlockSize = Max(BlockSize, Buffer->MinimumBlockSize);

        Block = Dynamic_Buffer_Allocate_Block(Buffer, BlockSize);
        if(!Block) {
            Assert(false);
            return {};
        }
        SLL_Push_Back(Buffer->First, Buffer->Last, Block);
    }
    Buffer->Current = Block;
    dynamic_buffer_block* CurrentBlock = Buffer->Current;

    CurrentBlock->Used = Align_Pow2(CurrentBlock->Used, Buffer->Alignment);
    Assert(CurrentBlock->Used+Size <= CurrentBlock->Size);

    if(!CurrentBlock->Ptr) {
        CurrentBlock->Ptr = GDI_Context_Buffer_Map(Buffer->Renderer->Context, CurrentBlock->Buffer);
        if(!CurrentBlock->Ptr) {
            Assert(false);
            return {};
        }
    }

    dynamic_binding Result = {
        .BindGroup = CurrentBlock->BindGroup,
        .Offset    = Safe_U32(CurrentBlock->Used),
        .Data      = CurrentBlock->Ptr + CurrentBlock->Used
    };

    CurrentBlock->Used += Size;
    return Result;
}

internal dynamic_buffer Dynamic_Buffer_Create(renderer* Renderer, arena* Arena, uptr BlockSize) {
    dynamic_buffer Result = {
        .Arena = Arena,
        .Renderer = Renderer,
        .MinimumBlockSize = BlockSize,
        .Alignment = GDI_Context_Get_Info(Renderer->Context)->ConstantBufferAlignment
    };

    return Result;
}

internal void Dynamic_Buffer_Reset(dynamic_buffer* Buffer) {
    for(dynamic_buffer_block* Block = Buffer->First; Block; Block = Block->Next) {
        Block->Used = 0;
        if(Block->Ptr) {
            GDI_Context_Buffer_Unmap(Buffer->Renderer->Context, Block->Buffer);
            Block->Ptr = nullptr;
        }
    }
    Buffer->Current = Buffer->First;
}