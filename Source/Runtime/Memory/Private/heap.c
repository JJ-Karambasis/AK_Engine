//heap* Heap_Create(allocator* Allocator, size_t InitialBlockSize);
//void  Heap_Delete(heap* Heap);

typedef int32_t heap__comparision_func(heap_block* BlockA, heap_block* BlockB);

typedef struct heap__block_key
{
    heap_memory_block* Block;
    size_t             Offset;
} heap__block_key;

ALLOCATOR_ALLOCATE(Heap__Allocate)
{
    heap* Heap = (heap*)Allocator;
    return Heap_Allocate(Heap, Size, MEMORY_NO_CLEAR);
}

ALLOCATOR_FREE(Heap__Free)
{
    heap* Heap = (heap*)Allocator;
    Heap_Free(Heap, Memory);
}

HASH_FUNCTION(Hash__Heap_Block)
{
    heap__block_key* BlockKey = (heap__block_key*)Key;
    uint32_t Hash = Hash_Ptr(BlockKey->Block);
    Hash_Combine(&Hash, Hash_Ptr((void*)BlockKey->Offset));
    return Hash;
}

KEY_COMPARE(Hash__Heap_Key_Compare)
{
    heap__block_key* BlockKeyA = (heap__block_key*)LeftKey;
    heap__block_key* BlockKeyB = (heap__block_key*)RightKey;
    return Key_Compare_Ptr(BlockKeyA->Block, BlockKeyB->Block) && Key_Compare_Ptr((void*)BlockKeyA->Offset, (void*)BlockKeyB->Offset);
}

heap_block* Heap__Get_Min_Block(heap* Heap, heap_block* Block)
{
    heap_block* Node = Block;
    if(!Node) return NULL;
    while(Node->LeftChild) Node = Node->LeftChild;
    return Node;
}

void Heap__Add_Free_Block_To_Tree(heap* Heap, heap_block* Block)
{
    heap_block* Node = Heap->FreeBlockTree.Root;
    heap_block* Parent = NULL;
    
    while(Node)
    {
        if(Block->Value.Size < Node->Value.Size)
        {
            Parent = Node;
            Node = Node->LeftChild;
        } 
        else
        {
            Parent = Node;
            Node = Node->RightChild;
        }
    }
    
    Heap__Set_Parent(Block, Parent);
    if(Parent == NULL)
    {
        Heap->FreeBlockTree.Root = Block;
        Heap__Set_Black(Block);
    }
    else
    {
        if(Block->Value.Size < Parent->Value.Size) Parent->LeftChild  = Block;
        else                                       Parent->RightChild = Block;
        Heap__Fixup_Tree_Add(Heap, Block);
    }
}

void Heap__Swap_Block_Values(heap* Heap, heap_block* BlockA, heap_block* BlockB)
{
    heap__block_key KeyA = {BlockA->Value.Block, BlockA->Value.Offset};
    heap__block_key KeyB = {BlockB->Value.Block, BlockB->Value.Offset};
    
    *Hashmap_Find(Heap->OffsetMap, heap_block*, &KeyA) = BlockB;
    *Hashmap_Find(Heap->OffsetMap, heap_block*, &KeyB) = BlockA;
    
    heap_block_value Tmp = BlockA->Value;
    BlockB->Value = BlockA->Value;
    BlockA->Value = Tmp;
}

heap_block* Heap__Remove_Free_Block_From_Tree(heap* Heap, heap_block* Block)
{
    heap_block* Out;
    if(!Block->LeftChild || !Block->RightChild)
    {
        Out = Block;
    }
    else
    {
        Out = Heap__Get_Min_Block(Heap, Block->RightChild);
        Heap__Swap_Block_Values(Heap, Block, Out);
    }
    
    heap_block* ChildLink = Out->LeftChild ? Out->LeftChild : Out->RightChild;
    heap_block* ParentLink = Heap__Get_Parent(Out);
    
    if(ChildLink) Heap__Set_Parent(ChildLink, ParentLink);
    
    bool32_t ChooseLeft = ParentLink && ParentLink->LeftChild == Out;
    
    *(ParentLink ? (ParentLink->LeftChild == Out ? &ParentLink->LeftChild : &ParentLink->RightChild) : &Heap->FreeBlockTree.Root) = ChildLink;
    
    if(Heap__Get_Color(Out) == HEAP__BLACK_NODE && Heap->FreeBlockTree.Root)
        Heap__Fixup_Tree_Remove(Heap, ChildLink, ParentLink, ChooseLeft);
    
    return Out;
}

