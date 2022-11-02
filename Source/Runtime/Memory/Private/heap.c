#define HEAP__RED_NODE 0
#define HEAP__BLACK_NODE 1

#define Heap__Get_Parent(Node) ((heap_block*)((Node)->Color & ~HEAP__BLACK_NODE))
#define Heap__Get_Color(Node) ((Node)->Color & HEAP__BLACK_NODE)

#define Heap__Set_Red(Node)            ((Node)->Color &= (~(size_t)HEAP__BLACK_NODE))
#define Heap__Set_Black(Node)          ((Node)->Color |= ((size_t)HEAP__BLACK_NODE))
#define Heap__Set_Parent(Node, Parent) ((Node)->Color = Heap__Get_Color(Node) | (size_t)(Parent))

typedef int32_t heap__comparision_func(heap_block* BlockA, heap_block* BlockB);

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

inline bool8_t Heap__Is_Block_Free(heap_block* Block)
{
    return Block && !Block->Color;
}

heap_block* Heap__Get_Min_Block(heap* Heap, heap_block* Block)
{
    heap_block* Node = Block;
    if(!Node) return NULL;
    while(Node->LeftChild) Node = Node->LeftChild;
    return Node;
}

void Heap__Swap_Block_Values(heap* Heap, heap_block* BlockA, heap_block* BlockB)
{
    heap_block_value Tmp = BlockA->Value;
    BlockB->Value = BlockA->Value;
    BlockA->Value = Tmp;
}

void Heap__Rot_Tree_Left(heap* Heap, heap_block* Block)
{
    heap_block* RightChild = Block->RightChild;
    if((Block->RightChild = RightChild->LeftChild) != NULL)
        Heap__Set_Parent(RightChild->LeftChild, Block);
    
    heap_block* Parent = Heap__Get_Parent(Block);
    Heap__Set_Parent(RightChild, Parent);
    *(Parent ? (Parent->LeftChild == Block ? &Parent->LeftChild : &Parent->RightChild) : &Heap->FreeBlockTree.Root) = RightChild;
    RightChild->LeftChild = Block;
    Heap__Set_Parent(Block, RightChild);
}

void Heap__Rot_Tree_Right(heap* Heap, heap_block* Block)
{
    heap_block* LeftChild = Block->LeftChild;
    if((Block->LeftChild = LeftChild->RightChild) != NULL)
        Heap__Set_Parent(LeftChild->RightChild, Block);
    
    heap_block* Parent = Heap__Get_Parent(Block);
    Heap__Set_Parent(LeftChild, Parent);
    *(Parent ? (Parent->LeftChild == Block ? &Parent->LeftChild : &Parent->RightChild) : &Heap->FreeBlockTree.Root) = LeftChild;
    LeftChild->RightChild = Block;
    Heap__Set_Parent(Block, LeftChild);
}

void Heap__Fixup_Tree_Add(heap* Heap, heap_block* Block)
{
    while(Block != Heap->FreeBlockTree.Root && Heap__Get_Color(Heap__Get_Parent(Block)) == HEAP__RED_NODE)
    {
        if(Heap__Get_Parent(Block) == Heap__Get_Parent(Heap__Get_Parent(Block))->LeftChild)
        {
            heap_block* Temp = Heap__Get_Parent(Heap__Get_Parent(Block))->RightChild;
            if(Temp && (Heap__Get_Color(Temp) == HEAP__RED_NODE))
            {
                Heap__Set_Black(Temp);
                Block = Heap__Get_Parent(Block);
                Heap__Set_Black(Block);
                Block = Heap__Get_Parent(Block);
                Heap__Set_Red(Block);
            }
            else
            {
                if(Block == Heap__Get_Parent(Block)->RightChild)
                {
                    Block = Heap__Get_Parent(Block);
                    Heap__Rot_Tree_Left(Heap, Block);
                }
                
                Temp = Heap__Get_Parent(Block);
                Heap__Set_Black(Temp);
                Temp = Heap__Get_Parent(Temp);
                Heap__Set_Red(Temp);
                Heap__Rot_Tree_Right(Heap, Temp);
            }
        }
        else 
        {
            heap_block* Temp = Heap__Get_Parent(Heap__Get_Parent(Block))->LeftChild;
            if(Temp && (Heap__Get_Color(Temp) == HEAP__RED_NODE))
            {
                Heap__Set_Black(Temp);
                Block = Heap__Get_Parent(Block);
                Heap__Set_Black(Block);
                Block = Heap__Get_Parent(Block);
                Heap__Set_Red(Block);
            } 
            else 
            {
                if(Block == Heap__Get_Parent(Block)->LeftChild)
                {
                    Block = Heap__Get_Parent(Block);
                    Heap__Rot_Tree_Right(Heap, Block);
                }
                
                Temp = Heap__Get_Parent(Block);
                Heap__Set_Black(Temp);
                Temp = Heap__Get_Parent(Block);
                Heap__Set_Red(Temp);
                Heap__Rot_Tree_Left(Heap, Temp);
            }
        }
    }
    
    Heap__Set_Black(Heap->FreeBlockTree.Root);
}

