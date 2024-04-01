//NOTE(EVERYONE): Helper red black tree macros
#define GDI_HEAP__RED_NODE 0
#define GDI_HEAP__BLACK_NODE 1

#define GDI_Heap__Get_Parent(Node) ((gdi_heap_block_node*)((Node)->Color & ~GDI_HEAP__BLACK_NODE))
#define GDI_Heap__Get_Color(Node) ((Node)->Color & GDI_HEAP__BLACK_NODE)

#define GDI_Heap__Set_Red(Node)            ((Node)->Color &= (~(size_t)GDI_HEAP__BLACK_NODE))
#define GDI_Heap__Set_Black(Node)          ((Node)->Color |= ((size_t)GDI_HEAP__BLACK_NODE))
#define GDI_Heap__Set_Parent(Node, Parent) ((Node)->Color = GDI_Heap__Get_Color(Node) | (size_t)(Parent))

//~NOTE(EVERYONE): Heap red-black tree implementation
internal inline bool GDI_Heap__Is_Block_Free(gdi_heap_block* Block) {
    return Block && Block->Node;
}

internal gdi_heap_block_node* GDI_Heap__Get_Min_Block(gdi_heap_block_node* Node) {
    if(!Node) return NULL;
    while(Node->LeftChild) Node = Node->LeftChild;
    return Node;
}

internal void GDI_Heap__Swap_Block_Values(gdi_heap_block_node* NodeA, gdi_heap_block_node* NodeB) {
    gdi_heap_block* Tmp = NodeB->Block;
    NodeB->Block    = NodeA->Block;
    NodeA->Block    = Tmp;
    
    NodeA->Block->Node = NodeA;
    NodeB->Block->Node = NodeB;
}

internal void GDI_Heap__Rot_Tree_Left(gdi_heap_block_tree* Tree, gdi_heap_block_node* Node) {
    gdi_heap_block_node* RightChild = Node->RightChild;
    if((Node->RightChild = RightChild->LeftChild) != NULL)
        GDI_Heap__Set_Parent(RightChild->LeftChild, Node);
    
    gdi_heap_block_node* Parent = GDI_Heap__Get_Parent(Node);
    GDI_Heap__Set_Parent(RightChild, Parent);
    *(Parent ? (Parent->LeftChild == Node ? &Parent->LeftChild : &Parent->RightChild) : &Tree->Root) = RightChild;
    RightChild->LeftChild = Node;
    GDI_Heap__Set_Parent(Node, RightChild);
}

internal void GDI_Heap__Rot_Tree_Right(gdi_heap_block_tree* Tree, gdi_heap_block_node* Node) {
    gdi_heap_block_node* LeftChild = Node->LeftChild;
    if((Node->LeftChild = LeftChild->RightChild) != NULL)
        GDI_Heap__Set_Parent(LeftChild->RightChild, Node);
    
    gdi_heap_block_node* Parent = GDI_Heap__Get_Parent(Node);
    GDI_Heap__Set_Parent(LeftChild, Parent);
    *(Parent ? (Parent->LeftChild == Node ? &Parent->LeftChild : &Parent->RightChild) : &Tree->Root) = LeftChild;
    LeftChild->RightChild = Node;
    GDI_Heap__Set_Parent(Node, LeftChild);
}

internal gdi_heap_block_node* GDI_Heap__Create_Tree_Node(gdi_heap* Heap) {
    gdi_heap_block_tree* Tree = &Heap->FreeBlockTree;
    gdi_heap_block_node* Result = Tree->FreeNodes;
    if(Result) Tree->FreeNodes = Tree->FreeNodes->LeftChild;
    else Result = Arena_Push_Struct(Heap->FreeBlockTree.NodeArena, gdi_heap_block_node);
    Zero_Struct(Result);
    Assert((((size_t)Result) & 1) == 0);
    return Result;
}

internal void GDI_Heap__Delete_Tree_Node(gdi_heap* Heap, gdi_heap_block_node* Node) {
    Node->LeftChild = Heap->FreeBlockTree.FreeNodes;
    Heap->FreeBlockTree.FreeNodes = Node;
}

