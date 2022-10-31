#ifndef WIN32_ALLOCATOR_H
#define WIN32_ALLOCATOR_H

typedef struct win32_allocator
{
    allocator Allocator;
    volatile  int64_t NumberOfAllocations;
    volatile  int64_t NumberOfFrees;
    volatile  int64_t NumberOfActiveAllocations;
    volatile  int64_t NumberOfAllocatedBytes;
    volatile  int64_t NumberOfFreedBytes;
    volatile  int64_t NumberOfActiveBytes;
} win32_allocator;

void Win32_Get_Main_Allocator(win32_allocator* Allocator);

#endif