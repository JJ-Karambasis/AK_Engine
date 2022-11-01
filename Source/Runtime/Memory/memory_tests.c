typedef struct custom_allocator
{
    allocator Allocator;
    volatile  int64_t NumberOfAllocations;
    volatile  int64_t NumberOfFrees;
    volatile  int64_t NumberOfActiveAllocations;
    volatile  int64_t NumberOfAllocatedBytes;
    volatile  int64_t NumberOfFreedBytes;
    volatile  int64_t NumberOfActiveBytes;
} custom_allocator;

#include "Tests/arena_tests.c"
#include "memory.c"