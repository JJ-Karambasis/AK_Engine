#ifndef DIRECTSOUND_H
#define DIRECTSOUND_H

#include <e

struct dsound : public adi {
    heap*           MainHeap;
    lock_allocator* MainAllocator;
};

#endif