heap_block* Heap__Remove_Free_Block(heap* Heap, heap_block* Block)
{
    heap_block* Out = Heap__Remove_Free_Block_From_Tree(Heap, Block);
    heap__block_key Key;
    Key.Block  = Out->Value.Block;
    Key.Offset = Out->Value.Offset;
    Hashmap_Remove(Heap->OffsetMap, &Key);
    return Out;
}

void Heap__Add_Free_Block(heap* Heap, heap_block* Block)
{
    Heap__Add_Free_Block_To_Tree(Heap, Block);
    heap__block_key Key;
    Key.Block  = Block->Value.Block;
    Key.Offset = Block->Value.Offset;
    Hashmap_Add(Heap->OffsetMap, &Key, Block);
}

heap_block* Heap__Create_Heap_Block(heap* Heap)
{
    heap_block* Result = Heap->FreeBlockTree.OrphanBlocks;
    if(Result) Heap->FreeBlockTree.OrphanBlocks = Heap->FreeBlockTree.OrphanBlocks->LeftChild;
    else Result = Arena_Push_Struct(Heap->Arena, heap_block);
    Zero_Struct(Result, heap_block);
    return Result;
}

void Heap__Delete_Heap_Block(heap* Heap, heap_block* Block)
{
    heap_block* Out = Block;
    if(!Block->Color) Out = Heap__Remove_Free_Block(Heap, Block);
    Out->LeftChild = Heap->FreeBlockTree.OrphanBlocks;
    Heap->FreeBlockTree.OrphanBlocks = Out;
}

heap_block* Heap__Find_Best_Fitting_Block(heap* Heap, size_t Size)
{
    return NULL;
}

heap_block* Heap__Find_Best_Offset(heap* Heap, heap_memory_block* MemoryBlock, size_t Offset)
{
    heap__block_key Key;
    Key.Block  = MemoryBlock;
    Key.Offset = Offset;
    heap_block** Block = Hashmap_Find(Heap->OffsetMap, heap_block*, &Key);
    if(!Block) return NULL;
    return *Block;
}

void Heap__Increase_Block_Size(heap* Heap, heap_block* Block, size_t Addend)
{
    heap_block* Out = Heap__Remove_Free_Block_From_Tree(Heap, Block);
    Out->Value.Size += Addend;
    Heap__Add_Free_Block_To_Tree(Heap, Out);
}

heap_block* Heap__Split_Block(heap* Heap, heap_block* Block, size_t Size)
{
    int64_t BlockDiff = (int64_t)Block->Value.Size - (int64_t)(Size+sizeof(heap_block*));
    if(BlockDiff > 0)
    {
        size_t Offset = Block->Value.Offset + sizeof(heap_block*) + Size;
        
        heap_block* NewBlock = Heap__Create_Heap_Block(Heap);
        heap_block** NewBlockPtr = (heap_block**)(Block->Value.Block->Memory + Offset);
        *NewBlockPtr = NewBlock;
        
        NewBlock->Value.Block  = Block->Value.Block;
        NewBlock->Value.Size   = (size_t)BlockDiff;
        NewBlock->Value.Offset = Offset;
        Heap__Add_Free_Block(Heap, NewBlock);
    }
    
    Block->Value.Size = Size;
    return Heap__Remove_Free_Block(Heap, Block);
}

heap_block* Heap__Add_Free_Block_From_Memory_Block(heap* Heap, heap_memory_block* MemoryBlock)
{
    heap_block* Block = Heap__Create_Heap_Block(Heap);
    
    heap_block** BlockPtr = (heap_block**)(MemoryBlock->Memory);
    *BlockPtr = Block;
    
    Block->Value.Block  = MemoryBlock;
    Block->Value.Size   = MemoryBlock->Size-sizeof(heap_block*);
    Heap__Add_Free_Block(Heap, Block);
    return Block;
}

