ALLOCATOR_ALLOCATE(Win32_Allocate_Memory)
{
    win32_allocator* Win32Allocator = (win32_allocator*)Allocator;
    
    void* Result = VirtualAlloc(NULL, Size+sizeof(size_t), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    if(Result)
    {
        size_t* SizePtr = (size_t*)Result;
        *SizePtr = Size;
        
        Atomic_Increment64(&Win32Allocator->NumberOfAllocations);
        Atomic_Increment64(&Win32Allocator->NumberOfActiveAllocations);
        Atomic_Add64(&Win32Allocator->NumberOfAllocatedBytes, Size);
        Atomic_Add64(&Win32Allocator->NumberOfActiveBytes, Size);
        
        return SizePtr+1;
    }
    return NULL;
}

ALLOCATOR_FREE(Win32_Free_Memory)
{
    win32_allocator* Win32Allocator = (win32_allocator*)Allocator;
    size_t* SizePtr = (size_t*)Memory;
    int64_t Size = (int64_t)SizePtr[-1];
    VirtualFree(SizePtr, 0, MEM_RELEASE);
    
    Atomic_Increment64(&Win32Allocator->NumberOfFrees);
    Atomic_Decrement64(&Win32Allocator->NumberOfActiveAllocations);
    Atomic_Add64(&Win32Allocator->NumberOfFreedBytes, Size);
    Atomic_Add64(&Win32Allocator->NumberOfActiveBytes, -Size);
}

void Win32_Get_Main_Allocator(win32_allocator* Allocator)
{
    Allocator->Allocator.Allocate = Win32_Allocate_Memory;
    Allocator->Allocator.Free = Win32_Free_Memory;
}