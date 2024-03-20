#ifndef HASH_H
#define HASH_H

u32 Hash_U32(uint32_t V);
u32 Hash_U64(uint64_t V);

//FNV-1a hash algorithm
u32 Hash_Bytes(const void* Data, uptr Size, u32 Seed=0x811c9dc5);

#endif