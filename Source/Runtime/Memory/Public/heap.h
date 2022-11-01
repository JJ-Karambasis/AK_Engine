#ifndef HEAP_H
#define HEAP_H

typedef struct heap_memory_block
{
    uint8_t*                  Memory;
    size_t                    Size;
    struct heap_memory_block* Next;
} heap_memory_block;

typedef struct heap_block_value
{
    heap_memory_block*      Block;
    size_t                  Offset;
    size_t                  Size;
} heap_block_value;

typedef struct heap_block
{
    heap_block_value   Value;
    size_t             Color;
    struct heap_block* LeftChild;
    struct heap_block* RightChild;
} heap_block;

typedef struct heap_block_tree
{
    heap_block* Root;
    heap_block* OrphanBlocks;
} heap_block_tree;

typedef struct heap
{
    allocator          BaseAllocator;
    arena*             Arena;
    heap_memory_block* FirstBlock;
    size_t             InitialBlockSize;
    
    hashmap*           OffsetMap;
    heap_block_tree    FreeBlockTree;
} heap;

heap* Heap_Create(allocator* Allocator, size_t InitialBlockSize);
void  Heap_Delete(heap* Heap);
void* Heap_Allocate(heap* Heap, size_t Size, memory_clear_flag ClearFlag);
void  Heap_Free(heap* Heap, void* Memory);
void  Heap_Clear(heap* Heap, memory_clear_flag ClearFlag);

#define Heap_Allocate_Struct(Heap, Type) (Type*)Heap_Allocate(Heap, sizeof(Type), MEMORY_CLEAR)
#define Heap_Allocate_Array(Heap, Type, Count) (Type*)Heap_Allocate(Heap, sizeof(Type)*Count, MEMORY_CLEAR)

#endif