void Heap__Add_Free_Block(heap* Heap, heap_block* Block)
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

void Heap__Fixup_Tree_Remove(heap* Heap, heap_block* Block, heap_block* Parent, bool32_t ChooseLeft)
{
    while(Block != Heap->FreeBlockTree.Root && (Block == NULL || Heap__Get_Color(Block) == HEAP__BLACK_NODE))
    {
        if(ChooseLeft)
        {
            heap_block* Temp = Parent->RightChild;
            if(Heap__Get_Color(Temp) == HEAP__RED_NODE)
            {
                Heap__Set_Black(Temp);
                Heap__Set_Red(Parent);
                Heap__Rot_Tree_Left(Heap, Parent);
                Temp = Parent->RightChild;
            }
            
            if((Temp->LeftChild == NULL  || Heap__Get_Color(Temp->LeftChild)  == HEAP__BLACK_NODE) &&
               (Temp->RightChild == NULL || Heap__Get_Color(Temp->RightChild) == HEAP__BLACK_NODE))
            {
                Heap__Set_Red(Temp);
                Block = Parent;
                Parent = Heap__Get_Parent(Parent);
                ChooseLeft = Parent && (Parent->LeftChild == Block);
            }
            else
            {
                if(Temp->RightChild == NULL || Heap__Get_Color(Temp->RightChild) == HEAP__BLACK_NODE)
                {
                    Heap__Set_Black(Temp->LeftChild);
                    Heap__Set_Red(Temp);
                    Heap__Rot_Tree_Right(Heap, Temp);
                    Temp = Parent->RightChild;
                }
                
                (Heap__Get_Color(Parent) == HEAP__RED_NODE) ? Heap__Set_Red(Temp) : Heap__Set_Black(Temp);
                
                if(Temp->RightChild) Heap__Set_Black(Temp->RightChild);
                Heap__Set_Black(Parent);
                Heap__Rot_Tree_Left(Heap, Parent);
                break;
            }
        }
        else
        {
            heap_block* Temp = Parent->LeftChild;
            if(Heap__Get_Color(Temp) == HEAP__RED_NODE)
            {
                Heap__Set_Black(Temp);
                Heap__Set_Red(Parent);
                Heap__Rot_Tree_Right(Heap, Parent);
                Temp = Parent->LeftChild;
            }
            
            if((Temp->LeftChild == NULL || Heap__Get_Color(Temp->LeftChild) == HEAP__BLACK_NODE) &&
               (Temp->RightChild == NULL || Heap__Get_Color(Temp->RightChild) == HEAP__BLACK_NODE))
            {
                Heap__Set_Red(Temp);
                Block = Parent;
                Parent = Heap__Get_Parent(Parent);
                ChooseLeft = Parent && (Parent->LeftChild == Block);
            }
            else
            {
                if(Temp->LeftChild == NULL || Heap__Get_Color(Temp->LeftChild) == HEAP__BLACK_NODE)
                {
                    Heap__Set_Black(Temp->RightChild);
                    Heap__Set_Red(Temp);
                    Heap__Rot_Tree_Left(Heap, Temp);
                    Temp = Parent->LeftChild;
                }
                
                (Heap__Get_Color(Parent) == HEAP__RED_NODE) ? Heap__Set_Red(Temp) : Heap__Set_Black(Temp);
                
                if(Temp->LeftChild) Heap__Set_Black(Temp->LeftChild);
                Heap__Set_Black(Parent);
                Heap__Rot_Tree_Right(Heap, Parent);
                break;
            }
        }
    }
    
    if(Block) Heap__Set_Black(Block);
}