internal void GDI_Heap__Fixup_Tree_Add(gdi_heap_block_tree* Tree, gdi_heap_block_node* Node) {
    while(Node != Tree->Root && GDI_Heap__Get_Color(GDI_Heap__Get_Parent(Node)) == GDI_HEAP__RED_NODE) {
        if(GDI_Heap__Get_Parent(Node) == GDI_Heap__Get_Parent(GDI_Heap__Get_Parent(Node))->LeftChild) {
            gdi_heap_block_node* Temp = GDI_Heap__Get_Parent(GDI_Heap__Get_Parent(Node))->RightChild;
            if(Temp && (GDI_Heap__Get_Color(Temp) == GDI_HEAP__RED_NODE)) {
                GDI_Heap__Set_Black(Temp);
                Node = GDI_Heap__Get_Parent(Node);
                GDI_Heap__Set_Black(Node);
                Node = GDI_Heap__Get_Parent(Node);
                GDI_Heap__Set_Red(Node);
            }
            else {
                if(Node == GDI_Heap__Get_Parent(Node)->RightChild) {
                    Node = GDI_Heap__Get_Parent(Node);
                    GDI_Heap__Rot_Tree_Left(Tree, Node);
                }
                
                Temp = GDI_Heap__Get_Parent(Node);
                GDI_Heap__Set_Black(Temp);
                Temp = GDI_Heap__Get_Parent(Temp);
                GDI_Heap__Set_Red(Temp);
                GDI_Heap__Rot_Tree_Right(Tree, Temp);
            }
        }
        else {
            gdi_heap_block_node* Temp = GDI_Heap__Get_Parent(GDI_Heap__Get_Parent(Node))->LeftChild;
            if(Temp && (GDI_Heap__Get_Color(Temp) == GDI_HEAP__RED_NODE)) {
                GDI_Heap__Set_Black(Temp);
                Node = GDI_Heap__Get_Parent(Node);
                GDI_Heap__Set_Black(Node);
                Node = GDI_Heap__Get_Parent(Node);
                GDI_Heap__Set_Red(Node);
            } 
            else {
                if(Node == GDI_Heap__Get_Parent(Node)->LeftChild) {
                    Node = GDI_Heap__Get_Parent(Node);
                    GDI_Heap__Rot_Tree_Right(Tree, Node);
                }
                
                Temp = GDI_Heap__Get_Parent(Node);
                GDI_Heap__Set_Black(Temp);
                Temp = GDI_Heap__Get_Parent(Temp);
                GDI_Heap__Set_Red(Temp);
                GDI_Heap__Rot_Tree_Left(Tree, Temp);
            }
        }
    }
    
    GDI_Heap__Set_Black(Tree->Root);
}

internal void GDI_Heap__Add_Free_Block(gdi_heap* Heap, gdi_heap_block* Block) {
    Assert(!Block->Node);
    
    gdi_heap_block_tree* Tree = &Heap->FreeBlockTree;
    
    gdi_heap_block_node* Node = Tree->Root;
    gdi_heap_block_node* Parent = NULL;
    
    while(Node) {
        if(Block->Size <= Node->Block->Size) {
            Parent = Node;
            Node = Node->LeftChild;
        } 
        else {
            Parent = Node;
            Node = Node->RightChild;
        }
    }
    
    Node = GDI_Heap__Create_Tree_Node(Heap);
    Block->Node = Node;
    Node->Block = Block;
    
    GDI_Heap__Set_Parent(Node, Parent);
    if(Parent == NULL) {
        Tree->Root = Node;
        GDI_Heap__Set_Black(Node);
    }
    else {
        if(Block->Size <= Parent->Block->Size) Parent->LeftChild  = Node;
        else                                   Parent->RightChild = Node;
        GDI_Heap__Fixup_Tree_Add(Tree, Node);
    }
}

