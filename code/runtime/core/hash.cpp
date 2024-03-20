u32 Hash_U32(u32 Key) {
    Key = (Key+0x7ed55d16) + (Key<<12);
    Key = (Key^0xc761c23c) ^ (Key>>19);
    Key = (Key+0x165667b1) + (Key<<5);
    Key = (Key+0xd3a2646c) ^ (Key<<9);
    Key = (Key+0xfd7046c5) + (Key<<3);
    Key = (Key^0xb55a4f09) ^ (Key>>16);
    return Key;
}

u32 Hash_U64(u64 Key) {
    Key = (~Key) + (Key << 18); // Key = (Key << 18) - Key - 1;
    Key = Key ^ (Key >> 31);
    Key = Key * 21; // Key = (Key + (Key << 2)) + (Key << 4);
    Key = Key ^ (Key >> 11);
    Key = Key + (Key << 6);
    Key = Key ^ (Key >> 22);
    return (u32)Key;
}

u32 Hash_Bytes(const void* Data, uptr Size, u32 Seed) {
    u32 Hash = Seed;
    const u8* Ptr = (const u8*)Data;
    while(Size--) {
        Hash ^= (u32)*Ptr;
        Hash *= 0x01000193;
        Ptr++;
    }
    return Hash;
}