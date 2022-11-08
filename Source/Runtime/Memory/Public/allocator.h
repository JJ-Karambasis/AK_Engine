#ifndef ALLOCATOR_H
#define ALLOCATOR_H

typedef struct allocator allocator;

#define ALLOCATOR_ALLOCATE(name) void* name(allocator* Allocator, size_t Size)
typedef ALLOCATOR_ALLOCATE(allocator_allocate);

#define ALLOCATOR_FREE(name) void name(allocator* Allocator, void* Memory)
typedef ALLOCATOR_FREE(allocator_free);

typedef enum memory_clear_flag
{
    MEMORY_CLEAR,
    MEMORY_NO_CLEAR
} memory_clear_flag;

struct allocator
{
    allocator_allocate* Allocate;
    allocator_free*     Free;
    void*               UserData;
};

void* Allocate(allocator* Allocator, size_t Size, memory_clear_flag ClearFlag);
void  Free(allocator* Allocator, void* Memory);

void* Allocate_Aligned(allocator* Allocator, size_t Size, size_t Alignment, memory_clear_flag ClearFlag);
void  Free_Aligned(allocator* Allocator, void* Memory);

#define Allocate_Struct(Allocator, Type) Allocate_Aligned(Allocator, sizeof(Type), alignof(Type), MEMORY_CLEAR)
#define Allocate_Array(Allocator, Type, Count) Allocate_Aligned(Allocator, sizeof(Type)*Count, alignof(Type), MEMORY_CLEAR)
#define Allocate_Struct_No_Clear(Allocator, Type) Allocate_Aligned(Allocator, sizeof(Type), alignof(Type), MEMORY_NO_CLEAR)
#define Allocate_Array_No_Clear(Allocator, Type, Count) Allocate_Aligned(Allocator, sizeof(Type)*Count, alignof(Type), MEMORY_NO_CLEAR)
#define Free_Struct(Allocator, Memory) Free_Aligned(Allocator, Memory)
#define Free_Array(Allocator, Memory)  Free_Aligned(Allocator, Memory)

#define Get_Base_Allocator(Allocator) &(Allocator)->BaseAllocator

#endif