internal void GDI_Heap__Fixup_Tree_Remove(gdi_heap_block_tree* Tree, gdi_heap_block_node* Node, gdi_heap_block_node* Parent, bool ChooseLeft) {
    while(Node != Tree->Root && (Node == NULL || GDI_Heap__Get_Color(Node) == GDI_HEAP__BLACK_NODE)) {
        if(ChooseLeft) {
            gdi_heap_block_node* Temp = Parent->RightChild;
            if(GDI_Heap__Get_Color(Temp) == GDI_HEAP__RED_NODE) {
                GDI_Heap__Set_Black(Temp);
                GDI_Heap__Set_Red(Parent);
                GDI_Heap__Rot_Tree_Left(Tree, Parent);
                Temp = Parent->RightChild;
            }
            
            if((Temp->LeftChild == NULL  || GDI_Heap__Get_Color(Temp->LeftChild)  == GDI_HEAP__BLACK_NODE) &&
               (Temp->RightChild == NULL || GDI_Heap__Get_Color(Temp->RightChild) == GDI_HEAP__BLACK_NODE)) {
                GDI_Heap__Set_Red(Temp);
                Node = Parent;
                Parent = GDI_Heap__Get_Parent(Parent);
                ChooseLeft = Parent && (Parent->LeftChild == Node);
            }
            else {
                if(Temp->RightChild == NULL || GDI_Heap__Get_Color(Temp->RightChild) == GDI_HEAP__BLACK_NODE) {
                    GDI_Heap__Set_Black(Temp->LeftChild);
                    GDI_Heap__Set_Red(Temp);
                    GDI_Heap__Rot_Tree_Right(Tree, Temp);
                    Temp = Parent->RightChild;
                }
                
                (GDI_Heap__Get_Color(Parent) == GDI_HEAP__RED_NODE) ? GDI_Heap__Set_Red(Temp) : GDI_Heap__Set_Black(Temp);
                
                if(Temp->RightChild) GDI_Heap__Set_Black(Temp->RightChild);
                GDI_Heap__Set_Black(Parent);
                GDI_Heap__Rot_Tree_Left(Tree, Parent);
                break;
            }
        }
        else {
            gdi_heap_block_node* Temp = Parent->LeftChild;
            if(GDI_Heap__Get_Color(Temp) == GDI_HEAP__RED_NODE) {
                GDI_Heap__Set_Black(Temp);
                GDI_Heap__Set_Red(Parent);
                GDI_Heap__Rot_Tree_Right(Tree, Parent);
                Temp = Parent->LeftChild;
            }
            
            if((Temp->LeftChild  == NULL || GDI_Heap__Get_Color(Temp->LeftChild)  == GDI_HEAP__BLACK_NODE) &&
               (Temp->RightChild == NULL || GDI_Heap__Get_Color(Temp->RightChild) == GDI_HEAP__BLACK_NODE)) {
                GDI_Heap__Set_Red(Temp);
                Node = Parent;
                Parent = GDI_Heap__Get_Parent(Parent);
                ChooseLeft = Parent && (Parent->LeftChild == Node);
            }
            else {
                if(Temp->LeftChild == NULL || GDI_Heap__Get_Color(Temp->LeftChild) == GDI_HEAP__BLACK_NODE) {
                    GDI_Heap__Set_Black(Temp->RightChild);
                    GDI_Heap__Set_Red(Temp);
                    GDI_Heap__Rot_Tree_Left(Tree, Temp);
                    Temp = Parent->LeftChild;
                }
                
                (GDI_Heap__Get_Color(Parent) == GDI_HEAP__RED_NODE) ? GDI_Heap__Set_Red(Temp) : GDI_Heap__Set_Black(Temp);
                
                if(Temp->LeftChild) GDI_Heap__Set_Black(Temp->LeftChild);
                GDI_Heap__Set_Black(Parent);
                GDI_Heap__Rot_Tree_Right(Tree, Parent);
                break;
            }
        }
    }
    
    if(Node) GDI_Heap__Set_Black(Node);
}

internal void GDI_Heap__Remove_Free_Block(gdi_heap* Heap, gdi_heap_block* Block) {
    Assert(Block->Node);
    
    gdi_heap_block_tree* Tree = &Heap->FreeBlockTree;
    
    gdi_heap_block_node* Node = Block->Node;
    Assert(Node->Block == Block);
    
    gdi_heap_block_node* Out = Node;
    
    if(Node->LeftChild && Node->RightChild) {
        Out = GDI_Heap__Get_Min_Block(Node->RightChild);
        GDI_Heap__Swap_Block_Values(Node, Out);
    }
    
    gdi_heap_block_node* ChildLink  = Out->LeftChild ? Out->LeftChild : Out->RightChild;
    gdi_heap_block_node* ParentLink = GDI_Heap__Get_Parent(Out);
    
    if(ChildLink) GDI_Heap__Set_Parent(ChildLink, ParentLink);
    
    bool ChooseLeft = ParentLink && ParentLink->LeftChild == Out;
    
    *(ParentLink ? (ParentLink->LeftChild == Out ? &ParentLink->LeftChild : &ParentLink->RightChild) : &Tree->Root) = ChildLink;
    
    if(GDI_Heap__Get_Color(Out) == GDI_HEAP__BLACK_NODE && Heap->FreeBlockTree.Root)
        GDI_Heap__Fixup_Tree_Remove(Tree, ChildLink, ParentLink, ChooseLeft);
    
    GDI_Heap__Delete_Tree_Node(Heap, Out);
    Block->Node = NULL;
}

