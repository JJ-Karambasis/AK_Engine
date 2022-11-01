//heap* Heap_Create(allocator* Allocator, size_t InitialBlockSize);
//void  Heap_Delete(heap* Heap);

ALLOCATOR_ALLOCATE(Heap__Allocate)
{
    heap* Heap = (heap*)Allocator;
    return Heap_Allocate(Heap, Size, MEMORY_NO_CLEAR);
}

ALLOCATOR_FREE(Heap__Free)
{
    heap* Heap = (heap*)Allocator;
    return Heap_Free(Heap, Memory);
}

heap_block* Heap__Find_Best_Fitting_Block(heap* Heap, size_t Size)
{
    return NULL;
}

heap_block* Heap__Find_Best_Offset(heap* Heap, heap_memory_block* MemoryBlock, size_t Offset)
{
    return NULL;
}

void Heap__Add_To_Free_Block_Tree(heap* Heap, heap_block* Block)
{
}

void Heap__Remove_From_Free_Block_Tree(heap* Heap, heap_block* Block)
{
}

void Heap__Increase_Block_Size(heap* Heap, heap_block* Block, size_t Addend)
{
}

void Heap__Split_Block(heap* Heap, heap_block* Block, size_t Size)
{
    int64_t BlockDiff = (int64_t)Block->Size - (int64_t)(Size+sizeof(heap_block));
    if(BlockDiff > 0)
    {
        size_t Offset = Block->Offset + sizeof(heap_block) + Size;
        heap_block* NewBlock = (heap_block*)(Block->Block->Memory + Offset);
        Zero_Struct(NewBlock, heap_block);
        
        NewBlock->Block  = Block->Block;
        NewBlock->Size   = (size_t)BlockDiff;
        NewBlock->Offset = Offset;
        Heap__Add_To_Free_Block_Tree(Heap, NewBlock);
    }
    
    Block->Size = Size;
    Heap__Remove_From_Free_Block_Tree(Heap, Block);
}

void Heap__Orphan_Block(heap* Heap, heap_block* Block)
{
}

heap_block* Heap__Add_Block_From_Memory_Block(heap* Heap, heap_memory_block* MemoryBlock)
{
    heap_block* Block = (heap_block*)(MemoryBlock->Memory);
    Zero_Struct(Block, heap_block);
    Block->Block  = MemoryBlock;
    Block->Size   = MemoryBlock->Size-sizeof(heap_block);
    Heap__Add_To_Free_Block_Tree(Heap, Block);
    return Block;
}

heap_block* Heap__Create_Memory_Block(heap* Heap, size_t BlockSize)
{
    heap_memory_block* MemoryBlock = (heap_memory_block*)Arena_Push(Heap->Arena, BlockSize+sizeof(heap_memory_block)+sizeof(heap_block), MEMORY_NO_CLEAR);
    Zero_Struct(MemoryBlock, heap_memory_block);
    
    MemoryBlock->Memory = (uint8_t*)(MemoryBlock+1);
    MemoryBlock->Size   = BlockSize+sizeof(heap_block);
    
    MemoryBlock->Next = Heap->FirstBlock;
    Heap->FirstBlock  = MemoryBlock;
    
    return Heap__Add_Block_From_Memory_Block(Heap, MemoryBlock);
}

heap* Heap_Create(allocator* Allocator, size_t InitialBlockSize)
{
    arena* Arena = Arena_Create(Allocator, InitialBlockSize+InitialBlockSize/4);
    heap* Result = Arena_Push_Struct(Arena, heap);
    
    Result->BaseAllocator.Allocate = Heap__Allocate;
    Result->BaseAllocator.Free     = Heap__Free;
    
    Result->Arena            = Arena;
    Result->InitialBlockSize = InitialBlockSize;
    
    return Result;
}

void Heap_Delete(heap* Heap)
{
    Arena_Delete(Heap->Arena);
}

void* Heap_Allocate(heap* Heap, size_t Size, memory_clear_flag ClearFlag)
{
    if(!Size) return NULL;
    heap_block* Block = Heap__Find_Best_Fitting_Block(Heap, Size);
    if(!Block)
    {
        size_t BlockSize = Max(Size, Heap->InitialBlockSize);
        Block = Heap__Create_Memory_Block(Heap, BlockSize);
        if(!Block) return NULL;
    }
    
    Heap__Split_Block(Heap, Block, Size);
    
    Assert(Block->Size == Size);
    Assert(Block->Offset + Size + sizeof(heap_block) <= Block->Block->Size);
    void* Result = Block->Block->Memory + Block->Offset + sizeof(heap_block);
    
    if(ClearFlag == MEMORY_CLEAR) Memory_Clear(Result, Size);
    return Result;
}

void Heap_Free(heap* Heap, void* Memory)
{
    if(Memory)
    {
        heap_block* Block = ((heap_block*)Memory - 1);
        
        heap_block* LeftBlock  = Heap__Find_Best_Offset(Heap, Block->Block, Block->Offset);
        heap_block* RightBlock = Heap__Find_Best_Offset(Heap, Block->Block, Block->Offset+sizeof(heap_block)+Block->Size);
        
        if(!LeftBlock && !RightBlock)
        {
            Heap__Add_To_Free_Block_Tree(Heap, Block);
        }
        else if(LeftBlock && !RightBlock)
        {
            Heap__Increase_Block_Size(Heap, LeftBlock, Block->Size);
            Heap__Orphan_Block(Heap, Block);
        }
        else if(!LeftBlock && RightBlock)
        {
            Block->Size += RightBlock->Size;
            Heap__Orphan_Block(Heap, RightBlock);
            Heap__Add_To_Free_Block_Tree(Heap, Block);
        }
        else
        {
            Heap__Increase_Block_Size(Heap, LeftBlock, LeftBlock->Size+Block->Size+RightBlock->Size);
            Heap__Orphan_Block(Heap, Block);
            Heap__Orphan_Block(Heap, RightBlock);
        }
    }
}

void Heap_Clear(heap* Heap, memory_clear_flag ClearFlag)
{
    Arena_Clear(Heap->Arena, ClearFlag);
    Heap->FreeBlockTree.RootNodeSize   = NULL;
    Heap->FreeBlockTree.RootNodeOffset = NULL;
    Heap->OrphanNodes                  = NULL;
    
    for(heap_memory_block* MemoryBlock = Heap->FirstBlock; MemoryBlock; MemoryBlock = MemoryBlock->Next)
    {
        Heap__Add_Block_From_Memory_Block(Heap, MemoryBlock);
    }
}

//void  Heap_Clear(heap* Heap, memory_clear_flag ClearFlag);