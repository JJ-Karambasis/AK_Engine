#ifndef HASH_H
#define HASH_H

#define HASH_FUNCTION(name) uint32_t name(void* Key)
#define KEY_COMPARE(name) bool8_t name(void* LeftKey, void* RightKey)

typedef HASH_FUNCTION(hash_function);
typedef KEY_COMPARE(key_compare);

#endif