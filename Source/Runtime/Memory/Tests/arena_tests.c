
typedef struct ArenaTest
{
    custom_allocator* Allocator;
    arena* Arena;
} ArenaTest;

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

UTEST_F_SETUP(ArenaTest)
{
    custom_allocator* Allocator = (custom_allocator*)malloc(sizeof(custom_allocator));
    Zero_Struct(Allocator, custom_allocator);
    utest_fixture->Allocator = Allocator;
    Allocator->Allocator.Allocate = Allocate_Memory;
    Allocator->Allocator.Free = Free_Memory;
    
    arena* Arena = Arena_Create(&utest_fixture->Allocator->Allocator, Kilo(512));
    utest_fixture->Arena = Arena;
    
    ASSERT_NE(Arena, NULL);
    ASSERT_EQ(Allocator->NumberOfAllocations, 1);
    ASSERT_EQ(Allocator->NumberOfFrees, 0);
    ASSERT_EQ(Allocator->NumberOfActiveAllocations, 1);
    ASSERT_EQ(Arena->InitialBlockSize, Kilo(512));
}

UTEST_F(ArenaTest, Allocate)
{
    arena* Arena = utest_fixture->Arena;
    
    void* Memory0 = Arena_Push(Arena, Kilo(312), MEMORY_NO_CLEAR);
    ASSERT_EQ(Arena->FirstBlock, Arena->CurrentBlock);
    ASSERT_EQ(Arena->CurrentBlock->Used, Kilo(312));
    
    void* Memory1 = Arena_Push(Arena, Kilo(100), MEMORY_CLEAR);
    ASSERT_EQ(Arena->CurrentBlock->Used, Kilo(412));
    
    uint8_t* StartStream = (uint8_t*)Memory1;
    for(uint64_t Index = 0; Index < Kilo(100); Index++)
    {
        ASSERT_EQ(StartStream[Index], 0);
    }
    
    arena_marker Marker = Arena_Get_Marker(Arena);
    ASSERT_EQ(Marker.Marker, Kilo(412));
    ASSERT_EQ(Marker.Block, Arena->CurrentBlock);
    
    Arena_Push(Arena, Kilo(200), MEMORY_NO_CLEAR);
    ASSERT_NE(Arena->FirstBlock, Arena->CurrentBlock);
    ASSERT_EQ(Arena->CurrentBlock->Used, Kilo(200));
    
    Arena_Push(Arena, Kilo(1000), MEMORY_NO_CLEAR);
    ASSERT_EQ(Arena->CurrentBlock->Used, Arena->CurrentBlock->Size);
    
    Arena_Set_Marker(Arena, &Marker);
    
    ASSERT_EQ(Arena->FirstBlock, Arena->CurrentBlock);
    ASSERT_EQ(Arena->CurrentBlock->Used, Kilo(412));
    ASSERT_EQ(Arena->CurrentBlock->Next->Used, 0ULL);
    ASSERT_EQ(Arena->CurrentBlock->Next->Next->Used, 0ULL);
    
    Arena_Push(Arena, Kilo(200), MEMORY_NO_CLEAR);
    ASSERT_EQ(Arena->FirstBlock->Next, Arena->CurrentBlock);
    ASSERT_EQ(Arena->CurrentBlock->Used, Kilo(200));
    
    Arena_Set_Marker(Arena, &Marker);
    ASSERT_EQ(Arena->FirstBlock, Arena->CurrentBlock);
    ASSERT_EQ(Arena->CurrentBlock->Used, Kilo(412));
    
    Arena_Push(Arena, Kilo(3000), MEMORY_NO_CLEAR);
    ASSERT_EQ(Arena->CurrentBlock->Next, NULL);
    ASSERT_EQ(Arena->CurrentBlock->Used, Kilo(3000));
    ASSERT_EQ(Arena->FirstBlock->Next->Next->Next, Arena->CurrentBlock);
    
    Arena_Clear(Arena, MEMORY_CLEAR);
    ASSERT_EQ(Arena->FirstBlock, Arena->CurrentBlock);
    for(arena_block* Block = Arena->FirstBlock; Block; Block = Block->Next)
    {
        ASSERT_EQ(Block->Used, 0ULL);
        uint8_t* Memory = Block->Memory;
        for(uint64_t Index = 0; Index < Block->Size; Index++)
            ASSERT_EQ(Memory[Index], 0);
    }
}

UTEST_F_TEARDOWN(ArenaTest)
{
    arena* Arena = utest_fixture->Arena;
    custom_allocator* Allocator = utest_fixture->Allocator;
    
    Arena_Delete(Arena);
    ASSERT_EQ(Allocator->NumberOfAllocations, Allocator->NumberOfFrees);
    ASSERT_EQ(Allocator->NumberOfActiveAllocations, 0);
    ASSERT_EQ(Allocator->NumberOfAllocatedBytes, Allocator->NumberOfFreedBytes);
    ASSERT_EQ(Allocator->NumberOfActiveBytes, 0);
    
    free(Allocator);
}
