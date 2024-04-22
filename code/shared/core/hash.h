#ifndef HASH_H
#define HASH_H

u32 Hash_U32(uint32_t V);
u32 Hash_U64(uint64_t V);

//FNV-1a hash algorithm
u32 Hash_Bytes(const void* Data, uptr Size, u32 Seed=0x811c9dc5);

u32 Hash_CRC(const void* Data, uptr Size, u32 Seed = 0);

template <typename type>
struct hasher {
    u32 Hash(const type& Value) const;
};

template <typename type>
struct comparer {
    bool Equal(const type& A, const type& B) const;
};

inline void Hash_Combine(u32* Seed) { }

template <typename type, typename... rest, typename hasher=hasher<type>>
inline void Hash_Combine(u32* Seed, const type& Value, rest... Rest) {
    *Seed ^= hasher{}.Hash(Value) + 0x9e3779b9 + (*Seed << 6) + (*Seed >> 2);
    Hash_Combine(Seed, Rest...); 
}

template <typename type, typename... rest, typename hasher=hasher<type>>
inline u32 Hash_Combine(const type& Value, rest... Rest) {
    u32 Result = hasher{}.Hash(Value);
    Hash_Combine(&Result, Rest...);
    return Result;
}

template <>
struct hasher<u32> {
    inline u32 Hash(u32 Key) {
        return Hash_U32(Key);
    }
};

template <>
struct comparer<u32> {
    inline bool Equal(u32 A, u32 B) {
        return A == B;
    }
};

template <>
struct hasher<string> {
    inline u32 Hash(string Key) {
        return Hash_CRC(Key.Str, Key.Size);
    }
};

template <>
struct comparer<string> {
    inline bool Equal(string A, string B) {
        return A == B;
    }
};

#endif