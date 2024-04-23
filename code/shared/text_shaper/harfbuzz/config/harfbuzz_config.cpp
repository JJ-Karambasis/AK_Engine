#include <Core/core.h>

typedef struct {
    uptr ByteSize;
} block;

extern "C" void* hb_malloc_impl(size_t size) {
    uptr Offset = DEFAULT_ALIGNMENT - 1 + sizeof(void*);
    block* Block = (block*)Allocator_Allocate_Memory_Internal(Core_Get_Base_Allocator(), sizeof(block)+Offset+size);
    if(!Block) return nullptr;

    Block->ByteSize = size;
    void* P1  = Block+1;
    void** P2 = (void**)(((uptr)(P1) + Offset) & ~(DEFAULT_ALIGNMENT - 1));
    P2[-1] = P1;
    return P2;
}

extern "C" void* hb_calloc_impl(size_t nmemb, size_t size) {
    return hb_malloc_impl(nmemb*size);
}

extern "C" void* hb_realloc_impl(void *ptr, size_t size) {
    if(!ptr) return hb_malloc_impl(size);
    void* OriginalUnaligned = ((void**)ptr)[-1];
    block* Block = ((block*)OriginalUnaligned)-1;

    uptr Offset = DEFAULT_ALIGNMENT - 1 + sizeof(void*);
    block* NewBlock = (block*)Allocator_Allocate_Memory_Internal(Core_Get_Base_Allocator(), size+Offset+sizeof(block));
    if(!NewBlock) return nullptr;

    NewBlock->ByteSize = size;
    void* P1  = NewBlock+1;
    void** P2 = (void**)(((uptr)(P1) + Offset) & ~(DEFAULT_ALIGNMENT - 1));
    P2[-1] = P1;

    Memory_Copy(P2, ptr, Min(NewBlock->ByteSize, Block->ByteSize));

    Allocator_Free_Memory_Internal(Core_Get_Base_Allocator(), Block);

    return P2;
}

extern "C" void hb_free_impl(void *ptr) {
    if(ptr) {
        void* OriginalUnaligned = ((void**)ptr)[-1];
        block* Block = ((block*)OriginalUnaligned)-1;
        Allocator_Free_Memory_Internal(Core_Get_Base_Allocator(), Block);
    }
}