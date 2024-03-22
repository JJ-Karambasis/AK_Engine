#ifndef MEMORY_H
#define MEMORY_H

struct buffer {
    u8*  Ptr  = nullptr;
    uptr Size = 0;

    buffer() = default;
    buffer(void* Data, uptr BufferSize);
    buffer(allocator* Allocator, uptr BufferSize);
};

struct const_buffer {
    const u8* Ptr  = nullptr;
    uptr      Size = 0;

    const_buffer() = default;
    const_buffer(const void* Data, uptr BufferSize);
    const_buffer(const buffer& Buffer);

    template <typename type>
    inline const_buffer(const type* Entry) : Ptr((const u8*)Entry), Size(sizeof(type)) { }

    template <typename type, uptr N>
    inline const_buffer(const type (&Array)[N]) : Ptr((const u8*)Array), Size(sizeof(type)*N) { }
};

void* Memory_Copy(void* Dst, const void* Src, uptr Size);
void* Memory_Clear(void* Memory, uptr Size);
void* Memory_Set(void* Memory, u8 Value, uptr Size);

template <typename type>
inline type* Zero_Struct(type* Ptr) {
    return (type*)Memory_Clear(Ptr, sizeof(type));
}

template <typename type>
inline type* Zero_Array(type* Ptr, uptr Count) {
    return (type*)Memory_Clear(Ptr, sizeof(type)*Count);
}

template <typename type>
inline type* Array_Copy(type* Dst, const type* Src, uptr Count) {
    return (type*)Memory_Copy(Dst, Src, sizeof(type)*Count);
}

enum allocator_clear_flag {
    ALLOCATOR_CLEAR_FLAG_CLEAR,
    ALLOCATOR_CLEAR_FLAG_NO_CLEAR
};

#define DEFAULT_CLEAR_FLAG ALLOCATOR_CLEAR_FLAG_CLEAR

enum allocator_type {
	ALLOCATOR_TYPE_VIRTUAL,
	ALLOCATOR_TYPE_HEAP,
	ALLOCATOR_TYPE_ARENA,
	ALLOCATOR_TYPE_LOCK
};

struct allocator_tracker {
	allocator_type Type;
	string         Name;
	allocator*     Allocator;
	allocator_tracker* FirstChild;
	allocator_tracker* LastChild;
	allocator_tracker* NextSibling;
	allocator_tracker* PrevSibling;
};

struct allocator_stats {
	uptr TotalReservedMemory;
	uptr TotalUsedMemory;
};

#define ALLOCATOR_ALLOCATE_MEMORY_DEFINE(name) void* 		   name(allocator* Allocator, uptr Size)
#define ALLOCATOR_FREE_MEMORY_DEFINE(name) 	   void  		   name(allocator* Allocator, void* Memory)
#define ALLOCATOR_GET_STATS_DEFINE(name)       allocator_stats name(allocator* Allocator)

typedef ALLOCATOR_ALLOCATE_MEMORY_DEFINE(allocator_allocate_memory_func);
typedef ALLOCATOR_FREE_MEMORY_DEFINE(allocator_free_memory_func);
typedef ALLOCATOR_GET_STATS_DEFINE(allocator_get_stats_func);

struct allocator_vtable {
	allocator_allocate_memory_func* Allocate_Memory;
	allocator_free_memory_func*     Free_Memory;
	allocator_get_stats_func*       Get_Stats;
};

struct allocator {
	allocator_vtable*  VTable;
	allocator* 		   ParentAllocator;
	allocator_tracker* Tracker;
};

#define Allocator_Allocate_Memory_Internal(Allocator, ...) (Allocator)->VTable->Allocate_Memory(Allocator, __VA_ARGS__)
#define Allocator_Free_Memory_Internal(Allocator, ...) 	  (Allocator)->VTable->Free_Memory(Allocator, __VA_ARGS__)
#define Allocator_Get_Stats(Allocator) 			  (Allocator)->VTable->Get_Stats(Allocator)

void* Allocator_Allocate_Memory(allocator* Allocator, uptr Size, allocator_clear_flag ClearFlag = DEFAULT_CLEAR_FLAG);
void* Allocator_Allocate_Aligned_Memory(allocator* Allocator, uptr Size, uptr Alignment, allocator_clear_flag ClearFlag = DEFAULT_CLEAR_FLAG);
void  Allocator_Free_Memory(allocator* Allocator, void* Memory);

#define Allocator_Allocate_Struct(allocator, type) (type*)Allocator_Allocate_Aligned_Memory(allocator, sizeof(type), alignof(type))
#define Allocator_Allocate_Array(allocator, count, type) (type*)Allocator_Allocate_Aligned_Memory(allocator, sizeof(type)*count, alignof(type))

struct virtual_allocator : public allocator {
	ak_atomic_u64 TotalReservedMemory;
	ak_atomic_u64 TotalCommittedMemory;
};

