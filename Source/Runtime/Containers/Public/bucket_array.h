#ifndef BUCKET_ARRAY_H
#define BUCKET_ARRAY_H

typedef struct bucket
{
    void*    Data;
    uint64_t Length;
} bucket;

typedef struct bucket_array
{
    allocator* Allocator;
    arena*     BucketAllocator;
    uint64_t   BucketCapacity;
    size_t     EntrySize;
    uint64_t   EntryCount;
    vector     Buckets;
} bucket_array;

bucket_array Bucket_Array_Create_(allocator* Allocator, size_t EntrySize, uint64_t BucketCapacity);
void         Bucket_Array_Delete(bucket_array* Array);
void         Bucket_Array_Push(bucket_array* Array, void* Entry);
void*        Bucket_Array_Get_(bucket_array* Array, uint64_t Index);

#define Bucket_Array_Create(Allocator, Type, Capacity) Bucket_Array_Create_(Allocator, sizeof(Type), Capacity)
#define Bucket_Array_Get(Array, Type, Index) (Type*)Bucket_Array_Get_(Array, Index)

#endif
