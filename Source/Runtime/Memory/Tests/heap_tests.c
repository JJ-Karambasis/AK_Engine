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
}

UTEST_F(HeapTest, Allocate)
{
}

UTEST_F_TEARDOWN(HeapTest)
{
    heap* Heap = utest_fixture->Heap;
    custom_allocator* Allocator = utest_fixture->Allocator;
    
    Heap_Delete(Heap);
    Test_Custom_Allocator_Is_Not_Leaking(utest_result, Allocator);
    Free_Custom_Allocator(Allocator);
}