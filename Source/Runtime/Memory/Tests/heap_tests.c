typedef struct HeapTest
{
    custom_allocator* Allocator;
    heap* Heap;
} HeapTest;

UTEST_F_SETUP(HeapTest)
{
    custom_allocator* Allocator = Allocate_Custom_Allocator();
    utest_fixture->Allocator = Allocator;
    
    heap* Heap = Heap_Create(&utest_fixture->Allocator->Allocator, Kilo(512));
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
    void* Memory8 = Heap_Allocate(Heap, Kilo(30), MEMORY_CLEAR);
    Heap_Free(Heap, Memory8);
    
}

UTEST_F_TEARDOWN(HeapTest)
{
    heap* Heap = utest_fixture->Heap;
    custom_allocator* Allocator = utest_fixture->Allocator;
    
    Heap_Delete(Heap);
    Test_Custom_Allocator_Is_Not_Leaking(utest_result, Allocator);
    Free_Custom_Allocator(Allocator);
}