internal gdi_heap_block* GDI_Heap__Create_Heap_Block(gdi_heap* Heap) {
    gdi_heap_block* Result = Heap->OrphanBlocks;
    if(Result) Heap->OrphanBlocks = Heap->OrphanBlocks->Next;
    else Result = Arena_Push_Struct(Heap->Arena, gdi_heap_block);
    Zero_Struct(Result);
    return Result;
}

internal void GDI_Heap__Delete_Heap_Block(gdi_heap* Heap, gdi_heap_block* Block) {
    if(Block->Node) GDI_Heap__Remove_Free_Block(Heap, Block);
    Block->Next = Heap->OrphanBlocks;
    Heap->OrphanBlocks = Block;
}

internal gdi_heap_block* GDI_Heap__Find_Best_Fitting_Block(gdi_heap* Heap, size_t Size) {
    gdi_heap_block_node* Node   = Heap->FreeBlockTree.Root;
    gdi_heap_block_node* Result = NULL;
    while(Node)
    {
        int64_t Diff = (int64_t)Size - (int64_t)Node->Block->Size;
        if(!Diff) return Node->Block;
        
        if(Diff < 0) {
            Result = Node;
            Node = Node->LeftChild;
        }
        else {
            Node = Node->RightChild;
        }
    }
    
    if(Result) return Result->Block;
    return NULL;
}

internal void GDI_Heap__Increase_Block_Size(gdi_heap* Heap, gdi_heap_block* Block, size_t Addend) {
    GDI_Heap__Remove_Free_Block(Heap, Block);
    Block->Size += Addend;
    GDI_Heap__Add_Free_Block(Heap, Block);
}

internal void GDI_Heap__Split_Block(gdi_heap* Heap, gdi_heap_block* Block, size_t Size) {
    //NOTE(EVERYONE): When splitting a block, it is super important to check that the block has the requested size
    //plus a pointer size. We need to make sure that each new block has a pointer to its metadata before the actual
    //memory block starts. If the block is not large enough, we cannot split the block
    int64_t BlockDiff = (int64_t)Block->Size - (int64_t)Size;
    if(BlockDiff > 0) {
        size_t Offset = Block->Offset + Size;
        
        //NOTE(EVERYONE): If we can split the block, add the blocks metadata to the beginning of the blocks memory
        gdi_heap_block* NewBlock = GDI_Heap__Create_Heap_Block(Heap);
        
        NewBlock->Block  = Block->Block;
        NewBlock->Size   = (size_t)BlockDiff;
        NewBlock->Offset = Offset;
        NewBlock->Prev   = Block;
        NewBlock->Next   = Block->Next;
        if(NewBlock->Next) NewBlock->Next->Prev = NewBlock;
        Block->Next      = NewBlock;
        Block->Size      = Size;
        
        GDI_Heap__Add_Free_Block(Heap, NewBlock);
    }
    
    GDI_Heap__Remove_Free_Block(Heap, Block);
}

internal gdi_heap_block* GDI_Heap__Add_Free_Block_From_Memory_Block(gdi_heap* Heap, gdi_heap_memory_block* MemoryBlock) {
#ifdef DEBUG_BUILD
    Assert(!MemoryBlock->FirstBlock);
#endif
    gdi_heap_block* Block = GDI_Heap__Create_Heap_Block(Heap);
    
    //NOTE(EVERYONE): Similar to splitting a block. When a new memory block is created, the underlying heap block
    //needs to have its sized take into accout the heaps metadata. So we always shrink the block by a pointer size
    Block->Block  = MemoryBlock;
    Block->Size   = MemoryBlock->Size;
    GDI_Heap__Add_Free_Block(Heap, Block);
    
#ifdef DEBUG_BUILD
    //NOTE(EVERYONE): To verify that the heap offsets are working properly. Each memory block contains
    //a pointer to its first heap block in the memory block. These will then get properly split when
    //allocations are requested
    MemoryBlock->FirstBlock = Block;
#endif
    
    return Block;
}


