#include <Random/random.h>

typedef struct HeapTest
{
    custom_allocator* Allocator;
    heap* Heap;
} HeapTest;

UTEST_F_SETUP(HeapTest)
{
    custom_allocator* Allocator = Allocate_Custom_Allocator();
    utest_fixture->Allocator = Allocator;
    
    heap* Heap = Heap_Create(Get_Base_Allocator(utest_fixture->Allocator), Kilo(512));
    utest_fixture->Heap = Heap;
    
    ASSERT_TRUE(Heap_Verify(Heap));
}

UTEST_F(HeapTest, Test_Merge_Boundaries)
{
    heap* Heap = utest_fixture->Heap;
    
    void* Memory1 = Heap_Allocate(Heap, Kilo(10), MEMORY_NO_CLEAR);
    void* Memory2 = Heap_Allocate(Heap, Kilo(11), MEMORY_NO_CLEAR);
    void* Memory3 = Heap_Allocate(Heap, Kilo(12), MEMORY_NO_CLEAR);
    void* Memory4 = Heap_Allocate(Heap, Kilo(13), MEMORY_NO_CLEAR);
    void* Memory5 = Heap_Allocate(Heap, Kilo(9), MEMORY_NO_CLEAR);
    void* Memory6 = Heap_Allocate(Heap, Kilo(8), MEMORY_NO_CLEAR);
    void* Memory7 = Heap_Allocate(Heap, Kilo(7), MEMORY_NO_CLEAR);
    ASSERT_TRUE(Heap_Verify(Heap));
    
    heap_block_tree* Tree = &Heap->FreeBlockTree;
    
    Heap_Free(Heap, Memory5);
    ASSERT_TRUE(Heap_Verify(Heap));  
    ASSERT_EQ(Memory5, Tree->Root->LeftChild->Block->Block->Memory + Tree->Root->LeftChild->Block->Offset + sizeof(heap_block*));
    Heap_Free(Heap, Memory3);
    ASSERT_TRUE(Heap_Verify(Heap));
    Heap_Free(Heap, Memory4);
    ASSERT_TRUE(Heap_Verify(Heap));
    ASSERT_EQ(Memory3, Tree->Root->Block->Block->Memory + Tree->Root->Block->Offset + sizeof(heap_block*));
    
    Heap_Free(Heap, Memory1);
    ASSERT_TRUE(Heap_Verify(Heap));
    void* Memory8 = Heap_Allocate(Heap, Kilo(30), MEMORY_CLEAR);
    ASSERT_TRUE(Heap_Verify(Heap));
    Heap_Free(Heap, Memory8);
    ASSERT_TRUE(Heap_Verify(Heap));
}

#define HEAP_ALLOC(Heap, Size) \
Heap_Allocate(Heap, Size, MEMORY_CLEAR); ASSERT_TRUE(Heap_Verify(Heap))

#define HEAP_FREE(Heap, Memory) \
Heap_Free(Heap, Memory); ASSERT_TRUE(Heap_Verify(Heap))

UTEST_F(HeapTest, Heap_Stress_Test)
{
    heap* LargeHeap = Heap_Create(Get_Base_Allocator(utest_fixture->Allocator), Mega(64));
    uint32_t Loops = 1;
    for(uint32_t k = 0; k < Loops; k++)
    {
#if 1
        uint32_t Seed;
        OS_Get_Random_Seed(&Seed, sizeof(uint32_t));
#else
        uint32_t Seed = 314003169;
#endif
        random32 Random = Random32_Init_Seed(Seed);
        
        void* FirstBatchBlocks[500];
        for(uint32_t i = 0; i < 500; i++)
        {
            FirstBatchBlocks[i] = HEAP_ALLOC(LargeHeap, Random32_SBetween(&Random, Kilo(1), Mega(2)));
        }
        
        void* SecondBatchBlocks[1000];
        for(uint32_t i = 0; i < 500; i++)
        {
            SecondBatchBlocks[(i*2)] = HEAP_ALLOC(LargeHeap, Random32_SBetween(&Random, Kilo(1), Mega(2)));
            SecondBatchBlocks[(i*2)+1] = HEAP_ALLOC(LargeHeap, Random32_SBetween(&Random, Kilo(1), Mega(2)));
            HEAP_FREE(LargeHeap, FirstBatchBlocks[i]);
        }
        
        void* ThirdBatchBlocks[1000];
        for(uint32_t i = 0; i < 200; i++)
        {
            for(uint32_t j = 0; j < 5; j++)
            {
                ThirdBatchBlocks[i*5+j] = HEAP_ALLOC(LargeHeap, Random32_SBetween(&Random, Kilo(1), Mega(2)));
            }
            
            for(uint32_t j = 0; j < 5; j++)
            {
                HEAP_FREE(LargeHeap, SecondBatchBlocks[i*5+j]);
            }
        }
        
        for(uint32_t i = 0; i < 1000; i++)
        {
            HEAP_FREE(LargeHeap, ThirdBatchBlocks[i]);
        }
    }
    
    Heap_Delete(LargeHeap);
}

UTEST_F_TEARDOWN(HeapTest)
{
    heap* Heap = utest_fixture->Heap;
    custom_allocator* Allocator = utest_fixture->Allocator;
    
    Heap_Delete(Heap);
    Test_Custom_Allocator_Is_Not_Leaking(utest_result, Allocator);
    Free_Custom_Allocator(Allocator);
}