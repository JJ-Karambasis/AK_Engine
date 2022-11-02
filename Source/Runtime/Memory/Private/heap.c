#define HEAP__RED_NODE 0
#define HEAP__BLACK_NODE 1

#define Heap__Get_Parent(Node) ((heap_block_node*)((Node)->Color & ~HEAP__BLACK_NODE))
#define Heap__Get_Color(Node) ((Node)->Color & HEAP__BLACK_NODE)

#define Heap__Set_Red(Node)            ((Node)->Color &= (~(size_t)HEAP__BLACK_NODE))
#define Heap__Set_Black(Node)          ((Node)->Color |= ((size_t)HEAP__BLACK_NODE))
#define Heap__Set_Parent(Node, Parent) ((Node)->Color = Heap__Get_Color(Node) | (size_t)(Parent))

#ifdef DEBUG_BUILD
bool8_t Heap__Has_Allocated_Block(heap* Heap, heap_block* Block);

#define Heap__Check_If_Block_Is_Allocated(Heap, block) \
Assert(Heap__Has_Allocated_Block(Heap, Block)); \
if(Block->Value.PreviousAllocated) Block->Value.PreviousAllocated->Value.NextAllocated = Block->Value.NextAllocated; \
if(Block->Value.NextAllocated) Block->Value.NextAllocated->Value.PreviousAllocated = Block->Value.PreviousAllocated; \
Block->Value.PreviousAllocated = Block->Value.NextAllocated = NULL

#define Heap__Verify(Condition) \
do \
{ \
if(!(Condition)) \
return false; \
} while(0)
#else
#define Heap__Check_If_Block_Is_Allocated(Heap, Block)
#define Heap__Verify(Condition)
#endif

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
    return Block && Block->Node;
}

heap_block_node* Heap__Get_Min_Block(heap_block_node* Node)
{
    if(!Node) return NULL;
    while(Node->LeftChild) Node = Node->LeftChild;
    return Node;
}

void Heap__Swap_Block_Values(heap_block_node* NodeA, heap_block_node* NodeB)
{
    heap_block* Tmp = NodeA->Block;
    NodeB->Block    = NodeA->Block;
    NodeA->Block    = Tmp;
    
    NodeA->Block->Node = NodeA;
    NodeB->Block->Node = NodeB;
}

void Heap__Rot_Tree_Left(heap_block_tree* Tree, heap_block_node* Node)
{
    heap_block_node* RightChild = Node->RightChild;
    if((Node->RightChild = RightChild->LeftChild) != NULL)
        Heap__Set_Parent(RightChild->LeftChild, Node);
    
    heap_block_node* Parent = Heap__Get_Parent(Node);
    Heap__Set_Parent(RightChild, Parent);
    *(Parent ? (Parent->LeftChild == Node ? &Parent->LeftChild : &Parent->RightChild) : &Tree->Root) = RightChild;
    RightChild->LeftChild = Node;
    Heap__Set_Parent(Node, RightChild);
}

void Heap__Rot_Tree_Right(heap_block_tree* Tree, heap_block_node* Node)
{
    heap_block_node* LeftChild = Node->LeftChild;
    if((Node->LeftChild = LeftChild->RightChild) != NULL)
        Heap__Set_Parent(LeftChild->RightChild, Node);
    
    heap_block_node* Parent = Heap__Get_Parent(Node);
    Heap__Set_Parent(LeftChild, Parent);
    *(Parent ? (Parent->LeftChild == Node ? &Parent->LeftChild : &Parent->RightChild) : &Tree->Root) = LeftChild;
    LeftChild->RightChild = Node;
    Heap__Set_Parent(Node, LeftChild);
}

heap_block_node* Heap__Create_Tree_Node(heap* Heap)
{
    heap_block_tree* Tree = &Heap->FreeBlockTree;
    heap_block_node* Result = Tree->FreeNodes;
    if(Result) Tree->FreeNodes = Tree->FreeNodes->LeftChild;
    else Result = Arena_Push_Struct(Heap->Arena, heap_block_node);
    Zero_Struct(Result, heap_block_node);
    return Result;
}

void Heap__Delete_Tree_Node(heap* Heap, heap_block_node* Node)
{
    Node->LeftChild = Heap->FreeBlockTree.FreeNodes;
    Heap->FreeBlockTree.FreeNodes = Node;
}

