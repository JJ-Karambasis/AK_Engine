#ifndef ALLOCATOR_H
#define ALLOCATOR_H

typedef struct allocator allocator;

#define ALLOCATOR_ALLOCATE(name) void* name(allocator* Allocator, size_t Size)
typedef ALLOCATOR_ALLOCATE(allocator_allocate);

#define ALLOCATOR_FREE(name) void name(allocator* Allocator, void* Memory)
typedef ALLOCATOR_FREE(allocator_free);

struct allocator
{
    allocator_allocate* Allocate;
    allocator_free*     Free;
    void*               UserData;
};

#endif