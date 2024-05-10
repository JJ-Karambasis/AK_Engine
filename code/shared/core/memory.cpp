buffer::buffer(void* Data, uptr BufferSize) : Ptr((u8*)Data), Size(BufferSize) { }

buffer::buffer(allocator* Allocator, uptr BufferSize) {
    Ptr = (u8*)Allocator_Allocate_Memory(Allocator, BufferSize);
    Size = BufferSize;
}

bool buffer::Is_Empty() const {
    return !Ptr || !Size;
}

const_buffer::const_buffer(const void* Data, uptr BufferSize) : Ptr((const u8*)Data), Size(BufferSize) { }

const_buffer::const_buffer(const buffer& Buffer) : Ptr(Buffer.Ptr), Size(Buffer.Size) { }

void* Memory_Copy(void* Dst, const void* Src, uptr Size) {
    return memcpy(Dst, Src, Size);
}

void* Memory_Clear(void* Memory, uptr Size) {
    return memset(Memory, 0, Size);
}

void* Memory_Set(void* Memory, u8 Value, uptr Size) {
    return memset(Memory, 0, Size);
}

void* Allocator_Allocate_Memory(allocator* Allocator, uptr Size, allocator_clear_flag ClearFlag) {
    void* Result = Allocator_Allocate_Aligned_Memory(Allocator, Size, DEFAULT_ALIGNMENT, ALLOCATOR_CLEAR_FLAG_NO_CLEAR);
    if(Result) {
        Memory_Clear(Result, Size);
    }
    return Result;
}

void* Allocator_Allocate_Aligned_Memory(allocator* Allocator, uptr Size, uptr Alignment, allocator_clear_flag ClearFlag) {
    Assert(Alignment > 0 && Is_Pow2(Alignment));

    uptr Offset = Alignment - 1 + sizeof(void*);
    void* P1 = Allocator_Allocate_Memory_Internal(Allocator, Size+Offset);
    if(!P1) return NULL;

    void** P2 = (void**)(((uptr)(P1) + Offset) & ~(Alignment - 1));
    P2[-1] = P1;

    if(ClearFlag == ALLOCATOR_CLEAR_FLAG_CLEAR) {
        Memory_Clear(P2, Size);
    }
         
    return P2;
}

void Allocator_Free_Memory(allocator* Allocator, void* Memory) {
    if(Memory) {
        void** P2 = (void**)Memory;
        Allocator_Free_Memory_Internal(Allocator, P2[-1]);
    }
}

internal ALLOCATOR_ALLOCATE_MEMORY_DEFINE(Virtual_Allocator_Allocate_Memory) {
    virtual_allocator* VirtualAllocator = (virtual_allocator*)Allocator;
    uptr TotalSize = Size+sizeof(uptr);
    uptr* Memory = (uptr*)Virtual_Allocator_Reserve(VirtualAllocator, TotalSize);
    if(Memory) {
        if(Virtual_Allocator_Commit(VirtualAllocator, Memory, TotalSize)) {
            *Memory = Size;
            return Memory+1;
        }
        Virtual_Allocator_Release(VirtualAllocator, Memory, TotalSize);
    }
    return NULL;
}

internal ALLOCATOR_FREE_MEMORY_DEFINE(Virtual_Allocator_Free_Memory) {
    virtual_allocator* VirtualAllocator = (virtual_allocator*)Allocator;
    if(Memory) {
        uptr* MemoryPtr = ((uptr*)Memory)-1;
        uptr TotalSize = *MemoryPtr + sizeof(uptr);
        Virtual_Allocator_Decommit(VirtualAllocator, MemoryPtr, TotalSize);
        Virtual_Allocator_Release(VirtualAllocator, MemoryPtr, TotalSize);
    }
}

internal ALLOCATOR_GET_STATS_DEFINE(Virtual_Allocator_Get_Stats) {
    virtual_allocator* VirtualAllocator = (virtual_allocator*)Allocator;
    allocator_stats Result = {
        (uptr)(AK_Atomic_Load_U64_Relaxed(&VirtualAllocator->TotalReservedMemory)), 
        (uptr)(AK_Atomic_Load_U64_Relaxed(&VirtualAllocator->TotalCommittedMemory))
    };
    return Result;
}

global allocator_vtable G_VirtualAllocatorVTable = {
    Virtual_Allocator_Allocate_Memory,
    Virtual_Allocator_Free_Memory,
    Virtual_Allocator_Get_Stats
};