heap_block* Heap__Create_Memory_Block(heap* Heap, size_t BlockSize)
{
    heap_memory_block* MemoryBlock = (heap_memory_block*)Arena_Push(Heap->Arena, BlockSize+sizeof(heap_memory_block)+sizeof(heap_block*), MEMORY_NO_CLEAR);
    Zero_Struct(MemoryBlock, heap_memory_block);
    
    MemoryBlock->Memory = (uint8_t*)(MemoryBlock+1);
    MemoryBlock->Size   = BlockSize+sizeof(heap_block);
    
    MemoryBlock->Next = Heap->FirstBlock;
    Heap->FirstBlock  = MemoryBlock;
    
    return Heap__Add_Free_Block_From_Memory_Block(Heap, MemoryBlock);
}

heap* Heap_Create(allocator* Allocator, size_t InitialBlockSize)
{
    arena* Arena = Arena_Create(Allocator, InitialBlockSize+InitialBlockSize/4);
    heap* Result = Arena_Push_Struct(Arena, heap);
    
    Result->BaseAllocator.Allocate = Heap__Allocate;
    Result->BaseAllocator.Free     = Heap__Free;
    
    Result->Arena            = Arena;
    Result->InitialBlockSize = InitialBlockSize;
    
    Result->OffsetMap  = Arena_Push_Struct(Arena, hashmap);
    *Result->OffsetMap = Hashmap_Create(Allocator, heap__block_key, heap_block*, Hash__Heap_Block, Hash__Heap_Key_Compare, 512, 64);
    
    return Result;
}

void Heap_Delete(heap* Heap)
{
    Hashmap_Delete(Heap->OffsetMap);
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
    
    Block = Heap__Split_Block(Heap, Block, Size);
    
    Assert(Block->Value.Size == Size);
    Assert(Block->Value.Offset + Size + sizeof(heap_block) <= Block->Value.Block->Size);
    void* Result = Block->Value.Block->Memory + Block->Value.Offset + sizeof(heap_block*);
    
    if(ClearFlag == MEMORY_CLEAR) Memory_Clear(Result, Size);
    return Result;
}

void Heap_Free(heap* Heap, void* Memory)
{
    if(Memory)
    {
        heap_block** BlockPtr = (heap_block**)((heap_block**)Memory - 1);
        heap_block* Block = *BlockPtr;
        
        heap_block* LeftBlock  = Heap__Find_Best_Offset(Heap, Block->Value.Block, Block->Value.Offset);
        heap_block* RightBlock = Heap__Find_Best_Offset(Heap, Block->Value.Block, Block->Value.Offset+sizeof(heap_block*)+Block->Value.Size);
        
        if(!LeftBlock && !RightBlock)
        {
            Heap__Add_Free_Block(Heap, Block);
        }
        else if(LeftBlock && !RightBlock)
        {
            Heap__Increase_Block_Size(Heap, LeftBlock, Block->Value.Size);
            Heap__Delete_Heap_Block(Heap, Block);
        }
        else if(!LeftBlock && RightBlock)
        {
            Block->Value.Size += RightBlock->Value.Size;
            Heap__Delete_Heap_Block(Heap, RightBlock);
            Heap__Add_Free_Block(Heap, Block);
        }
        else
        {
            Heap__Increase_Block_Size(Heap, LeftBlock, Block->Value.Size+RightBlock->Value.Size);
            Heap__Delete_Heap_Block(Heap, Block);
            Heap__Delete_Heap_Block(Heap, RightBlock);
        }
    }
}

void Heap_Clear(heap* Heap, memory_clear_flag ClearFlag)
{
    Hashmap_Clear(Heap->OffsetMap);
    Arena_Clear(Heap->Arena, ClearFlag);
    Zero_Struct(&Heap->FreeBlockTree, heap_block_tree);
    
    for(heap_memory_block* MemoryBlock = Heap->FirstBlock; MemoryBlock; MemoryBlock = MemoryBlock->Next)
    {
        Heap__Add_Free_Block_From_Memory_Block(Heap, MemoryBlock);
    }
}

//void  Heap_Clear(heap* Heap, memory_clear_flag ClearFlag);