#ifndef HASHMAP_H
#define HASHMAP_H

#define HASH_FUNCTION(name) uint32_t name(void* Key)
#define KEY_COMPARE(name) bool8_t name(void* LeftKey, void* RightKey)

typedef HASH_FUNCTION(hash_function);
typedef KEY_COMPARE(key_compare);

typedef struct hash_slot
{
    bool32_t IsValid;
    uint32_t ItemIndex;
    uint32_t BaseCount;
    uint32_t Hash;
} hash_slot;

typedef struct hash_slot_list
{
    uint32_t   SlotCapacity;
    hash_slot* Slots;
} hash_slot_list;

typedef struct hashmap
{
    allocator*     Allocator;
    hash_slot_list SlotList;
    uint32_t       EntryCount;
    uint32_t       EntryCapacity;
    uint32_t*      EntrySlots;
    size_t         KeyItemSize;
    size_t         ValueItemSize;
    void*          Keys;
    void*          Values;
    hash_function* Hash;
    key_compare*   KeyCompare;
} hashmap;

hashmap Hashmap_Create_(allocator* Allocator, size_t KeySize, size_t ValueSize, hash_function* Hash, key_compare* KeyCompare, uint32_t InitialSlotCapacity, uint32_t InitialEntryCapacity);
void    Hashmap_Delete(hashmap* Hashmap);
void    Hashmap_Add(hashmap* Hashmap, void* Key, void* Value);
void*   Hashmap_Find_(hashmap* Hashmap, void* Key);
void    Hashmap_Remove(hashmap* Hashmap, void* Key);
void    Hashmap_Clear(hashmap* Hashmap);

HASH_FUNCTION(Hash_U32);

KEY_COMPARE(Key_Compare_U32);

#define Hashmap_Create(Allocator, KeyType, ValueType, Hash, KeyCompare, SlotCapacity, EntryCapacity) Hashmap_Create_(Allocator, sizeof(KeyType), sizeof(ValueType), Hash, KeyCompare, SlotCapacity, EntryCapacity)
#define Hashmap_Find(Hashmap, ValueType, Key) (ValueType*)Hashmap_Find_(Hashmap, Key)

#endif