#if defined(OS_WIN32)
virtual_allocator* Virtual_Allocator_Create() {
    virtual_allocator* Result = (virtual_allocator*)VirtualAlloc(NULL, sizeof(virtual_allocator), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Memory_Clear(Result, sizeof(virtual_allocator));
    Result->VTable = &G_VirtualAllocatorVTable;
    return Result;
}

void Virtual_Allocator_Delete(virtual_allocator* VirtualAllocator) {
    Allocator_Delete__Tracker((allocator*)VirtualAllocator);
    VirtualFree(VirtualAllocator, 0, MEM_RELEASE);
}

void* Virtual_Allocator_Reserve(virtual_allocator* VirtualAllocator, uptr Size) {
    void* Result = VirtualAlloc(NULL, Size, MEM_RESERVE, PAGE_READWRITE);
    if(Result) {
        s64 SignedSize = (s64)Size;
        AK_Atomic_Fetch_Add_U64_Relaxed(&VirtualAllocator->TotalReservedMemory, SignedSize);
    }
    return Result;
}

void* Virtual_Allocator_Commit(virtual_allocator*  VirtualAllocator, void* Memory, uptr Size) {
    void* Result = VirtualAlloc(Memory, Size, MEM_COMMIT, PAGE_READWRITE);
    if(Result) {
        s64 SignedSize = (s64)Size;
        AK_Atomic_Fetch_Add_U64_Relaxed(&VirtualAllocator->TotalCommittedMemory, SignedSize);
    }
    return Result;
}

void Virtual_Allocator_Decommit(virtual_allocator* VirtualAllocator, void* Memory, uptr Size) {
    if(VirtualFree(Memory, Size, MEM_DECOMMIT) != 0) {
        s64 SignedSize = (s64)Size;
        AK_Atomic_Fetch_Add_U64_Relaxed(&VirtualAllocator->TotalCommittedMemory, -SignedSize);
    }
}

void Virtual_Allocator_Release(virtual_allocator*  VirtualAllocator, void* Memory, uptr Size) {
    if(VirtualFree(Memory, 0, MEM_RELEASE) != 0) {
        s64 SignedSize = (s64)Size;
        AK_Atomic_Fetch_Add_U64_Relaxed(&VirtualAllocator->TotalReservedMemory, -SignedSize);
    }
}
#elif defined(OS_POSIX)
virtual_allocator* Virtual_Allocator_Create() {
    virtual_allocator* Result = mmap(NULL, sizeof(virtual_allocator), (PROT_READ|PROT_WRITE), (MAP_FIXED|MAP_SHARED|MAP_ANONYMOUS), -1, 0);
    msync(Result, sizeof(virtual_allocator), (MS_SYNC|MS_INVALIDATE));
    Result->Base.VTable = &G_VirtualAllocatorVTable;
    return Result;
}

void Virtual_Allocator_Delete(virtual_allocator* VirtualAllocator) {
    if(VirtualAllocator) {
        Allocator_Delete__Tracker((allocator*)VirtualAllocator);
        msync(VirtualAllocator, sizeof(virtual_allocator), MS_SYNC);
        munmap(VirtualAllocator, sizeof(virtual_allocator));
    }    
}

void* Virtual_Allocator_Reserve(virtual_allocator* VirtualAllocator, uptr Size) {
    void* Result = mmap(NULL, Size, PROT_NONE, (MAP_PRIVATE|MAP_ANONYMOUS), -1, 0);
    if(Result) {
        msync(Result, Size, (MS_SYNC|MS_INVALIDATE));
        AK_Atomic_Fetch_Add_U64_Relaxed(&VirtualAllocator->TotalReservedMemory, Size);
    }
    return Result;
}

void* Virtual_Allocator_Commit(virtual_allocator*  VirtualAllocator, void* Memory, uptr Size) {
    if(Memory) {
        void* Result = mmap(Memory, Size, (PROT_READ|PROT_WRITE), (MAP_FIXED|MAP_SHARED|MAP_ANONYMOUS), -1, 0);
        msync(Memory, Size, (MS_SYNC|MS_INVALIDATE));
        AK_Atomic_Fetch_Add_U64_Relaxed(&VirtualAllocator->TotalCommittedMemory, Size);
        return Result;
    }
    return NULL;
}

void Virtual_Allocator_Decommit(virtual_allocator* VirtualAllocator, void* Memory, uptr Size) {
    if(Memory) {
        mmap(Memory, Size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        msync(Memory, Size, MS_SYNC|MS_INVALIDATE);
        s64 SignedSize = (s64)Size;
        AK_Atomic_Fetch_Add_U64_Relaxed(&VirtualAllocator->TotalCommittedMemory, SignedSize);
    }
}

void Virtual_Allocator_Release(virtual_allocator* VirtualAllocator, void* Memory, uptr Size) {
    if(Memory) {
        msync(Memory, Size, MS_SYNC);
        munmap(Memory, Size);
        s64 SignedSize = (s64)Size;
        AK_Atomic_Fetch_Add_U64_Relaxed(&VirtualAllocator->TotalReservedMemory, SignedSize);
    }
}

#else
#error "Not Implemented"
#endif

void Virtual_Allocator_Track(virtual_allocator* VirtualAllocator, string DebugName) {
    Allocator_Create__Tracker(VirtualAllocator, DebugName, ALLOCATOR_TYPE_VIRTUAL);
}

internal ALLOCATOR_ALLOCATE_MEMORY_DEFINE(Arena_Allocate_Memory) {
    arena* Arena = (arena*)Allocator;
    return Arena_Push(Arena, Size, 1);
}

internal ALLOCATOR_FREE_MEMORY_DEFINE(Arena_Free_Memory) {
    //Noop
}

internal ALLOCATOR_GET_STATS_DEFINE(Arena_Get_Stats) {
    arena* Arena = (arena*)Allocator;
    allocator_stats Result = {0};

    for(arena_block* Block = Arena->FirstBlock; Block; Block = Block->Next) {
        Result.TotalReservedMemory += Block->Size;
        Result.TotalUsedMemory += Block->Used;
    }

    return Result;
}

global allocator_vtable G_ArenaVTable = {
    Arena_Allocate_Memory,
    Arena_Free_Memory,
    Arena_Get_Stats
};

internal arena_block* Arena__Get_Current_Block(arena* Arena, uptr Size, uptr Alignment) {
    arena_block* Result = Arena->CurrentBlock;
    while(Result && (Result->Size < (Align_Pow2(Result->Used, Alignment)+Size))) {
        Result = Result->Next;
    }
    return Result;
}

internal arena_block* Arena__Allocate_Block(arena* Arena, uptr BlockSize) {
    allocator* Allocator = Arena->ParentAllocator;
    arena_block* Result = (arena_block*)Allocator_Allocate_Memory(Allocator, BlockSize+sizeof(arena_block));
    Memory_Clear(Result, sizeof(arena_block));
    Result->Size = BlockSize;
    Result->Ptr = (u8*)(Result+1);
    return Result;
}

arena* Arena_Create(allocator* ParentAllocator, uptr MinimumBlockSize) {
    arena* Result = Allocator_Allocate_Struct(ParentAllocator, arena);
    Memory_Clear(Result, sizeof(arena));
    Result->VTable = &G_ArenaVTable;
    Result->ParentAllocator = ParentAllocator;
    Result->MinimumBlockSize = (MinimumBlockSize == (uptr)-1) ? KB(4) : Ceil_Pow2(MinimumBlockSize);
    return Result;
}

void Arena_Delete(arena* Arena) {
    Allocator_Delete__Tracker((allocator*)Arena);
    if(Arena->ParentAllocator) {
        allocator* Allocator = Arena->ParentAllocator;
        
        arena_block* Block = Arena->FirstBlock;
        while(Block) {
            arena_block* BlockToDelete = Block;
            Block = BlockToDelete->Next;
            Allocator_Free_Memory(Allocator, BlockToDelete);
        }

        Allocator_Free_Memory(Allocator, Arena);
    }
}

void* Arena_Push(arena* Arena, uptr Size, uptr Alignment, allocator_clear_flag ClearFlag) {
    Assert(Is_Pow2(Alignment));

    arena_block* Block = Arena__Get_Current_Block(Arena, Size, Alignment);
    if(!Block) {
        uptr BlockSize = 0;
        if(Arena->LastBlock) {
            //Increase the block size by two
            BlockSize = Arena->LastBlock->Size*2;
        }

        uptr Mask = Alignment-1;
        
        //If the block size still doesn't handle the allocation, round 
        //the block up to the nearest power of two that handles the 
        //allocation
        if(BlockSize < (Size+Mask)) {
            BlockSize = Ceil_Pow2(Size+Mask);
        }

        //If the blocksize is less than the minimum block size we will round up
        BlockSize = Max(BlockSize, Arena->MinimumBlockSize);

        Block = Arena__Allocate_Block(Arena, BlockSize);
        SLL_Push_Back(Arena->FirstBlock, Arena->LastBlock, Block);
    }

    Arena->CurrentBlock = Block;
    arena_block* CurrentBlock = Arena->CurrentBlock;

    CurrentBlock->Used = Align_Pow2(CurrentBlock->Used, Alignment);
    Assert(CurrentBlock->Used+Size <= CurrentBlock->Size);

    void* Result = CurrentBlock->Ptr + CurrentBlock->Used;
    CurrentBlock->Used += Size;

    if(ClearFlag == ALLOCATOR_CLEAR_FLAG_CLEAR) {
        Memory_Clear(Result, Size);
    }

    return Result;
}

arena_marker Arena_Get_Marker(arena* Arena) {
    arena_marker Marker = {
        Arena, 
        Arena->CurrentBlock, 
        Arena->CurrentBlock ? Arena->CurrentBlock->Used : 0
    };
    return Marker;
}

void Arena_Set_Marker(arena* Arena, arena_marker* Marker) {
    Assert(Arena == Marker->Arena);
    if(Marker->CurrentBlock) {
        Arena->CurrentBlock = Marker->CurrentBlock;
        Arena->CurrentBlock->Used = Marker->Marker;
        for(arena_block* Block = Arena->CurrentBlock->Next; Block; Block = Block->Next) {
            Block->Used = 0;
        }
    } else {
        Arena->CurrentBlock = Arena->FirstBlock;
        if(Arena->CurrentBlock) Arena->CurrentBlock->Used = 0;
    }
}

void Arena_Clear(arena* Arena) {
    for(arena_block* Block = Arena->FirstBlock; Block; Block = Block->Next) {
        Block->Used = 0;
    }
    Arena->CurrentBlock = Arena->FirstBlock;
}

void Arena_Track(arena* Arena, string DebugName) {
    Allocator_Create__Tracker((allocator*)Arena, DebugName, ALLOCATOR_TYPE_ARENA);
}

internal ALLOCATOR_ALLOCATE_MEMORY_DEFINE(Heap_Allocate_Memory) {
    heap* Heap = (heap*)Allocator;
    return Heap_Allocate(Heap, Size);
}

internal ALLOCATOR_FREE_MEMORY_DEFINE(Heap_Free_Memory) {
    heap* Heap = (heap*)Allocator;
    Heap_Free(Heap, Memory);
}

internal ALLOCATOR_GET_STATS_DEFINE(Heap_Get_Stats) {
    heap* Heap = (heap*)Allocator;
    allocator_stats Result = {0};
    for(heap_memory_block* Block = Heap->FirstBlock; Block; Block = Block->Next) {
        Result.TotalReservedMemory += Block->Size;
    }
    Result.TotalUsedMemory = Heap->CurrentAllocatedMemory;
    return Result;
}

global allocator_vtable G_HeapVTable = {
    Heap_Allocate_Memory,
    Heap_Free_Memory,
    Heap_Get_Stats
};

//NOTE(EVERYONE): Helper red black tree macros
#define HEAP__RED_NODE 0
#define HEAP__BLACK_NODE 1

#define Heap__Get_Parent(Node) ((heap_block_node*)((Node)->Color & ~HEAP__BLACK_NODE))
#define Heap__Get_Color(Node) ((Node)->Color & HEAP__BLACK_NODE)

#define Heap__Set_Red(Node)            ((Node)->Color &= (~(uptr)HEAP__BLACK_NODE))
#define Heap__Set_Black(Node)          ((Node)->Color |= ((uptr)HEAP__BLACK_NODE))
#define Heap__Set_Parent(Node, Parent) ((Node)->Color = Heap__Get_Color(Node) | (uptr)(Parent))

//~NOTE(EVERYONE): Heap red-black tree implementation
internal inline bool Heap__Is_Block_Free(heap_block* Block)
{
    return Block && Block->Node;
}

internal heap_block_node* Heap__Get_Min_Block(heap_block_node* Node)
{
    if(!Node) return NULL;
    while(Node->LeftChild) Node = Node->LeftChild;
    return Node;
}

internal void Heap__Swap_Block_Values(heap_block_node* NodeA, heap_block_node* NodeB)
{
    heap_block* Tmp = NodeB->Block;
    NodeB->Block    = NodeA->Block;
    NodeA->Block    = Tmp;
    
    NodeA->Block->Node = NodeA;
    NodeB->Block->Node = NodeB;
}

internal void Heap__Rot_Tree_Left(heap* Heap, heap_block_node* Node)
{
    heap_block_tree* Tree = &Heap->FreeBlockTree;

    heap_block_node* RightChild = Node->RightChild;
    if((Node->RightChild = RightChild->LeftChild) != NULL)
        Heap__Set_Parent(RightChild->LeftChild, Node);
    
    heap_block_node* Parent = Heap__Get_Parent(Node);
    Heap__Set_Parent(RightChild, Parent);
    *(Parent ? (Parent->LeftChild == Node ? &Parent->LeftChild : &Parent->RightChild) : &Tree->Root) = RightChild;
    RightChild->LeftChild = Node;
    Heap__Set_Parent(Node, RightChild);
}

internal void Heap__Rot_Tree_Right(heap* Heap, heap_block_node* Node)
{
    heap_block_tree* Tree = &Heap->FreeBlockTree;

    heap_block_node* LeftChild = Node->LeftChild;
    if((Node->LeftChild = LeftChild->RightChild) != NULL)
        Heap__Set_Parent(LeftChild->RightChild, Node);
    
    heap_block_node* Parent = Heap__Get_Parent(Node);
    Heap__Set_Parent(LeftChild, Parent);
    *(Parent ? (Parent->LeftChild == Node ? &Parent->LeftChild : &Parent->RightChild) : &Tree->Root) = LeftChild;
    LeftChild->RightChild = Node;
    Heap__Set_Parent(Node, LeftChild);
}

internal heap_block_node* Heap__Create_Tree_Node(heap* Heap)
{
    heap_block_tree* Tree = &Heap->FreeBlockTree;
    heap_block_node* Result = Tree->FreeNodes;
    if(Result) Tree->FreeNodes = Tree->FreeNodes->LeftChild;
    else Result = Arena_Push_Struct(Tree->NodeArena, heap_block_node);
    Memory_Clear(Result, sizeof(heap_block_node));
    Assert((((uptr)Result) & 1) == 0);
    return Result;
}

internal void Heap__Delete_Tree_Node(heap* Heap, heap_block_node* Node)
{
    Node->LeftChild = Heap->FreeBlockTree.FreeNodes;
    Heap->FreeBlockTree.FreeNodes = Node;
}

internal void Heap__Fixup_Tree_Add(heap* Heap, heap_block_node* Node)
{
    heap_block_tree* Tree = &Heap->FreeBlockTree;
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
                    Heap__Rot_Tree_Left(Heap, Node);
                }
                
                Temp = Heap__Get_Parent(Node);
                Heap__Set_Black(Temp);
                Temp = Heap__Get_Parent(Temp);
                Heap__Set_Red(Temp);
                Heap__Rot_Tree_Right(Heap, Temp);
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
                    Heap__Rot_Tree_Right(Heap, Node);
                }
                
                Temp = Heap__Get_Parent(Node);
                Heap__Set_Black(Temp);
                Temp = Heap__Get_Parent(Temp);
                Heap__Set_Red(Temp);
                Heap__Rot_Tree_Left(Heap, Temp);
            }
        }
    }
    
    Heap__Set_Black(Tree->Root);
}

