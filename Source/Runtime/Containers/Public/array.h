#ifndef ARRAY_H
#define ARRAY_H

typedef struct array
{
    void*    Entries;
    size_t   EntrySize;
    uint64_t EntryCount;
} array;

array Array_Init_(void* Entries, size_t EntrySize, uint64_t Count);
array Array_Push_(arena* Arena, size_t EntrySize, uint64_t Count);
void* Array_Get_(array* Array, uint64_t Index);

#define Array_Init(Entries, Type, Count) Array_Init_(Entries, sizeof(Type), Count)
#define Array_Push(Arena, Type, Count) Array_Push_(Arena, sizeof(Type), Count)
#define Array_Get(Array, Type, Index) (Type*)Array_Get_(Array, Index)

#endif