heap_block* Heap__Remove_Free_Block(heap* Heap, heap_block* Block)
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
    if(Heap__Is_Block_Free(Block)) Out = Heap__Remove_Free_Block(Heap, Block);
    Out->LeftChild = Heap->FreeBlockTree.OrphanBlocks;
    Heap->FreeBlockTree.OrphanBlocks = Out;
}

heap_block* Heap__Find_Best_Fitting_Block(heap* Heap, size_t Size)
{
    heap_block* Result = Heap->FreeBlockTree.Root;
    while(Result)
    {
        int64_t Diff = (int64_t)Size - (int64_t)Result->Value.Size;
        if(!Diff) return Result;
        Result = (Diff < 0) ? Result->LeftChild : Result->RightChild;
    }
    return Result;
}

void Heap__Increase_Block_Size(heap* Heap, heap_block* Block, size_t Addend)
{
    heap_block* Out = Heap__Remove_Free_Block(Heap, Block);
    Out->Value.Size += Addend;
    Heap__Add_Free_Block(Heap, Out);
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
        NewBlock->Value.Prev   = Block;
        Block->Value.Next      = NewBlock;
        
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
        
        heap_block* LeftBlock  = Block->Value.Prev;
        heap_block* RightBlock = Block->Value.Next;
        
        bool32_t IsLeftBlockFree  = Heap__Is_Block_Free(Block->Value.Prev);
        bool32_t IsRightBlockFree = Heap__Is_Block_Free(Block->Value.Next);
        
        if(!IsLeftBlockFree && !IsRightBlockFree)
        {
            Heap__Add_Free_Block(Heap, Block);
        }
        else if(LeftBlock && !RightBlock)
        {
            Assert(LeftBlock->Value.Block == Block->Value.Block);
            Assert((LeftBlock->Value.Offset+LeftBlock->Value.Size+sizeof(heap_block*)) == Block->Value.Offset);
            
            LeftBlock->Value.Next = Block->Value.Next;
            if(Block->Value.Next) Block->Value.Next->Value.Prev = LeftBlock;
            
            Heap__Increase_Block_Size(Heap, LeftBlock, Block->Value.Size);
            Heap__Delete_Heap_Block(Heap, Block);
        }
        else if(!LeftBlock && RightBlock)
        {
            Assert(RightBlock->Value.Block == Block->Value.Block);
            Assert((Block->Value.Offset+Block->Value.Size+sizeof(heap_block*)) == RightBlock->Value.Offset);
            
            Block->Value.Next = RightBlock->Value.Next;
            if(RightBlock->Value.Next) RightBlock->Value.Next->Value.Prev = Block;
            
            Block->Value.Size += RightBlock->Value.Size;
            Heap__Delete_Heap_Block(Heap, RightBlock);
            Heap__Add_Free_Block(Heap, Block);
        }
        else
        {
            Assert(LeftBlock->Value.Block  == Block->Value.Block);
            Assert(RightBlock->Value.Block == Block->Value.Block);
            Assert((LeftBlock->Value.Offset+LeftBlock->Value.Size+sizeof(heap_block*)) == Block->Value.Offset);
            Assert((Block->Value.Offset+Block->Value.Size+sizeof(heap_block*)) == RightBlock->Value.Offset);
            
            LeftBlock->Value.Next = RightBlock->Value.Next;
            if(RightBlock->Value.Next) RightBlock->Value.Next->Value.Prev = LeftBlock;
            Heap__Increase_Block_Size(Heap, LeftBlock, Block->Value.Size+RightBlock->Value.Size);
            Heap__Delete_Heap_Block(Heap, Block);
            Heap__Delete_Heap_Block(Heap, RightBlock);
        }
    }
}

void Heap_Clear(heap* Heap, memory_clear_flag ClearFlag)
{
    Arena_Clear(Heap->Arena, ClearFlag);
    Zero_Struct(&Heap->FreeBlockTree, heap_block_tree);
    
    for(heap_memory_block* MemoryBlock = Heap->FirstBlock; MemoryBlock; MemoryBlock = MemoryBlock->Next)
    {
        Heap__Add_Free_Block_From_Memory_Block(Heap, MemoryBlock);
    }
}