void Memory_Set(void* Memory, uint8_t Value, size_t Size)
{
    memset(Memory, Value, Size);
}

void Memory_Clear(void* Memory, size_t Size)
{
    Memory_Set(Memory, 0, Size);
}

void Memory_Copy(void* DstMemory, const void* SrcMemory, size_t Size)
{
    memcpy(DstMemory, SrcMemory, Size);
}

#include "Private/allocator.c"
#include "Private/arena.c"
#include "Private/heap.c"