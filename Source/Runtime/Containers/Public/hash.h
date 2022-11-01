#ifndef HASH_H
#define HASH_H

#define HASH_FUNCTION(name) uint32_t name(void* Key)
#define KEY_COMPARE(name) bool8_t name(void* LeftKey, void* RightKey)

typedef HASH_FUNCTION(hash_function);
typedef KEY_COMPARE(key_compare);

HASH_FUNCTION(Hash_U32);
HASH_FUNCTION(Hash_U64);
HASH_FUNCTION(Hash_Ptr);

KEY_COMPARE(Key_Compare_U32);
KEY_COMPARE(Key_Compare_U64);
KEY_COMPARE(Key_Compare_Ptr);

void Hash_Combine(uint32_t* Result, uint32_t Hash);

#endif