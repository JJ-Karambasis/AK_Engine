#ifndef ARENA_H
#define ARENA_H

typedef struct arena_block
{
    uint8_t* Memory;
    size_t   Used;
    size_t   Size;
    struct arena_block* Next;
} arena_block;

typedef struct arena arena;

typedef struct arena_marker
{
    arena*       Arena;
    arena_block* Block;
    size_t       Marker;
} arena_marker;

typedef struct arena
{
    allocator    BaseAllocator;
    allocator*   Allocator;
    arena_block* FirstBlock;
    arena_block* LastBlock;
    arena_block* CurrentBlock;
    size_t       InitialBlockSize;
} arena;

arena*       Arena_Create(allocator* Allocator, size_t InitialBlockSize);
void         Arena_Delete(arena* Arena);
void*        Arena_Push(arena* Arena, size_t Size, memory_clear_flag ClearFlag);
arena_marker Arena_Get_Marker(arena* Arena);
void         Arena_Set_Marker(arena* Arena, arena_marker* Marker);
void         Arena_Clear(arena* Arena, memory_clear_flag ClearFlag);

#define Arena_Push_Struct(Arena, Type) (Type*)Arena_Push(Arena, sizeof(Type), MEMORY_CLEAR)
#define Arena_Push_Array(Arena, Type, Count) (Type*)Arena_Push(Arena, sizeof(Type)*Count, MEMORY_CLEAR)

#endif