void Heap__Fixup_Tree_Add(heap_block_tree* Tree, heap_block_node* Node)
{
    while(Node != Tree->Root && Heap__Get_Color(Heap__Get_Parent(Node)) == HEAP__RED_NODE)
    {
        if(Heap__Get_Parent(Node) == Heap__Get_Parent(Heap__Get_Parent(Node))->LeftChild)
        {
            heap_block_node* Temp = Heap__Get_Parent(Heap__Get_Parent(Node))->RightChild;
            if(Temp && (Heap__Get_Color(Temp) == HEAP__RED_NODE))
            {
                Heap__Set_Black(Temp);
                Node = Heap__Get_Parent(Node);
                Heap__Set_Black(Node);
                Node = Heap__Get_Parent(Node);
                Heap__Set_Red(Node);
            }
            else
            {
                if(Node == Heap__Get_Parent(Node)->RightChild)
                {
                    Node = Heap__Get_Parent(Node);
                    Heap__Rot_Tree_Left(Tree, Node);
                }
                
                Temp = Heap__Get_Parent(Node);
                Heap__Set_Black(Temp);
                Temp = Heap__Get_Parent(Temp);
                Heap__Set_Red(Temp);
                Heap__Rot_Tree_Right(Tree, Temp);
            }
        }
        else 
        {
            heap_block_node* Temp = Heap__Get_Parent(Heap__Get_Parent(Node))->LeftChild;
            if(Temp && (Heap__Get_Color(Temp) == HEAP__RED_NODE))
            {
                Heap__Set_Black(Temp);
                Node = Heap__Get_Parent(Node);
                Heap__Set_Black(Node);
                Node = Heap__Get_Parent(Node);
                Heap__Set_Red(Node);
            } 
            else 
            {
                if(Node == Heap__Get_Parent(Node)->LeftChild)
                {
                    Node = Heap__Get_Parent(Node);
                    Heap__Rot_Tree_Right(Tree, Node);
                }
                
                Temp = Heap__Get_Parent(Node);
                Heap__Set_Black(Temp);
                Temp = Heap__Get_Parent(Node);
                Heap__Set_Red(Temp);
                Heap__Rot_Tree_Left(Tree, Temp);
            }
        }
    }
    
    Heap__Set_Black(Tree->Root);
}