internal gdi_heap_block* GDI_Heap__Create_Memory_Block(gdi_heap* Heap, size_t BlockSize) {
    //NOTE(EVERYONE): When we allocate a memory block, we need the block size, plus the memory block metadata, and
    //addtional pointer size so we can add an underlying heap block that is BlockSize
    gdi_heap_memory_block* MemoryBlock = GDI_Heap_Allocate_Memory_Block(Heap, BlockSize);
    if(!MemoryBlock) {
        return NULL;
    }
    MemoryBlock->Size = BlockSize;
    MemoryBlock->Next = Heap->FirstBlock;
    Heap->FirstBlock  = MemoryBlock;
    
    return GDI_Heap__Add_Free_Block_From_Memory_Block(Heap, MemoryBlock);
}

#ifdef DEBUG_BUILD
internal void GDI_Heap__Add_To_Allocated_List(gdi_heap* Heap, gdi_heap_block* Block) {
    if(Heap->AllocatedList) Heap->AllocatedList->PrevAllocated = Block;
    Block->NextAllocated = Heap->AllocatedList;
    Heap->AllocatedList = Block;
}

internal bool GDI_Heap__Is_Block_Allocated(gdi_heap* Heap, gdi_heap_block* Block)
{
    for(gdi_heap_block* TargetBlock = Heap->AllocatedList; TargetBlock; TargetBlock = TargetBlock->NextAllocated)
        if(Block == TargetBlock) return true;
    return false;
}

internal void GDI_Heap__Remove_Allocated_Block(gdi_heap* Heap, gdi_heap_block* Block) {
    if(Block->PrevAllocated) Block->PrevAllocated->NextAllocated = Block->NextAllocated;
    if(Block->NextAllocated) Block->NextAllocated->PrevAllocated = Block->PrevAllocated;
    if(Block == Heap->AllocatedList) Heap->AllocatedList = Heap->AllocatedList->NextAllocated;
    Block->PrevAllocated = Block->NextAllocated = NULL;
}


#define GDI_Heap__Verify_Check(Condition) \
do \
{ \
if(!(Condition)) \
return false; \
} while(0)

internal bool GDI_Heap__Tree_Verify(gdi_heap* Heap, gdi_heap_block_node* Parent, gdi_heap_block_node* Node, u32 BlackNodeCount, u32 LeafBlackNodeCount) {
    gdi_heap_block_tree* Tree = &Heap->FreeBlockTree;
    if(Parent == NULL) {
        GDI_Heap__Verify_Check(Tree->Root == Node);
        GDI_Heap__Verify_Check(Node == NULL || GDI_Heap__Get_Color(Node) == GDI_HEAP__BLACK_NODE);
    }
    else {
        GDI_Heap__Verify_Check(Parent->LeftChild == Node || Parent->RightChild == Node);
    }
    
    if(Node) {
        GDI_Heap__Verify_Check(GDI_Heap__Get_Parent(Node) == Parent);
        if(Parent) {
            if(Parent->LeftChild == Node)
                GDI_Heap__Verify_Check(Parent->Block->Size >= Node->Block->Size);
            else {
                GDI_Heap__Verify_Check(Parent->RightChild == Node);
                GDI_Heap__Verify_Check(Parent->Block->Size <= Node->Block->Size);
            }
        }
        
        if(GDI_Heap__Get_Color(Node) == GDI_HEAP__RED_NODE) {
            if(Node->LeftChild)  GDI_Heap__Verify_Check(GDI_Heap__Get_Color(Node->LeftChild)  == GDI_HEAP__BLACK_NODE);
            if(Node->RightChild) GDI_Heap__Verify_Check(GDI_Heap__Get_Color(Node->RightChild) == GDI_HEAP__BLACK_NODE);
        }
        else {
            BlackNodeCount++;
        }
        
        if(!Node->LeftChild && !Node->RightChild)
            GDI_Heap__Verify_Check(BlackNodeCount == LeafBlackNodeCount);
        
        bool LeftResult  = GDI_Heap__Tree_Verify(Heap, Node, Node->LeftChild,  BlackNodeCount, LeafBlackNodeCount);
        bool RightResult = GDI_Heap__Tree_Verify(Heap, Node, Node->RightChild, BlackNodeCount, LeafBlackNodeCount);
        return LeftResult && RightResult;
    }
    
    return true;
}