virtual_allocator* Virtual_Allocator_Create();
void               Virtual_Allocator_Delete(virtual_allocator* VirtualAllocator);
void* 			   Virtual_Allocator_Reserve(virtual_allocator* VirtualAllocator, uptr Size);
void* 			   Virtual_Allocator_Commit(virtual_allocator*  VirtualAllocator, void* Memory, uptr Size);
void  			   Virtual_Allocator_Decommit(virtual_allocator* VirtualAllocator, void* Memory, uptr Size);
void  			   Virtual_Allocator_Release(virtual_allocator*  VirtualAllocator, void* Memory, uptr Size);
void               Virtual_Allocator_Track(virtual_allocator* VirtualAllocator, string DebugName);

typedef struct arena_block arena_block;
typedef struct arena arena;

struct arena_block {
	u8*  		 Ptr;
	uptr 		 Size;
	uptr 		 Used;
	arena_block* Next;
};

typedef struct {
	arena* 		 Arena;
	arena_block* CurrentBlock;
	uptr 		 Marker;
} arena_marker;

struct arena : public allocator {
	uptr         MinimumBlockSize;
	arena_block* FirstBlock;
	arena_block* LastBlock;
	arena_block* CurrentBlock;
};

arena* 		 Arena_Create(allocator* ParentAllocator, uptr MinimumBlockSize = (uptr)-1);
void 		 Arena_Delete(arena* Arena);
void*  		 Arena_Push(arena* Arena, uptr Size, uptr Alignment = DEFAULT_ALIGNMENT, allocator_clear_flag ClearFlag = DEFAULT_CLEAR_FLAG);
arena_marker Arena_Get_Marker(arena* Arena);
void 		 Arena_Set_Marker(arena* Arena, arena_marker* Marker);
void 		 Arena_Clear(arena* Arena);
void         Arena_Track(arena* Arena, string DebugName);

#define Arena_Push_Struct(Arena, type) (type*)Arena_Push(Arena, sizeof(type), alignof(type))
#define Arena_Push_Array(Arena, count, type) (type*)Arena_Push(Arena, sizeof(type)*count, alignof(type))

struct heap_block;
struct heap_block_node;
struct heap_memory_block;

struct heap_memory_block {
	u8*  			   Ptr;
	uptr 			   Size;
	heap_memory_block* Next;
#ifdef DEBUG_BUILD
	heap_block* FirstBlock;
#endif
};

struct heap_block {
	heap_memory_block* Block;
	uptr               Offset;
	uptr               Size;
	heap_block_node*   Node;
	heap_block*        Prev;
	heap_block*        Next;

#ifdef DEBUG_BUILD
	heap_block* NextAllocated;
	heap_block* PrevAllocated;
#endif
};

struct heap_block_node {
	heap_block*      Block;
	uptr             Color;
	heap_block_node* LeftChild;
	heap_block_node* RightChild;
};

struct heap_block_tree {
	arena*           NodeArena;
	heap_block_node* Root;
	heap_block_node* FreeNodes;
};

struct heap : public allocator {
	heap_memory_block* FirstBlock;
    heap_block_tree    FreeBlockTree;
	uptr               MinimumBlockSize;
	uptr               LastBlockSize;
	uptr               CurrentAllocatedMemory;

#ifdef DEBUG_BUILD
	heap_block* AllocatedList;
#endif

};

heap* Heap_Create(allocator* ParentAllocator, uptr MinimumBlockSize = (uptr)-1);
void  Heap_Delete(heap* Heap);
void* Heap_Allocate(heap* Heap, uptr Size, allocator_clear_flag ClearFlag = DEFAULT_CLEAR_FLAG);
void  Heap_Free(heap* Heap, void* Memory);
void  Heap_Clear(heap* Heap);
void  Heap_Track(heap* Heap, string DebugName);
bool  Heap_Verify(heap* Heap);

struct lock_allocator : public allocator {
	ak_mutex  Lock;
};

lock_allocator* Lock_Allocator_Create(allocator* ParentAllocator);
void            Lock_Allocator_Delete(lock_allocator* LockAllocator);
void*           Lock_Allocator_Allocate(lock_allocator* LockAllocator, uptr Size, allocator_clear_flag ClearFlag = DEFAULT_CLEAR_FLAG);
void            Lock_Allocator_Free(lock_allocator* LockAllocator, void* Memory);
void            Lock_Allocator_Track(lock_allocator* LockAllocator, string DebugName);

void* operator new(uptr Size);
void  operator delete(void* Memory) noexcept;
void* operator new[](uptr Size);
void  operator delete[](void* Memory) noexcept;
void* operator new(uptr Size, allocator* Allocator) noexcept;
void  operator delete(void* Memory, allocator* Allocator) noexcept;
void* operator new[](uptr Size, allocator* Allocator) noexcept;
void  operator delete[](void* Memory, allocator* Allocator) noexcept;

#endif