internal void Heap__Add_Free_Block(heap* Heap, heap_block* Block)
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
        Heap__Fixup_Tree_Add(Heap, Node);
    }
}

internal void Heap__Fixup_Tree_Remove(heap* Heap, heap_block_node* Node, heap_block_node* Parent, bool ChooseLeft)
{
    heap_block_tree* Tree = &Heap->FreeBlockTree;
    while(Node != Tree->Root && (Node == NULL || Heap__Get_Color(Node) == HEAP__BLACK_NODE))
    {
        if(ChooseLeft)
        {
            heap_block_node* Temp = Parent->RightChild;
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
            heap_block_node* Temp = Parent->LeftChild;
            if(Heap__Get_Color(Temp) == HEAP__RED_NODE)
            {
                Heap__Set_Black(Temp);
                Heap__Set_Red(Parent);
                Heap__Rot_Tree_Right(Heap, Parent);
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
    
    if(Node) Heap__Set_Black(Node);
}

internal void Heap__Remove_Free_Block(heap* Heap, heap_block* Block)
{
    Assert(Block->Node);
    
    heap_block_tree* Tree = &Heap->FreeBlockTree;
    
    heap_block_node* Node = Block->Node;
    Assert(Node->Block == Block);
    
    heap_block_node* Out = Node;
    
    if(Node->LeftChild && Node->RightChild)
    {
        Out = Heap__Get_Min_Block(Node->RightChild);
        Heap__Swap_Block_Values(Node, Out);
    }
    
    heap_block_node* ChildLink  = Out->LeftChild ? Out->LeftChild : Out->RightChild;
    heap_block_node* ParentLink = Heap__Get_Parent(Out);
    
    if(ChildLink) Heap__Set_Parent(ChildLink, ParentLink);
    
    bool ChooseLeft = ParentLink && ParentLink->LeftChild == Out;
    
    *(ParentLink ? (ParentLink->LeftChild == Out ? &ParentLink->LeftChild : &ParentLink->RightChild) : &Tree->Root) = ChildLink;
    
    if(Heap__Get_Color(Out) == HEAP__BLACK_NODE && Tree->Root)
        Heap__Fixup_Tree_Remove(Heap, ChildLink, ParentLink, ChooseLeft);
    
    Heap__Delete_Tree_Node(Heap, Out);
    Block->Node = NULL;
}
//~End of heap red-black tree implementation
internal void Heap__Delete_Heap_Block(heap* Heap, heap_block* Block) {
    if(Block->Node) Heap__Remove_Free_Block(Heap, Block);
}

internal heap_block* Heap__Find_Best_Fitting_Block(heap* Heap, uptr Size)
{
    heap_block_node* Node   = Heap->FreeBlockTree.Root;
    heap_block_node* Result = NULL;
    while(Node)
    {
        s64 Diff = (s64)Size - (s64)Node->Block->Size;
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
    
    if(Result) return Result->Block;
    return NULL;
}

internal void Heap__Increase_Block_Size(heap* Heap, heap_block* Block, uptr Addend)
{
    Heap__Remove_Free_Block(Heap, Block);
    Block->Size += Addend;
    Heap__Add_Free_Block(Heap, Block);
}

internal void Heap__Split_Block(heap* Heap, heap_block* Block, uptr Size)
{
    //NOTE(EVERYONE): When splitting a block, it is super important to check that the block has the requested size
    //plus a pointer size. We need to make sure that each new block has a pointer to its metadata before the actual
    //memory block starts. If the block is not large enough, we cannot split the block
    s64 BlockDiff = (s64)Block->Size - (s64)(Size+sizeof(heap_block));
    if(BlockDiff > 0)
    {
        uptr Offset = Block->Offset + sizeof(heap_block) + Size;
        
        //NOTE(EVERYONE): If we can split the block, add the blocks metadata to the beginning of the blocks memory
        heap_block* NewBlock = (heap_block*)(Block->Block->Ptr + Offset);
        Memory_Clear(NewBlock, sizeof(heap_block));
        
        NewBlock->Block  = Block->Block;
        NewBlock->Size   = (uptr)BlockDiff;
        NewBlock->Offset = Offset;
        NewBlock->Prev   = Block;
        NewBlock->Next   = Block->Next;
        if(NewBlock->Next) NewBlock->Next->Prev = NewBlock;
        Block->Next      = NewBlock;
        Block->Size = Size;
        
        Heap__Add_Free_Block(Heap, NewBlock);
    }
    
    Heap__Remove_Free_Block(Heap, Block);
}

internal heap_block* Heap__Add_Free_Block_From_Memory_Block(heap* Heap, heap_memory_block* MemoryBlock)
{
#ifdef DEBUG_BUILD
    Assert(!MemoryBlock->FirstBlock);
#endif    
    heap_block* Block = (heap_block*)(MemoryBlock->Ptr);
    Memory_Clear(Block, sizeof(heap_block));
    
    //NOTE(EVERYONE): Similar to splitting a block. When a new memory block is created, the underlying heap block
    //needs to have its sized take into accout the heaps metadata. So we always shrink the block by a pointer size
    Block->Block  = MemoryBlock;
    Block->Size   = MemoryBlock->Size-sizeof(heap_block);
    Heap__Add_Free_Block(Heap, Block);
    
#ifdef DEBUG_BUILD
    //NOTE(EVERYONE): To verify that the heap offsets are working properly. Each memory block contains
    //a pointer to its first heap block in the memory block. These will then get properly split when
    //allocations are requested
    MemoryBlock->FirstBlock = Block;
#endif
    
    return Block;
}

internal heap_block* Heap__Create_Memory_Block(heap* Heap, uptr BlockSize)
{
    //NOTE(EVERYONE): When we allocate a memory block, we need the block size, plus the memory block metadata, and
    //addtional pointer size so we can add an underlying heap block that is BlockSize
    heap_memory_block* MemoryBlock = (heap_memory_block*)Allocator_Allocate_Memory(Heap->ParentAllocator, BlockSize+sizeof(heap_memory_block)+sizeof(heap_block));
    Memory_Clear(MemoryBlock, sizeof(heap_memory_block));

    MemoryBlock->Ptr = (u8*)(MemoryBlock+1);
    MemoryBlock->Size   = BlockSize+sizeof(heap_block);
    
    MemoryBlock->Next = Heap->FirstBlock;
    Heap->FirstBlock  = MemoryBlock;
    
    return Heap__Add_Free_Block_From_Memory_Block(Heap, MemoryBlock);
}

#ifdef DEBUG_BUILD
internal void Heap__Add_To_Allocated_List(heap* Heap, heap_block* Block) {
    if(Heap->AllocatedList) Heap->AllocatedList->PrevAllocated = Block;
    Block->NextAllocated = Heap->AllocatedList;
    Heap->AllocatedList = Block;
}

internal bool Heap__Is_Block_Allocated(heap* Heap, heap_block* Block)
{
    for(heap_block* TargetBlock = Heap->AllocatedList; TargetBlock; TargetBlock = TargetBlock->NextAllocated)
        if(Block == TargetBlock) return true;
    return false;
}

internal void Heap__Remove_Allocated_Block(heap* Heap, heap_block* Block) {
    if(Block->PrevAllocated) Block->PrevAllocated->NextAllocated = Block->NextAllocated;
    if(Block->NextAllocated) Block->NextAllocated->PrevAllocated = Block->PrevAllocated;
    if(Block == Heap->AllocatedList) Heap->AllocatedList = Heap->AllocatedList->NextAllocated;
    Block->PrevAllocated = Block->NextAllocated = NULL;
}


#define Heap__Verify_Check(Condition) \
do \
{ \
if(!(Condition)) \
return false; \
} while(0)

internal bool Heap__Tree_Verify(heap* Heap, heap_block_node* Parent, heap_block_node* Node, uint32_t BlackNodeCount, uint32_t LeafBlackNodeCount)
{
    heap_block_tree* Tree = &Heap->FreeBlockTree;
    if(Parent == NULL)
    {
        Heap__Verify_Check(Tree->Root == Node);
        Heap__Verify_Check(Node == NULL || Heap__Get_Color(Node) == HEAP__BLACK_NODE);
    }
    else
    {
        Heap__Verify_Check(Parent->LeftChild == Node || Parent->RightChild == Node);
    }
    
    if(Node)
    {
        Heap__Verify_Check(Heap__Get_Parent(Node) == Parent);
        if(Parent)
        {
            if(Parent->LeftChild == Node)
                Heap__Verify_Check(Parent->Block->Size >= Node->Block->Size);
            else
            {
                Heap__Verify_Check(Parent->RightChild == Node);
                Heap__Verify_Check(Parent->Block->Size <= Node->Block->Size);
            }
        }
        
        if(Heap__Get_Color(Node) == HEAP__RED_NODE)
        {
            if(Node->LeftChild)  Heap__Verify_Check(Heap__Get_Color(Node->LeftChild)  == HEAP__BLACK_NODE);
            if(Node->RightChild) Heap__Verify_Check(Heap__Get_Color(Node->RightChild) == HEAP__BLACK_NODE);
        }
        else
        {
            BlackNodeCount++;
        }
        
        if(!Node->LeftChild && !Node->RightChild)
            Heap__Verify_Check(BlackNodeCount == LeafBlackNodeCount);
        
        bool LeftResult  = Heap__Tree_Verify(Heap, Node, Node->LeftChild,  BlackNodeCount, LeafBlackNodeCount);
        bool RightResult = Heap__Tree_Verify(Heap, Node, Node->RightChild, BlackNodeCount, LeafBlackNodeCount);
        return LeftResult && RightResult;
    }
    
    return true;
}

internal bool Heap__Verify_Check_Offsets(heap* Heap)
{
    for(heap_memory_block* MemoryBlock = Heap->FirstBlock; MemoryBlock; MemoryBlock = MemoryBlock->Next)
    {
        heap_block* Block = MemoryBlock->FirstBlock;
        if(Block->Prev) { return false; }
        
        size_t Offset = 0;
        while(Block)
        {
            if(Block->Offset != Offset) { return false; }
            Offset += (sizeof(heap_block)+Block->Size);
            Block = Block->Next;
            if(!Block)  if(Offset != MemoryBlock->Size) { return false; }
        }
    }
    
    return true;
}

bool Heap_Verify(heap* Heap) {
    bool OffsetVerified = Heap__Verify_Check_Offsets(Heap);
    if(!OffsetVerified) return false;
    
    heap_block_tree* Tree = &Heap->FreeBlockTree;
    if(Tree->Root) Heap__Verify_Check(Heap__Get_Color(Tree->Root) == HEAP__BLACK_NODE);
    
    uint32_t LeafBlackNodeCount = 0;
    if(Tree->Root)
    {
        for(heap_block_node* Node = Tree->Root; Node; Node = Node->LeftChild)
        {
            if(Heap__Get_Color(Node) == HEAP__BLACK_NODE)
                LeafBlackNodeCount++;
        }
    }
    
    bool TreeVerified = Heap__Tree_Verify(Heap, NULL, Tree->Root, 0, LeafBlackNodeCount);
    return TreeVerified;
}

#else
bool Heap_Verify(heap* Heap) {
    return true;
}
#endif

heap* Heap_Create(allocator* ParentAllocator, uptr MinimumBlockSize) {
    heap* Result = Allocator_Allocate_Struct(ParentAllocator, heap);
    Memory_Clear(Result, sizeof(heap));

    Result->VTable = &G_HeapVTable;
    Result->ParentAllocator = ParentAllocator;
    Result->MinimumBlockSize = (MinimumBlockSize == (uptr)-1) ? KB(4) : Ceil_Pow2(MinimumBlockSize);
    Result->FreeBlockTree.NodeArena = Arena_Create(ParentAllocator, (uptr)-1);
    
    Heap_Clear(Result);
    return Result;
}

void Heap_Delete(heap* Heap) {
    Allocator_Delete__Tracker((allocator*)Heap);
    if(Heap->ParentAllocator) {
        allocator* Allocator = Heap->ParentAllocator;

        heap_memory_block* MemoryBlock = Heap->FirstBlock;
        while(MemoryBlock) {
            heap_memory_block* BlockToDelete = MemoryBlock;
            MemoryBlock = BlockToDelete->Next;
            Allocator_Free_Memory(Allocator, BlockToDelete);
        }

        Arena_Delete(Heap->FreeBlockTree.NodeArena);
        Allocator_Free_Memory(Allocator, Heap);
    }
}

void* Heap_Allocate(heap* Heap, uptr Size, allocator_clear_flag ClearFlag) {
//NOTE(EVERYONE): Allocating a block conceptually is a super simple process.
    //1. Find the best fitting block. 
    //2. Split it if the size requested is smaller than the block size
    //3. Get the pointer from the heap block
    
    if(!Size) return NULL;
    heap_block* Block = Heap__Find_Best_Fitting_Block(Heap, Size);
    if(!Block)
    {
        uptr BlockSize = Heap->LastBlockSize*2;

        if(BlockSize < Size) {
            BlockSize = Ceil_Pow2(Size);
        }

        Heap->LastBlockSize = BlockSize;

        BlockSize = Max(BlockSize, Heap->MinimumBlockSize);

        Block = Heap__Create_Memory_Block(Heap, BlockSize);
        if(!Block) return NULL;
    }
    
    Heap__Split_Block(Heap, Block, Size);
    
    //NOTE(EVERYONE): The block offset always starts at the beginning of the blocks metadata.
    //This is not where the returned memory starts. It always starts right after the metadata 
    //so we must offset an additional pointer size
    uptr Offset = Block->Offset + sizeof(heap_block);
    Assert(Offset + Size <= Block->Block->Size);
    void* Result = Block->Block->Ptr + Offset;
    
#ifdef DEBUG_BUILD
    Heap__Add_To_Allocated_List(Heap, Block);
    Assert(Heap_Verify(Heap));
#endif

    Heap->CurrentAllocatedMemory += Block->Size;

    if(ClearFlag == ALLOCATOR_CLEAR_FLAG_CLEAR) {
        Memory_Clear(Result, Size);
    }
        
    return Result;
}

void Heap_Free(heap* Heap, void* Memory) {
    if(Memory)
    {
        //NOTE(EVERYTONE): Freeing a block is a little more complex
        //1. Get the block's metadata from the memory address. Validate that its the correct block
        //2. Check to see if the blockers neighboring blocks are free. And do the following:
        //    -If no neighboring blocks are free, add the block to the free block tree
        //    -If the left block is free, merge the left and current block, and delete the current block
        //    -If the right block is free, merge the current and right block, and delete the right block
        //    -If both are free, merge left block with the current and right block, and delete the current and right block
        
        heap_block* Block = ((heap_block*)Memory)-1;        
        Assert((Block->Block->Ptr + Block->Offset + sizeof(heap_block)) == Memory);
        
#ifdef DEBUG_BUILD
        Assert(Heap__Is_Block_Allocated(Heap, Block));
        Heap__Remove_Allocated_Block(Heap, Block);
#endif

        Heap->CurrentAllocatedMemory -= Block->Size;
        
        heap_block* LeftBlock  = Block->Prev;
        heap_block* RightBlock = Block->Next;
        
        bool IsLeftBlockFree  = Heap__Is_Block_Free(LeftBlock);
        bool IsRightBlockFree = Heap__Is_Block_Free(RightBlock);
        
        if(!IsLeftBlockFree && !IsRightBlockFree)
        {
            Heap__Add_Free_Block(Heap, Block);
        }
        else if(IsLeftBlockFree && !IsRightBlockFree)
        {
            Assert(LeftBlock->Block == Block->Block);
            Assert((LeftBlock->Offset+LeftBlock->Size+sizeof(heap_block)) == Block->Offset);
            
            LeftBlock->Next = Block->Next;
            if(Block->Next) Block->Next->Prev = LeftBlock;
            
            Heap__Increase_Block_Size(Heap, LeftBlock, Block->Size+sizeof(heap_block));
            Heap__Delete_Heap_Block(Heap, Block);
        }
        else if(!IsLeftBlockFree && IsRightBlockFree)
        {
            Assert(RightBlock->Block == Block->Block);
            Assert((Block->Offset+Block->Size+sizeof(heap_block)) == RightBlock->Offset);
            
            Block->Next = RightBlock->Next;
            if(RightBlock->Next) RightBlock->Next->Prev = Block;
            
            Block->Size += RightBlock->Size+sizeof(heap_block);
            Heap__Delete_Heap_Block(Heap, RightBlock);
            Heap__Add_Free_Block(Heap, Block);
        }
        else
        {
            Assert(LeftBlock->Block  == Block->Block);
            Assert(RightBlock->Block == Block->Block);
            Assert((LeftBlock->Offset+LeftBlock->Size+sizeof(heap_block)) == Block->Offset);
            Assert((Block->Offset+Block->Size+sizeof(heap_block)) == RightBlock->Offset);
            
            size_t Addend = Block->Size+RightBlock->Size+sizeof(heap_block)*2;
            
            LeftBlock->Next = RightBlock->Next;
            if(RightBlock->Next) RightBlock->Next->Prev = LeftBlock;
            Heap__Increase_Block_Size(Heap, LeftBlock, Addend);
            Heap__Delete_Heap_Block(Heap, Block);
            Heap__Delete_Heap_Block(Heap, RightBlock);
        }
        
#ifdef DEBUG_BUILD
        Assert(Heap_Verify(Heap));
#endif
    }
}

internal void Heap_Block_Tree__Reset(heap_block_tree* Tree) {
    Arena_Clear(Tree->NodeArena);
    Tree->FreeNodes = NULL;
    Tree->Root = NULL;
}

void Heap_Clear(heap* Heap) {
    Heap_Block_Tree__Reset(&Heap->FreeBlockTree);
    for(heap_memory_block* Block = Heap->FirstBlock; Block; Block = Block->Next) {        
#ifdef DEBUG_BUILD
        Block->FirstBlock = NULL;
#endif

        Heap__Add_Free_Block_From_Memory_Block(Heap, Block);
    }
    Heap->CurrentAllocatedMemory = 0;
    Heap->LastBlockSize = 0;
    
#ifdef DEBUG_BUILD
    Heap->AllocatedList = NULL;
#endif
}

void Heap_Track(heap* Heap, string DebugName) {
    Allocator_Create__Tracker((allocator*)Heap, DebugName, ALLOCATOR_TYPE_HEAP);
    
    scratch Scratch = Scratch_Get();
    string NodeArenaName(&Scratch, "%.*s.NodeArena", DebugName.Size, DebugName.Str);
    Arena_Track(Heap->FreeBlockTree.NodeArena, NodeArenaName);
}

internal ALLOCATOR_ALLOCATE_MEMORY_DEFINE(Lock_Allocator_Allocate_Memory) {
    return Lock_Allocator_Allocate((lock_allocator*)Allocator, Size);
}

internal ALLOCATOR_FREE_MEMORY_DEFINE(Lock_Allocator_Free_Memory) {
    Lock_Allocator_Free((lock_allocator*)Allocator, Memory);
}

internal ALLOCATOR_GET_STATS_DEFINE(Lock_Allocator_Get_Stats) {
    return Allocator_Get_Stats(Allocator->ParentAllocator);
}

global allocator_vtable G_LockAllocatorVTable = {
    Lock_Allocator_Allocate_Memory,
    Lock_Allocator_Free_Memory,
    Lock_Allocator_Get_Stats
};

lock_allocator* Lock_Allocator_Create(allocator* ParentAllocator) {
    lock_allocator* Result = Allocator_Allocate_Struct(ParentAllocator, lock_allocator);
    Memory_Clear(Result, sizeof(lock_allocator));
    Result->VTable = &G_LockAllocatorVTable;
    Result->ParentAllocator = ParentAllocator;
    AK_Mutex_Create(&Result->Lock);
    return Result;
}

void Lock_Allocator_Delete(lock_allocator* LockAllocator) {
    Allocator_Delete__Tracker((allocator*)LockAllocator);
    if(LockAllocator->ParentAllocator) {
        allocator* Allocator = LockAllocator->ParentAllocator;
        AK_Mutex_Delete(&LockAllocator->Lock);
        Allocator_Free_Memory(Allocator, LockAllocator);
    }    
}

void* Lock_Allocator_Allocate(lock_allocator* LockAllocator, uptr Size, allocator_clear_flag ClearFlag) {
    AK_Mutex_Lock(&LockAllocator->Lock);
    void* Result = Allocator_Allocate_Memory_Internal(LockAllocator->ParentAllocator, Size);
    AK_Mutex_Unlock(&LockAllocator->Lock);
    
    if(ClearFlag == ALLOCATOR_CLEAR_FLAG_CLEAR) {
        Memory_Clear(Result, Size);
    }

    return Result;
}

void Lock_Allocator_Free(lock_allocator* LockAllocator, void* Memory) {
    AK_Mutex_Lock(&LockAllocator->Lock);
    Allocator_Free_Memory_Internal(LockAllocator->ParentAllocator, Memory);
    AK_Mutex_Unlock(&LockAllocator->Lock);
}

void Lock_Allocator_Track(lock_allocator* LockAllocator, string DebugName) {
    Allocator_Create__Tracker((allocator*)LockAllocator, DebugName, ALLOCATOR_TYPE_LOCK);
}

void* operator new(uptr Size) {
    return Allocator_Allocate_Memory(Core_Get_Base_Allocator(), Size);
}

void operator delete(void* Memory) noexcept {
	Allocator_Free_Memory(Core_Get_Base_Allocator(), Memory);
}

void* operator new[](uptr Size) {
	return Allocator_Allocate_Memory(Core_Get_Base_Allocator(), Size);
}

void operator delete[](void* Memory) noexcept {
	Allocator_Free_Memory(Core_Get_Base_Allocator(), Memory);
}

void* operator new(uptr Size, allocator* Allocator) noexcept {
    return Allocator_Allocate_Memory(Allocator, Size);
}

void  operator delete(void* Memory, allocator* Allocator) noexcept {
	Allocator_Free_Memory(Allocator, Memory);
}

void* operator new[](uptr Size, allocator* Allocator) noexcept {
    return Allocator_Allocate_Memory(Allocator, Size);
}

void operator delete[](void* Memory, allocator* Allocator) noexcept {
	Allocator_Free_Memory(Allocator, Memory);
}

inline memory_stream Memory_Stream_Begin(void* Start, void* End) {
    return {
        .Start = (u8*)Start,
        .End = (u8*)End,
        .At = (u8*)Start
    };
}

inline bool Memory_Stream_Is_Valid(memory_stream* Stream) {
    return Stream->At < Stream->End;
}

inline const void* Memory_Stream_Current(memory_stream* Stream) {
    return Stream->At;
}

inline void Memory_Stream_Skip(memory_stream* Stream, uptr Size) {
    Stream->At += Size;
    Assert(Stream->At <= Stream->End);
}