void Heap__Add_Free_Block(heap* Heap, heap_block* Block)
{
    Assert(!Block->Node);
    
    heap_block_tree* Tree = &Heap->FreeBlockTree;
    
    heap_block_node* Node = Tree->Root;
    heap_block_node* Parent = NULL;
    
    while(Node)
    {
        if(Block->Size <= Node->Block->Size)
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
    
    Node = Heap__Create_Tree_Node(Heap);
    Block->Node = Node;
    Node->Block = Block;
    
    Heap__Set_Parent(Node, Parent);
    if(Parent == NULL)
    {
        Tree->Root = Node;
        Heap__Set_Black(Node);
    }
    else
    {
        if(Block->Size <= Parent->Block->Size) Parent->LeftChild  = Node;
        else                                   Parent->RightChild = Node;
        Heap__Fixup_Tree_Add(Tree, Node);
    }
}

void Heap__Fixup_Tree_Remove(heap_block_tree* Tree, heap_block_node* Node, heap_block_node* Parent, bool32_t ChooseLeft)
{
    while(Node != Tree->Root && (Node == NULL || Heap__Get_Color(Node) == HEAP__BLACK_NODE))
    {
        if(ChooseLeft)
        {
            heap_block_node* Temp = Parent->RightChild;
            if(Heap__Get_Color(Temp) == HEAP__RED_NODE)
            {
                Heap__Set_Black(Temp);
                Heap__Set_Red(Parent);
                Heap__Rot_Tree_Left(Tree, Parent);
                Temp = Parent->RightChild;
            }
            
            if((Temp->LeftChild == NULL  || Heap__Get_Color(Temp->LeftChild)  == HEAP__BLACK_NODE) &&
               (Temp->RightChild == NULL || Heap__Get_Color(Temp->RightChild) == HEAP__BLACK_NODE))
            {
                Heap__Set_Red(Temp);
                Node = Parent;
                Parent = Heap__Get_Parent(Parent);
                ChooseLeft = Parent && (Parent->LeftChild == Node);
            }
            else
            {
                if(Temp->RightChild == NULL || Heap__Get_Color(Temp->RightChild) == HEAP__BLACK_NODE)
                {
                    Heap__Set_Black(Temp->LeftChild);
                    Heap__Set_Red(Temp);
                    Heap__Rot_Tree_Right(Tree, Temp);
                    Temp = Parent->RightChild;
                }
                
                (Heap__Get_Color(Parent) == HEAP__RED_NODE) ? Heap__Set_Red(Temp) : Heap__Set_Black(Temp);
                
                if(Temp->RightChild) Heap__Set_Black(Temp->RightChild);
                Heap__Set_Black(Parent);
                Heap__Rot_Tree_Left(Tree, Parent);
                break;
            }
        }
        else
        {
            heap_block_node* Temp = Parent->LeftChild;
            if(Heap__Get_Color(Temp) == HEAP__RED_NODE)
            {
                Heap__Set_Black(Temp);
                Heap__Set_Red(Parent);
                Heap__Rot_Tree_Right(Tree, Parent);
                Temp = Parent->LeftChild;
            }
            
            if((Temp->LeftChild  == NULL || Heap__Get_Color(Temp->LeftChild)  == HEAP__BLACK_NODE) &&
               (Temp->RightChild == NULL || Heap__Get_Color(Temp->RightChild) == HEAP__BLACK_NODE))
            {
                Heap__Set_Red(Temp);
                Node = Parent;
                Parent = Heap__Get_Parent(Parent);
                ChooseLeft = Parent && (Parent->LeftChild == Node);
            }
            else
            {
                if(Temp->LeftChild == NULL || Heap__Get_Color(Temp->LeftChild) == HEAP__BLACK_NODE)
                {
                    Heap__Set_Black(Temp->RightChild);
                    Heap__Set_Red(Temp);
                    Heap__Rot_Tree_Left(Tree, Temp);
                    Temp = Parent->LeftChild;
                }
                
                (Heap__Get_Color(Parent) == HEAP__RED_NODE) ? Heap__Set_Red(Temp) : Heap__Set_Black(Temp);
                
                if(Temp->LeftChild) Heap__Set_Black(Temp->LeftChild);
                Heap__Set_Black(Parent);
                Heap__Rot_Tree_Right(Tree, Parent);
                break;
            }
        }
    }
    
    if(Node) Heap__Set_Black(Node);
}

void Heap__Remove_Free_Block(heap* Heap, heap_block* Block)
{
    Assert(Block->Node);
    
    heap_block_tree* Tree = &Heap->FreeBlockTree;
    
    heap_block_node* Node = Block->Node;
    heap_block_node* Out = Node;
    
    if(Node->LeftChild && Node->RightChild)
    {
        Out = Heap__Get_Min_Block(Node->RightChild);
        Heap__Swap_Block_Values(Node, Out);
    }
    
    heap_block_node* ChildLink  = Out->LeftChild ? Out->LeftChild : Out->RightChild;
    heap_block_node* ParentLink = Heap__Get_Parent(Out);
    
    if(ChildLink) Heap__Set_Parent(ChildLink, ParentLink);
    
    bool32_t ChooseLeft = ParentLink && ParentLink->LeftChild == Out;
    
    *(ParentLink ? (ParentLink->LeftChild == Out ? &ParentLink->LeftChild : &ParentLink->RightChild) : &Tree->Root) = ChildLink;
    
    if(Heap__Get_Color(Out) == HEAP__BLACK_NODE && Heap->FreeBlockTree.Root)
        Heap__Fixup_Tree_Remove(Tree, ChildLink, ParentLink, ChooseLeft);
    
    Heap__Delete_Tree_Node(Heap, Out);
    Block->Node = NULL;
}

heap_block* Heap__Create_Heap_Block(heap* Heap)
{
    heap_block* Result = Heap->OrphanBlocks;
    if(Result) Heap->OrphanBlocks = Heap->OrphanBlocks->Next;
    else Result = Arena_Push_Struct(Heap->Arena, heap_block);
    Zero_Struct(Result, heap_block);
    return Result;
}

void Heap__Delete_Heap_Block(heap* Heap, heap_block* Block)
{
    if(Block->Node) Heap__Remove_Free_Block(Heap, Block);
    Block->Next = Heap->OrphanBlocks;
    Heap->OrphanBlocks = Block;
}

heap_block* Heap__Find_Best_Fitting_Block(heap* Heap, size_t Size)
{
    heap_block_node* Node   = Heap->FreeBlockTree.Root;
    heap_block_node* Result = NULL;
    while(Node)
    {
        int64_t Diff = (int64_t)Size - (int64_t)Node->Block->Size;
        if(!Diff) return Node->Block;
        
        if(Diff < 0)
        {
            Result = Node;
            Node = Node->LeftChild;
        }
        else
        {
            Node = Node->RightChild;
        }
    }
    
    if(Result->Block) return Result->Block;
    return NULL;
}

void Heap__Increase_Block_Size(heap* Heap, heap_block* Block, size_t Addend)
{
    Heap__Remove_Free_Block(Heap, Block);
    Block->Size += Addend;
    Heap__Add_Free_Block(Heap, Block);
}

void Heap__Split_Block(heap* Heap, heap_block* Block, size_t Size)
{
    int64_t BlockDiff = (int64_t)Block->Size - (int64_t)(Size+sizeof(heap_block*));
    if(BlockDiff > 0)
    {
        size_t Offset = Block->Offset + sizeof(heap_block*) + Size;
        
        heap_block* NewBlock = Heap__Create_Heap_Block(Heap);
        heap_block** NewBlockPtr = (heap_block**)(Block->Block->Memory + Offset);
        *NewBlockPtr = NewBlock;
        
        NewBlock->Block  = Block->Block;
        NewBlock->Size   = (size_t)BlockDiff;
        NewBlock->Offset = Offset;
        NewBlock->Prev   = Block;
        Block->Next      = NewBlock;
        
        Heap__Add_Free_Block(Heap, NewBlock);
    }
    
    Block->Size = Size;
    Heap__Remove_Free_Block(Heap, Block);
}

heap_block* Heap__Add_Free_Block_From_Memory_Block(heap* Heap, heap_memory_block* MemoryBlock)
{
#ifdef DEBUG_BUILD
    Assert(!MemoryBlock->FirstBlock);
#endif
    heap_block* Block = Heap__Create_Heap_Block(Heap);
    
    heap_block** BlockPtr = (heap_block**)(MemoryBlock->Memory);
    *BlockPtr = Block;
    
    Block->Block  = MemoryBlock;
    Block->Size   = MemoryBlock->Size-sizeof(heap_block*);
    Heap__Add_Free_Block(Heap, Block);
    
#ifdef DEBUG_BUILD
    MemoryBlock->FirstBlock = Block;
#endif
    
    return Block;
}

heap_block* Heap__Create_Memory_Block(heap* Heap, size_t BlockSize)
{
    heap_memory_block* MemoryBlock = (heap_memory_block*)Arena_Push(Heap->Arena, BlockSize+sizeof(heap_memory_block)+sizeof(heap_block*), MEMORY_NO_CLEAR);
    Zero_Struct(MemoryBlock, heap_memory_block);
    
    MemoryBlock->Memory = (uint8_t*)(MemoryBlock+1);
    MemoryBlock->Size   = BlockSize+sizeof(heap_block*);
    
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
    
    Heap__Split_Block(Heap, Block, Size);
    
    Assert(Block->Size == Size);
    Assert(Block->Offset + Size + sizeof(heap_block) <= Block->Block->Size);
    void* Result = Block->Block->Memory + Block->Offset + sizeof(heap_block*);
    
    if(ClearFlag == MEMORY_CLEAR) Memory_Clear(Result, Size);
    return Result;
}

void Heap_Free(heap* Heap, void* Memory)
{
    if(Memory)
    {
        heap_block** BlockPtr = (heap_block**)((heap_block**)Memory - 1);
        heap_block* Block = *BlockPtr;
        
        Assert((Block->Block->Memory + Block->Offset + sizeof(heap_block*)) == Memory);
        
        //Heap__Check_If_Block_Is_Allocated(Heap, Block);
        
        heap_block* LeftBlock  = Block->Prev;
        heap_block* RightBlock = Block->Next;
        
        bool32_t IsLeftBlockFree  = Heap__Is_Block_Free(LeftBlock);
        bool32_t IsRightBlockFree = Heap__Is_Block_Free(RightBlock);
        
        if(!IsLeftBlockFree && !IsRightBlockFree)
        {
            Heap__Add_Free_Block(Heap, Block);
        }
        else if(LeftBlock && !RightBlock)
        {
            Assert(LeftBlock->Block == Block->Block);
            Assert((LeftBlock->Offset+LeftBlock->Size+sizeof(heap_block*)) == Block->Offset);
            
            LeftBlock->Next = Block->Next;
            if(Block->Next) Block->Next->Prev = LeftBlock;
            
            Heap__Increase_Block_Size(Heap, LeftBlock, Block->Size);
            Heap__Delete_Heap_Block(Heap, Block);
        }
        else if(!LeftBlock && RightBlock)
        {
            Assert(RightBlock->Block == Block->Block);
            Assert((Block->Offset+Block->Size+sizeof(heap_block*)) == RightBlock->Offset);
            
            Block->Next = RightBlock->Next;
            if(RightBlock->Next) RightBlock->Next->Prev = Block;
            
            Block->Size += RightBlock->Size;
            Heap__Delete_Heap_Block(Heap, RightBlock);
            Heap__Add_Free_Block(Heap, Block);
        }
        else
        {
            Assert(LeftBlock->Block  == Block->Block);
            Assert(RightBlock->Block == Block->Block);
            Assert((LeftBlock->Offset+LeftBlock->Size+sizeof(heap_block*)) == Block->Offset);
            Assert((Block->Offset+Block->Size+sizeof(heap_block*)) == RightBlock->Offset);
            
            LeftBlock->Next = RightBlock->Next;
            if(RightBlock->Next) RightBlock->Next->Prev = LeftBlock;
            Heap__Increase_Block_Size(Heap, LeftBlock, Block->Size+RightBlock->Size);
            Heap__Delete_Heap_Block(Heap, Block);
            Heap__Delete_Heap_Block(Heap, RightBlock);
        }
    }
}

void Heap_Clear(heap* Heap, memory_clear_flag ClearFlag)
{
    Arena_Clear(Heap->Arena, ClearFlag);
    Zero_Struct(&Heap->FreeBlockTree, heap_block_tree);
    
#ifdef DEBUG_BUILD
    Heap->AllocatedList = NULL;
#endif
    
    for(heap_memory_block* MemoryBlock = Heap->FirstBlock; MemoryBlock; MemoryBlock = MemoryBlock->Next)
    {
#ifdef DEBUG_BUILD
        MemoryBlock->FirstBlock = NULL;
#endif
        Heap__Add_Free_Block_From_Memory_Block(Heap, MemoryBlock);
    }
}

#ifdef DEBUG_BUILD
bool8_t Heap__Has_Allocated_Block(heap* Heap, heap_block* Block)
{
    return true;
}

bool8_t Heap__Tree_Verify(heap_block_tree* Tree, heap_block_node* Parent, heap_block_node* Node, uint32_t BlackNodeCount, uint32_t LeafBlackNodeCount)
{
    if(Parent == NULL)
    {
        Heap__Verify(Tree->Root == Node);
        Heap__Verify(Node == NULL || Heap__Get_Color(Node) == HEAP__BLACK_NODE);
    }
    else
    {
        Heap__Verify(Parent->LeftChild == Node || Parent->RightChild == Node);
    }
    
    if(Node)
    {
        Heap__Verify(Heap__Get_Parent(Node) == Parent);
        if(Parent)
        {
            if(Parent->LeftChild == Node)
                Heap__Verify(Parent->Block->Size >= Node->Block->Size);
            else
            {
                Heap__Verify(Parent->RightChild == Node);
                Heap__Verify(Parent->Block->Size < Node->Block->Size);
            }
        }
        
        if(Heap__Get_Color(Node) == HEAP__RED_NODE)
        {
            if(Node->LeftChild)  Heap__Verify(Heap__Get_Color(Node->LeftChild)  == HEAP__BLACK_NODE);
            if(Node->RightChild) Heap__Verify(Heap__Get_Color(Node->RightChild) == HEAP__BLACK_NODE);
        }
        else
        {
            BlackNodeCount++;
        }
        
        if(!Node->LeftChild && !Node->RightChild)
            Heap__Verify(BlackNodeCount == LeafBlackNodeCount);
        
        bool32_t LeftResult  = Heap__Tree_Verify(Tree, Node, Node->LeftChild,  BlackNodeCount, LeafBlackNodeCount);
        bool32_t RightResult = Heap__Tree_Verify(Tree, Node, Node->RightChild, BlackNodeCount, LeafBlackNodeCount);
        return LeftResult && RightResult;
    }
    
    return true;
}

bool8_t Heap_Verify(heap* Heap)
{
    heap_block_tree* Tree = &Heap->FreeBlockTree;
    if(Tree->Root) Heap__Verify(Heap__Get_Color(Tree->Root) == HEAP__BLACK_NODE);
    
    uint32_t LeafBlackNodeCount = 0;
    if(Tree->Root)
    {
        for(heap_block_node* Node = Tree->Root; Node; Node = Node->LeftChild)
        {
            if(Heap__Get_Color(Node) == HEAP__BLACK_NODE)
                LeafBlackNodeCount++;
        }
    }
    
    return Heap__Tree_Verify(Tree, NULL, Tree->Root, 0, LeafBlackNodeCount);
}
#endif