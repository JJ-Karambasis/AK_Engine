#ifndef VECTOR_H
#define VECTOR_H

typedef struct vector
{
    allocator* Allocator;
    void*      Entries;
    size_t     EntrySize;
    uint64_t   EntryCount;
    uint64_t   EntryCapacity;
} vector;

vector Vector_Create_(allocator* Allocator, size_t EntrySize, uint64_t EntryCapacity);
void   Vector_Delete(vector* Vector);
void   Vector_Reserve(vector* Vector, uint64_t NewCapacity);
void   Vector_Push(vector* Vector, void* Entry);
void*  Vector_Get_(vector* Vector, uint64_t Index);

#define Vector_Create(Allocator, Type, Capacity) Vector_Create_(Allocator, sizeof(Type), Capacity)
#define Vector_Get(Vector, Type, Index) (Type*)Vector_Get_(Vector, Index)

#endif
