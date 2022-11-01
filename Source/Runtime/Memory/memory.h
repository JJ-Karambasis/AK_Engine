#ifndef MEMORY_H
#define MEMORY_H

#include "Public/allocator.h"
#include "Public/arena.h"
#include "Public/heap.h"

void Memory_Set(void* Memory, uint8_t Value, size_t Size);
void Memory_Clear(void* Memory, size_t Size);
void Memory_Copy(void* DstMemory, const void* SrcMemory, size_t Size);

#define Zero_Struct(Memory, Type) Memory_Clear(Memory, sizeof(Type))

#endif