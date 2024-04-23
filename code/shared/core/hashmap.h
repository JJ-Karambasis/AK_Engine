#ifndef HASHMAP_H
#define HASHMAP_H

#define DEFAULT_HASHMAP_ITEM_CAPACITY 32

struct hash_slot {
    static const u32 INVALID = (u32)-1;

    u32 Hash = 0;
    u32 ItemIndex = (u32)-1;
    u32 BaseCount = 0;
};

#define HASHMAP_INVALID hash_slot::INVALID

template <typename key, typename value>
struct kvp {
    const key& Key;
    value& Value;
};

template <typename key, typename value, typename hasher = hasher<key>, typename comparer = comparer<key>>
struct hashmap {
    allocator* Allocator = nullptr;
    hash_slot* Slots = nullptr;
    key*       Keys = nullptr;
    value*     Values = nullptr;
    u32*       ItemSlots = nullptr;
    u32        SlotCapacity = 0;
    u32        ItemCapacity = 0;
    u32        Count = 0;

    hashmap() = default;
    hashmap(allocator* _Allocator);
    hashmap(allocator* _Allocator, u32 ItemCount);

    struct kvp_iter {
        hashmap* Hashmap;
        u32      Index;

        inline kvp<key, value> operator*() const {
            return {Hashmap->Keys[Index], Hashmap->Values[Index]};
        }

        inline bool operator!=(const kvp_iter& Other) const {
            return Other.Index != Index;
        }

        inline kvp_iter& operator++() {
            Index++;
            return *this;
        } 
    };

    inline kvp_iter begin() {
        kvp_iter Result = {this, 0};
        return Result;
    }

    inline kvp_iter end() {
        kvp_iter Result = {this, Count};
        return Result;
    }
};

inline u32 Expand_Slots(allocator* Allocator, hash_slot** Slots, u32 OldCapacity, u32 NewCapacity, u32* ItemSlots) {
    u32 SlotCapacity = Ceil_Pow2(NewCapacity);
    u32 SlotMask = SlotCapacity-1;
    hash_slot* NewSlots = (hash_slot*)Allocator_Allocate_Memory(Allocator, SlotCapacity*sizeof(hash_slot));
    
    for(u32 SlotIndex = 0; SlotIndex < SlotCapacity; SlotIndex++) {
        NewSlots[SlotIndex] = hash_slot();
    }
    
    for(u32 i = 0; i < OldCapacity; i++)
    {
        if((*Slots)[i].ItemIndex != hash_slot::INVALID)
        {
            u32 Hash = (*Slots)[i].Hash;
            u32 BaseSlot = (Hash & SlotMask); 
            u32 Slot = BaseSlot;
            while(NewSlots[Slot].ItemIndex != hash_slot::INVALID)
                Slot = (Slot + 1) & SlotMask;
            NewSlots[Slot].Hash = Hash;
            u32 ItemIndex = (*Slots)[i].ItemIndex;
            NewSlots[Slot].ItemIndex = ItemIndex;
            ItemSlots[ItemIndex] = Slot;
            NewSlots[BaseSlot].BaseCount++;
        }
    }

    if(*Slots) Allocator_Free_Memory(Allocator, *Slots);
    *Slots = NewSlots;
    return SlotCapacity;
}


template <typename key, typename value>
inline u32 Expand_Items(allocator* Allocator, key** Keys, value** Values, u32** ItemSlots, u32 OldItemCapacity, u32 NewItemCapacity) {
    u32 ItemCapacity = NewItemCapacity;

    uptr ItemSize = sizeof(key)+sizeof(u32);
    if(Values) ItemSize += sizeof(value);

    key* NewKeys = (key*)Allocator_Allocate_Memory(Allocator, ItemSize*ItemCapacity);
    value* NewValues = Values ? (value*)(NewKeys+ItemCapacity) : nullptr;
    u32* NewItemSlots = Values ? (u32*)(NewValues+ItemCapacity) : (u32*)(NewKeys+ItemCapacity);

    for(u32 ItemIndex = 0; ItemIndex < ItemCapacity; ItemIndex++)
        NewItemSlots[ItemIndex] = hash_slot::INVALID;

    if(*Keys) {
        Array_Copy(NewKeys, *Keys, OldItemCapacity);
        if(NewValues) Array_Copy(NewValues, *Values, OldItemCapacity);
        Array_Copy(NewItemSlots, *ItemSlots, OldItemCapacity);
        Allocator_Free_Memory(Allocator, *Keys); 
    }

    *Keys = NewKeys;
    if(Values) *Values = NewValues;
    *ItemSlots = NewItemSlots;
    return ItemCapacity;
}

