typedef struct custom_allocator
{
    allocator BaseAllocator;
    volatile  int64_t NumberOfAllocations;
    volatile  int64_t NumberOfFrees;
    volatile  int64_t NumberOfActiveAllocations;
    volatile  int64_t NumberOfAllocatedBytes;
    volatile  int64_t NumberOfFreedBytes;
    volatile  int64_t NumberOfActiveBytes;
} custom_allocator;

ALLOCATOR_ALLOCATE(Allocate_Memory)
{
    custom_allocator* CustomAllocator = (custom_allocator*)Allocator;
    
    void* Result = malloc(Size+sizeof(size_t));
    if(Result)
    {
        size_t* SizePtr = (size_t*)Result;
        *SizePtr = Size;
        
        Atomic_Increment64(&CustomAllocator->NumberOfAllocations);
        Atomic_Increment64(&CustomAllocator->NumberOfActiveAllocations);
        Atomic_Add64(&CustomAllocator->NumberOfAllocatedBytes, Size);
        Atomic_Add64(&CustomAllocator->NumberOfActiveBytes, Size);
        
        return SizePtr+1;
    }
    return NULL;
}

ALLOCATOR_FREE(Free_Memory)
{
    custom_allocator* CustomAllocator = (custom_allocator*)Allocator;
    size_t* SizePtr = (size_t*)Memory;
    int64_t Size = (int64_t)SizePtr[-1];
    free(&SizePtr[-1]);
    
    Atomic_Increment64(&CustomAllocator->NumberOfFrees);
    Atomic_Decrement64(&CustomAllocator->NumberOfActiveAllocations);
    Atomic_Add64(&CustomAllocator->NumberOfFreedBytes, Size);
    Atomic_Add64(&CustomAllocator->NumberOfActiveBytes, -Size);
}

custom_allocator* Allocate_Custom_Allocator()
{
    custom_allocator* Allocator = (custom_allocator*)malloc(sizeof(custom_allocator));
    Zero_Struct(Allocator, custom_allocator);
    Allocator->BaseAllocator.Allocate = Allocate_Memory;
    Allocator->BaseAllocator.Free = Free_Memory;
    return Allocator;
}

void Test_Custom_Allocator_Is_Not_Leaking(int* utest_result, custom_allocator* Allocator)
{
    ASSERT_EQ(Allocator->NumberOfAllocations, Allocator->NumberOfFrees);
    ASSERT_EQ(Allocator->NumberOfActiveAllocations, 0);
    ASSERT_EQ(Allocator->NumberOfAllocatedBytes, Allocator->NumberOfFreedBytes);
    ASSERT_EQ(Allocator->NumberOfActiveBytes, 0);
}

void Free_Custom_Allocator(custom_allocator* Allocator)
{
    free(Allocator);
}

#include "Tests/heap_tests.c"
#include "Tests/arena_tests.c"