internal bool GDI_Heap__Verify_Check_Offsets(gdi_heap* Heap) {
    for(gdi_heap_memory_block* MemoryBlock = Heap->FirstBlock; MemoryBlock; MemoryBlock = MemoryBlock->Next) {
        gdi_heap_block* Block = MemoryBlock->FirstBlock;
        if(Block->Prev) { return false; }
        
        size_t Offset = 0;
        while(Block) {
            if(Block->Offset != Offset) { return false; }
            Offset += Block->Size;
            Block = Block->Next;
            if(!Block)  if(Offset != MemoryBlock->Size) { return false; }
        }
    }
    
    return true;
}

bool GDI_Heap_Verify(gdi_heap* Heap) {
    bool OffsetVerified = GDI_Heap__Verify_Check_Offsets(Heap);
    if(!OffsetVerified) return false;
    
    gdi_heap_block_tree* Tree = &Heap->FreeBlockTree;
    if(Tree->Root) GDI_Heap__Verify_Check(GDI_Heap__Get_Color(Tree->Root) == GDI_HEAP__BLACK_NODE);
    
    uint32_t LeafBlackNodeCount = 0;
    if(Tree->Root) {
        for(gdi_heap_block_node* Node = Tree->Root; Node; Node = Node->LeftChild) {
            if(GDI_Heap__Get_Color(Node) == GDI_HEAP__BLACK_NODE)
                LeafBlackNodeCount++;
        }
    }
    
    bool TreeVerified = GDI_Heap__Tree_Verify(Heap, NULL, Tree->Root, 0, LeafBlackNodeCount);
    return TreeVerified;
}

#else
bool GDI_Heap_Verify(gdi_heap* Heap) {
    return true;
}
#endif

void GDI_Heap_Create(gdi_heap* Heap, allocator* Allocator, uptr InitialBlockSize, gdi_heap_vtable* VTable) {
    Zero_Struct(Heap);
    AK_Mutex_Create(&Heap->Mutex);
    Heap->Arena = Arena_Create(Allocator);
    Heap->FreeBlockTree.NodeArena = Arena_Create(Allocator);
    Heap->VTable = VTable;
    Heap->InitialBlockSize = InitialBlockSize;
}

void GDI_Heap_Delete(gdi_heap* Heap) {
    gdi_heap_memory_block* MemoryBlock = Heap->FirstBlock;
    while(MemoryBlock) {
        gdi_heap_memory_block* BlockToDelete = MemoryBlock;
        MemoryBlock = BlockToDelete->Next;
        GDI_Heap_Free_Memory_Block(Heap, BlockToDelete);
    }

    Arena_Delete(Heap->Arena);
    Arena_Delete(Heap->FreeBlockTree.NodeArena);
    AK_Mutex_Delete(&Heap->Mutex);
    Zero_Struct(Heap);
}

gdi_heap_block* GDI_Heap_Allocate_Internal(gdi_heap* Heap, uptr Size) { 
    //NOTE(EVERYONE): Allocating a block conceptually is a super simple process.
    //1. Find the best fitting block. 
    //2. Split it if the size requested is smaller than the block size
    //3. Get the pointer from the heap block
    
    if(!Size) return NULL;
    gdi_heap_block* Block = GDI_Heap__Find_Best_Fitting_Block(Heap, Size);
    if(!Block) {
        size_t BlockSize = Max(Size, Heap->InitialBlockSize);
        Block = GDI_Heap__Create_Memory_Block(Heap, BlockSize);
        if(!Block) return NULL;
    }
    
    GDI_Heap__Split_Block(Heap, Block, Size);
    
    //NOTE(EVERYONE): The block offset always starts at the beginning of the blocks metadata.
    //This is not where the returned memory starts. It always starts right after the metadata 
    //so we must offset an additional pointer size
    Assert(Block->Offset + Size <= Block->Block->Size);
    
#ifdef DEBUG_BUILD
    GDI_Heap__Add_To_Allocated_List(Heap, Block);
    GDI_Heap_Verify(Heap);
#endif
    
    return Block;
}