template <typename key, typename comparer>
inline u32 Find_Slot(hash_slot* Slots, u32 SlotCapacity, key* Keys, const key& Key, u32 Hash) {
    if(SlotCapacity == 0 || !Slots) return hash_slot::INVALID;

    u32 SlotMask = SlotCapacity-1;
    u32 BaseSlot = (Hash & SlotMask);
    u32 BaseCount = Slots[BaseSlot].BaseCount;
    u32 Slot = BaseSlot;
    
    while (BaseCount > 0) {
        if (Slots[Slot].ItemIndex != hash_slot::INVALID) {
            u32 SlotHash = Slots[Slot].Hash;
            u32 SlotBase = (SlotHash & SlotMask);
            if (SlotBase == BaseSlot) {
                Assert(BaseCount > 0);
                BaseCount--;
                            
                if (SlotHash == Hash) { 
                    comparer Comparer = {};
                    if(Comparer.Equal(Key, Keys[Slots[Slot].ItemIndex]))
                        return Slot;
                }
            }
        }
        
        Slot = (Slot + 1) & SlotMask;
    }
    
    return hash_slot::INVALID;
}

inline u32 Find_Free_Slot(hash_slot* Slots, u32 SlotMask, u32 BaseSlot) {
    u32 BaseCount = Slots[BaseSlot].BaseCount;
    u32 Slot = BaseSlot;
    u32 FirstFree = Slot;
    while (BaseCount) 
    {
        if (Slots[Slot].ItemIndex == hash_slot::INVALID && Slots[FirstFree].ItemIndex != hash_slot::INVALID) FirstFree = Slot;
        u32 SlotHash = Slots[Slot].Hash;
        u32 SlotBase = (SlotHash & SlotMask);
        if (SlotBase == BaseSlot) 
            --BaseCount;
        Slot = (Slot + 1) & SlotMask;
    }
    
    Slot = FirstFree;
    while (Slots[Slot].ItemIndex != hash_slot::INVALID) 
        Slot = (Slot + 1) & SlotMask;

    return Slot;
}

template <typename key, typename value, typename hasher, typename comparer>
inline void Hashmap_Init(hashmap<key, value, hasher, comparer>* Hashmap, allocator* _Allocator, u32 ItemCount = DEFAULT_HASHMAP_ITEM_CAPACITY) {
    Zero_Struct(Hashmap);
    Hashmap->Allocator = _Allocator;
    Hashmap->ItemCapacity = Expand_Items(Hashmap->Allocator, &Hashmap->Keys, &Hashmap->Values, &Hashmap->ItemSlots, 0, ItemCount);
    Hashmap->SlotCapacity = Expand_Slots(Hashmap->Allocator, &Hashmap->Slots, 0, ItemCount*2, Hashmap->ItemSlots);
}

template <typename key, typename value, typename hasher, typename comparer>
inline void Hashmap_Add(hashmap<key, value, hasher, comparer>* Hashmap, const key& Key, const value& Value) {
    u32 Hash = hasher{}.Hash(Key);
    return Hashmap_Add_By_Hash(Hashmap, Key, Hash, Value);
}

template <typename key, typename value, typename hasher, typename comparer>
inline void Hashmap_Add_By_Hash(hashmap<key, value, hasher, comparer>* Hashmap, const key& Key, u32 Hash, const value& Value) {
    Assert((Find_Slot<key, comparer>(Hashmap->Slots, Hashmap->SlotCapacity, Hashmap->Keys, Key, Hash) == HASHMAP_INVALID));
    if(Hashmap->Count >= (Hashmap->SlotCapacity - (Hashmap->SlotCapacity/3))) {
        u32 NewSlotCapacity = Hashmap->SlotCapacity ? Hashmap->SlotCapacity*2 : DEFAULT_HASHMAP_ITEM_CAPACITY*2;
        Hashmap->SlotCapacity = Expand_Slots(Hashmap->Allocator, &Hashmap->Slots, Hashmap->SlotCapacity, NewSlotCapacity, Hashmap->ItemSlots);
    }

    u32 SlotMask = Hashmap->SlotCapacity-1;
    u32 BaseSlot = (Hash & SlotMask);
    u32 Slot = Find_Free_Slot(Hashmap->Slots, SlotMask, BaseSlot);
    
    if (Hashmap->Count >= Hashmap->ItemCapacity) {
        u32 NewItemCapacity = Hashmap->ItemCapacity ? Hashmap->ItemCapacity*2 : DEFAULT_HASHMAP_ITEM_CAPACITY;
        Hashmap->ItemCapacity = Expand_Items(Hashmap->Allocator, &Hashmap->Keys, &Hashmap->Values, &Hashmap->ItemSlots, Hashmap->ItemCapacity, NewItemCapacity);
    }

    Assert(Hashmap->Count < Hashmap->ItemCapacity);
    Assert(Hashmap->Slots[Slot].ItemIndex == HASHMAP_INVALID && (Hash & SlotMask) == BaseSlot);
    
    Hashmap->Slots[Slot].Hash = Hash;
    Hashmap->Slots[Slot].ItemIndex = Hashmap->Count;
    Hashmap->Slots[BaseSlot].BaseCount++;
    
    Hashmap->ItemSlots[Hashmap->Count] = Slot;
    Hashmap->Keys[Hashmap->Count] = Key;
    Hashmap->Values[Hashmap->Count] = Value;

    Hashmap->Count++;
    
}

