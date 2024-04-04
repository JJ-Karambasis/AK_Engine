#ifndef GDI_MEMORY_H
#define GDI_MEMORY_H

struct gdi_heap;
struct gdi_heap_block;
struct gdi_heap_block_node;
struct gdi_heap_memory_block;

#define GDI_HEAP_ALLOCATE_MEMORY_BLOCK_DEFINE(name) gdi_heap_memory_block* name(gdi_heap* Heap, uptr Size)
#define GDI_HEAP_FREE_MEMORY_BLOCK_DEFINE(name) void name(gdi_heap* Heap, gdi_heap_memory_block* MemoryBlock)

typedef GDI_HEAP_ALLOCATE_MEMORY_BLOCK_DEFINE(gdi_heap_allocate_memory_block_func);
typedef GDI_HEAP_FREE_MEMORY_BLOCK_DEFINE(gdi_heap_free_memory_block_func);

struct gdi_heap_memory_block {
    uptr                   Size;
    gdi_heap_memory_block* Next;
#ifdef DEBUG_BUILD
    gdi_heap_block* FirstBlock; 
#endif
};

struct gdi_heap_block {
    gdi_heap_memory_block* Block;
    uptr                   Offset;
    uptr                   Size;
    gdi_heap_block_node*   Node;
    gdi_heap_block*        Prev;
    gdi_heap_block*        Next;

#ifdef DEBUG_BUILD
    gdi_heap_block* NextAllocated;
    gdi_heap_block* PrevAllocated;
#endif
};

struct gdi_heap_block_node {
    gdi_heap_block*      Block;
    uptr                 Color;
    gdi_heap_block_node* LeftChild;
    gdi_heap_block_node* RightChild;
};

struct gdi_heap_block_tree {
    arena*               NodeArena;
    gdi_heap_block_node* Root;
    gdi_heap_block_node* FreeNodes;
};

struct gdi_heap_vtable {
    gdi_heap_allocate_memory_block_func* AllocateMemoryBlock;
    gdi_heap_free_memory_block_func*     FreeMemoryBlock;
};

struct gdi_heap {
    gdi_heap_vtable*       VTable;
    ak_mutex               Mutex;
    arena*                 Arena;
    gdi_heap_memory_block* FirstBlock;
    gdi_heap_block_tree    FreeBlockTree;
    gdi_heap_block*        OrphanBlocks;
    uptr                   InitialBlockSize;

#ifdef DEBUG_BUILD
    gdi_heap_block* AllocatedList;
#endif
};

struct gdi_allocate {
    gdi_heap_block* Block;
    uptr            Offset;
};

#define GDI_Heap_Allocate_Memory_Block(Heap, ...) (Heap)->VTable->AllocateMemoryBlock(Heap, __VA_ARGS__)
#define GDI_Heap_Free_Memory_Block(Heap, ...) (Heap)->VTable->FreeMemoryBlock(Heap, __VA_ARGS__)

void         GDI_Heap_Create(gdi_heap* Heap, allocator* Allocator, uptr InitialBlockSize, gdi_heap_vtable* VTable);
void         GDI_Heap_Delete(gdi_heap* Heap);
gdi_allocate GDI_Heap_Allocate(gdi_heap* Heap, uptr Size, uptr Alignment);
void         GDI_Heap_Free(gdi_heap* Heap, gdi_allocate* Allocate);


#endif