ALLOCATOR_ALLOCATE(Arena__Allocate)
{
    arena* Arena = (arena*)Allocator;
    return Arena_Push(Arena, Size, MEMORY_NO_CLEAR);
}

ALLOCATOR_FREE(Arena__Free)
{
    //NOTE(EVERYONE): Noop
}

arena_block* Arena__Create_Block(arena* Arena, size_t BlockSize)
{
    arena_block* Block = (arena_block*)Arena->Allocator->Allocate(Arena->Allocator, BlockSize+sizeof(arena_block));
    Zero_Struct(Block, arena_block);
    
    Block->Memory = (uint8_t*)(Block+1);
    Block->Size = BlockSize;
    return Block;
}

arena_block* Arena__Get_Block(arena* Arena, size_t Size)
{
    arena_block* Block = Arena->CurrentBlock;
    if(!Block) return NULL;
    
    while(Block->Used+Size > Block->Size)
    {
        Block = Block->Next;
        if(!Block) return NULL;
    }
    
    return Block;
}

arena* Arena_Create(allocator* Allocator, size_t InitialBlockSize)
{
    arena* Result = Allocator->Allocate(Allocator, sizeof(arena));
    Zero_Struct(Result, arena);
    
    Result->BaseAllocator.Allocate = Arena__Allocate;
    Result->BaseAllocator.Free     = Arena__Free;
    
    Result->Allocator        = Allocator;
    Result->InitialBlockSize = InitialBlockSize;
    
    return Result;
}

void Arena__Add_Block(arena* Arena, arena_block* Block)
{
    arena_block* Last = Arena->LastBlock;
    if(!Last)
    {
        Assert(!Arena->FirstBlock);
        Arena->FirstBlock = Arena->LastBlock = Block;
    }
    else
    {
        Last->Next = Block;
        Arena->LastBlock = Block;
    }
}

void* Arena_Push(arena* Arena, size_t Size, memory_clear_flag ClearFlag)
{
    if(!Size) return NULL;
    
    arena_block* Block = Arena__Get_Block(Arena, Size);
    if(!Block)
    {
        size_t BlockSize = Max(Arena->InitialBlockSize, Size);
        Block = Arena__Create_Block(Arena, BlockSize);
        if(!Block) return NULL;
        Arena__Add_Block(Arena, Block);
    }
    
    Arena->CurrentBlock = Block;
    Assert(Arena->CurrentBlock->Used+Size <= Arena->CurrentBlock->Size);
    
    void* Result = Arena->CurrentBlock->Memory + Arena->CurrentBlock->Used;
    Arena->CurrentBlock->Used += Size;
    
    if(ClearFlag == MEMORY_CLEAR) Memory_Clear(Result, Size);
    
    return Result;
}

arena_marker Arena_Get_Marker(arena* Arena)
{
    arena_marker Marker;
    Marker.Arena = Arena;
    Marker.Block = Arena->CurrentBlock;
    Marker.Marker = Marker.Block ? Marker.Block->Used : 0;
    return Marker;
}

void Arena_Set_Marker(arena* Arena, arena_marker* Marker)
{
    Assert(Arena == Marker->Arena);
    if(Marker->Block)
    {
        Arena->CurrentBlock = Marker->Block;
        Arena->CurrentBlock->Used = Marker->Marker;
        for(arena_block* ArenaBlock = Arena->CurrentBlock->Next; ArenaBlock; ArenaBlock = ArenaBlock->Next)
            ArenaBlock->Used = 0;
    }
    else
    {
        Arena_Clear(Arena, MEMORY_NO_CLEAR);
    }
}

void Arena_Clear(arena* Arena, memory_clear_flag ClearFlag)
{
    for(arena_block* ArenaBlock = Arena->FirstBlock; ArenaBlock; ArenaBlock = ArenaBlock->Next)
    {
        if(ClearFlag == MEMORY_CLEAR) Memory_Clear(ArenaBlock->Memory, ArenaBlock->Size);
        ArenaBlock->Used = 0;
    }
    Arena->CurrentBlock = Arena->FirstBlock;
}

void Arena_Delete(arena* Arena)
{
    allocator* Allocator = Arena->Allocator;
    for(arena_block* Block = Arena->FirstBlock; Block; ) 
    {
        arena_block* CurrentBlock = Block;
        Block = CurrentBlock->Next;
        Allocator->Free(Allocator, CurrentBlock);
    }
    Allocator->Free(Allocator, Arena);
}