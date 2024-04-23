#include <core/core.h>
#include "SBConfig.h"

void* Sheenbidi_Allocate_Memory(allocator* Allocator, uptr Size) {
    return Allocator_Allocate_Memory(Allocator, Size);
}

void Sheenbidi_Free_Memory(allocator* Allocator, void* Data) {
    Allocator_Free_Memory(Allocator, Data);
}