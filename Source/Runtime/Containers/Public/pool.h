#ifndef POOL_H
#define POOL_H

typedef union pool_id
{
    uint64_t ID;
    struct
    {
        uint32_t Key;
        union
        {
            struct
            {
                uint16_t PrevEntry;
                uint16_t NextEntry;
            };
            struct
            {
                uint16_t Unused;
                uint16_t Index;
            };
        };
    };
} pool_id;

typedef struct pool_entry
{
    pool_id ID;
    void*   Entry;
} pool_entry;

typedef struct pool
{
    allocator*   Allocator;
    size_t       EntrySize;
    bucket_array Entries;
    uint32_t     NextKey;
    uint16_t     FirstFreeIndex;
    uint16_t     FirstIndex;
    uint16_t     Length;
} pool;

typedef struct pool_handle
{
    pool*   Pool;
    pool_id ID;
} pool_handle;

typedef struct pool_iter
{
    bool32_t IsValid;
} pool_iter;

pool        Pool_Create_(allocator* Allocator, size_t EntrySize, uint64_t BucketCapacity);
void        Pool_Delete(pool* Pool);
pool_handle Pool_Allocate(pool* Pool);
void*       Pool_Get_(pool* Pool, pool_handle Handle);
void        Pool_Free(pool* Pool, pool_handle Handle);
void        Pool_Clear(pool* Pool);
pool_iter   Pool_Begin_Iter(pool* Pool);
void        Pool_Next_Iter(pool_iter* Iter);
void*       Pool_Get_Iter_Entry_(pool_iter* Iter);

#define Pool_Create(Allocator, Type, Capacity) Pool_Create_(Allocator, sizeof(Type), Capacity)
#define Pool_Get(Pool, Type, Handle) (Type*)Pool_Get_(Pool, Handle)
#define Pool_Get_Iter_Entry(Iter, Type) (Type*)Pool_Get_Iter_Entry_(Iter)

#endif