template <typename key, typename value, typename hasher, typename comparer>
inline value* Hashmap_Find_By_Hash(hashmap<key, value, hasher, comparer>* Hashmap, const key& Key, u32 Hash) {
    u32 CurrentSlot = Find_Slot<key, comparer>(Hashmap->Slots, Hashmap->SlotCapacity, Hashmap->Keys, Key, Hash);
    if(CurrentSlot == HASHMAP_INVALID) return nullptr;
    return Hashmap->Values + Hashmap->Slots[CurrentSlot].ItemIndex;
}

template <typename key, typename value, typename hasher, typename comparer>
inline value* Hashmap_Find(hashmap<key, value, hasher, comparer>* Hashmap, const key& Key) {
    u32 Hash = hasher{}.Hash(Key);
    return Hashmap_Find_By_Hash(Hashmap, Key, Hash);
}

template <typename key, typename value, typename hasher, typename comparer>
inline value* Hashmap_Find_Or_Create(hashmap<key, value, hasher, comparer>* Hashmap, const key& Key) {
    u32 Hash = hasher{}.Hash(Key);
    value* Result = Hashmap_Find_By_Hash(Hashmap, Key, Hash);
    if(!Result) {
        Hashmap_Add_By_Hash(Hashmap, Key, Hash, {});
        Result = Hashmap_Find_By_Hash(Hashmap, Key, Hash);
    } 
    return Result;
}

template <typename key, typename value, typename hasher, typename comparer>
inline void Hashmap_Remove(hashmap<key, value, hasher, comparer>* Hashmap, const key& Key) {
    u32 Hash = hasher{}.Hash(Key);
    u32 Slot = Find_Slot<key, comparer>(Hashmap->Slots, Hashmap->SlotCapacity, Hashmap->Keys, Key, Hash);
    
    if(Slot == HASHMAP_INVALID) return;

    u32 SlotMask = Hashmap->SlotCapacity-1;
    u32 BaseSlot = (Hash & SlotMask);
    u32 Index = Hashmap->Slots[Slot].ItemIndex;
    u32 LastIndex = Count-1;

    Hashmap->Slots[BaseSlot].BaseCount--;
    Hashmap->Slots[Slot].ItemIndex = HASHMAP_INVALID;

    if(Index != LastIndex) {
        Hashmap->Keys[Index] = Hashmap->Keys[LastIndex];
        Hashmap->Values[Index] = Hashmap->Values[LastIndex];
        Hashmap->ItemSlots[Index] = Hashmap->ItemSlots[LastIndex];
        Hashmap->Slots[Hashmap->ItemSlots[LastIndex]].ItemIndex = Index;
    }

    Hashmap->Count--;
}

template <typename key, typename value, typename hasher, typename comparer>
inline void Hashmap_Clear(hashmap<key, value, hasher, comparer>* Hashmap) {
    for(u32 SlotIndex = 0; SlotIndex < Hashmap->SlotCapacity; SlotIndex++) {
        Hashmap->Slots[SlotIndex] = hash_slot();
    }

    Zero_Array(Hashmap->Keys, Count);
    Zero_Array(Hashmap->Values, Count);
    for(u32 ItemIndex = 0; ItemIndex < Hashmap->ItemCapacity; ItemIndex++) 
        Hashmap->ItemSlots[ItemIndex] = HASHMAP_INVALID;
    Hashmap->Count = 0;
}

template <typename key, typename value, typename hasher, typename comparer>
inline void Hashmap_Free(hashmap<key, value, hasher, comparer>* Hashmap) {
    Hashmap->SlotCapacity = 0;
    Hashmap->ItemCapacity = 0;
    Hashmap->Count = 0;
    if(Hashmap->Slots) {
        Allocator_Free_Memory(Hashmap->Allocator, Hashmap->Slots);
        Hashmap->Slots = nullptr;
    }

    if(Hashmap->Keys) {
        Allocator_Free_Memory(Hashmap->Allocator, Hashmap->Keys);
        Hashmap->Keys = nullptr;
        Hashmap->Values = nullptr;
        Hashmap->ItemSlots = nullptr;
    }
}

template <typename key, typename value, typename hasher, typename comparer>
inline hashmap<key, value, hasher, comparer>::hashmap(allocator* _Allocator) : hashmap(_Allocator, DEFAULT_HASHMAP_ITEM_CAPACITY) { }

template <typename key, typename value, typename hasher, typename comparer>
inline hashmap<key, value, hasher, comparer>::hashmap(allocator* _Allocator, u32 ItemCount) {
   Hashmap_Init(this, _Allocator, ItemCount);
}

#endif