void GDI_Heap_Free_Internal(gdi_heap* Heap, gdi_heap_block* Block) {
    if(Block) {
        //NOTE(EVERYTONE): Freeing a block is a little more complex
        //1. Get the block's metadata from the memory address. Validate that its the correct block
        //2. Check to see if the blockers neighboring blocks are free. And do the following:
        //    -If no neighboring blocks are free, add the block to the free block tree
        //    -If the left block is free, merge the left and current block, and delete the current block
        //    -If the right block is free, merge the current and right block, and delete the right block
        //    -If both are free, merge left block with the current and right block, and delete the current and right block
        
#ifdef DEBUG_BUILD
        Assert(GDI_Heap__Is_Block_Allocated(Heap, Block));
        GDI_Heap__Remove_Allocated_Block(Heap, Block);
#endif

        gdi_heap_block* LeftBlock  = Block->Prev;
        gdi_heap_block* RightBlock = Block->Next;
        
        bool IsLeftBlockFree  = GDI_Heap__Is_Block_Free(LeftBlock);
        bool IsRightBlockFree = GDI_Heap__Is_Block_Free(RightBlock);
        
        if(!IsLeftBlockFree && !IsRightBlockFree) {
            GDI_Heap__Add_Free_Block(Heap, Block);
        }
        else if(IsLeftBlockFree && !IsRightBlockFree) {
            Assert(LeftBlock->Block == Block->Block);
            Assert((LeftBlock->Offset+LeftBlock->Size) == Block->Offset);
            
            LeftBlock->Next = Block->Next;
            if(Block->Next) Block->Next->Prev = LeftBlock;
            
            GDI_Heap__Increase_Block_Size(Heap, LeftBlock, Block->Size);
            GDI_Heap__Delete_Heap_Block(Heap, Block);
        }
        else if(!IsLeftBlockFree && IsRightBlockFree) {
            Assert(RightBlock->Block == Block->Block);
            Assert((Block->Offset+Block->Size) == RightBlock->Offset);
            
            Block->Next = RightBlock->Next;
            if(RightBlock->Next) RightBlock->Next->Prev = Block;
            
            Block->Size += RightBlock->Size;
            GDI_Heap__Delete_Heap_Block(Heap, RightBlock);
            GDI_Heap__Add_Free_Block(Heap, Block);
        }
        else {
            Assert(LeftBlock->Block  == Block->Block);
            Assert(RightBlock->Block == Block->Block);
            Assert((LeftBlock->Offset+LeftBlock->Size) == Block->Offset);
            Assert((Block->Offset+Block->Size) == RightBlock->Offset);
            
            size_t Addend = Block->Size+RightBlock->Size;
            
            LeftBlock->Next = RightBlock->Next;
            if(RightBlock->Next) RightBlock->Next->Prev = LeftBlock;
            GDI_Heap__Increase_Block_Size(Heap, LeftBlock, Addend);
            GDI_Heap__Delete_Heap_Block(Heap, Block);
            GDI_Heap__Delete_Heap_Block(Heap, RightBlock);
        }
        
#ifdef DEBUG_BUILD
        Assert(GDI_Heap_Verify(Heap));
#endif
    }
}

gdi_allocate GDI_Heap_Allocate(gdi_heap* Heap, uptr Size, uptr Alignment) {
    Assert(Alignment > 0 && Is_Pow2(Alignment));

    uptr Offset = Alignment-1;
    AK_Mutex_Lock(&Heap->Mutex);
    gdi_heap_block* Block = GDI_Heap_Allocate_Internal(Heap, Size+Offset);
    AK_Mutex_Unlock(&Heap->Mutex);
    if(!Block) return {};

    gdi_allocate Result = {Block};
    Result.Offset = (Block->Offset + Offset) & ~(Alignment-1);
    return Result;
}

void GDI_Heap_Free(gdi_heap* Heap, gdi_allocate* Allocate) {
    if(Allocate->Block) {
        AK_Mutex_Lock(&Heap->Mutex);
        GDI_Heap_Free_Internal(Heap, Allocate->Block);
        AK_Mutex_Unlock(&Heap->Mutex);
        Zero_Struct(Allocate);
    }
}