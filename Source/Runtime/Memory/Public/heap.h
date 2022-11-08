#ifndef HEAP_H
#define HEAP_H

typedef struct heap_block heap_block;
typedef struct heap_block_node heap_block_node;

typedef struct heap_memory_block
{
    uint8_t*                  Memory;
    size_t                    Size;
    struct heap_memory_block* Next;
    
#ifdef DEBUG_BUILD
    heap_block* FirstBlock;
#endif
} heap_memory_block;

typedef struct heap_block
{
    heap_memory_block* Block;
    size_t             Offset;
    size_t             Size;
    heap_block_node*   Node;
    heap_block*        Prev;
    heap_block*        Next;
    
#ifdef DEBUG_BUILD
    heap_block* NextAllocated;
    heap_block* PreviousAllocated;
#endif
} heap_block;

typedef struct heap_block_node
{
    heap_block*             Block;
    size_t                  Color;
    struct heap_block_node* LeftChild;
    struct heap_block_node* RightChild;
} heap_block_node;

typedef struct heap_block_tree
{
    heap_block_node* Root;
    heap_block_node* FreeNodes;
} heap_block_tree;

typedef struct heap
{
    allocator          BaseAllocator;
    arena*             Arena;
    heap_memory_block* FirstBlock;
    size_t             InitialBlockSize;
    heap_block_tree    FreeBlockTree;
    heap_block*        OrphanBlocks;
    
#ifdef DEBUG_BUILD
    heap_block* AllocatedList;
#endif
} heap;

heap*   Heap_Create(allocator* Allocator, size_t InitialBlockSize);
void    Heap_Delete(heap* Heap);
void*   Heap_Allocate(heap* Heap, size_t Size, memory_clear_flag ClearFlag);
void    Heap_Free(heap* Heap, void* Memory);
void    Heap_Clear(heap* Heap, memory_clear_flag ClearFlag);

#ifdef DEBUG_BUILD
bool8_t Heap__Verify(heap* Heap);
#define Heap_Verify(Heap) Heap__Verify(Heap)
#else
#define Heap_Verify(Heap)
#endif

#define Heap_Allocate_Struct(Heap, Type) (Type*)Heap_Allocate(Heap, sizeof(Type), MEMORY_CLEAR)
#define Heap_Allocate_Array(Heap, Type, Count) (Type*)Heap_Allocate(Heap, sizeof(Type)*Count, MEMORY_CLEAR)

#endif