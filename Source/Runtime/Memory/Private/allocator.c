void* Allocate(allocator* Allocator, size_t Size, memory_clear_flag ClearFlag)
{
    void* Result = Allocator->Allocate(Allocator, Size);
    if(Result && (ClearFlag == MEMORY_CLEAR)) Memory_Clear(Result, Size);
    return Result;
}

void Free(allocator* Allocator, void* Memory)
{
    Allocator->Free(Allocator, Memory);
}

void* Allocate_Aligned(allocator* Allocator, size_t Size, size_t Alignment, memory_clear_flag ClearFlag)
{
    Assert(Alignment > 0 && Is_Pow2(Alignment));
    
    void*  P1; // original block
    void** P2; // aligned block
    size_t Offset = Alignment - 1 + sizeof(void*);
    if ((P1 = (void*)Allocate(Allocator, Size + Offset, MEMORY_NO_CLEAR)) == NULL)
    {
        return NULL;
    }
    
    P2= (void**)(((size_t)(P1) + Offset) & ~(Alignment - 1));
    P2[-1] = P1;
    
    if(ClearFlag == MEMORY_CLEAR) Memory_Clear(P2, Size);
    
    return P2;
}

void Free_Aligned(allocator* Allocator, void* Memory)
{
    Free(Allocator, ((void**)Memory)[-1]);
}