#define Implement_Hashmap_Functions(type, value_type, key_type, alias) \
\
hash_slot_entry* Hashmap__Find_Slot_Entry_##alias(type* Hashmap, key_type* Key, hash_slot** OutSlot) \
{ \
} \
\
type Hashmap_Create_##alias(allocator* Allocator, u32 SlotCapacity, u32 ItemCapacity) \
{ \
type Result; \
Zero_Struct(Result, type); \
Result.Allocator = Allocator; \
Result.Arena = Arena_Create(Allocator, Kilo(128)); \
Result.SlotCount = Ceil_Pow2_U64(SlotCapacity); \
Result.Slots = Arena_Push_Array(Result.Arena, hash_slot, SlotCapacity); \
Result.ItemCapacity = ItemCapacity; \
Result.Keys = (key_type*)Allocate(Allocator, (sizeof(key_type)+sizeof(value_type))*ItemCapacity, MEMORY_CLEAR); \
Result.Values = (value_type*)(Result.Keys + ItemCapacity); \
return Result; \
} \
\
void Hashmap_Delete_##alias(type* Hashmap) \
{ \
Free(Hashmap->Allocator, Hashmap->Keys); \
Arena_Delete(Hashmap->Arena);\
Hashmap->Allocator = NULL; \
Hashmap->Arena = NULL; \
Hashmap->Slots = NULL; \
Hashmap->Keys = NULL; \
Hashmap->Value = NULL; \
Hashmap->FreeSlotEntries = NULL; \
Hashmap->ItemCapacity = 0; \
Hashmap->ItemCount = 0; \
Hashmap->SlotCount = 0; \
} \
\
void Hashmap_Add_##alias(type* Hashmap, key_type* Key, value_type* Value) \
{ \
Assert(!Hashmap__Find_Slot_Entry_##alias(Hashmap, Key, NULL)); \
if(Hashmap->ItemCount == Hashmap->ItemCapacity) \
{ \
key_type* NewKeys = Allocate(Hashmap->Allocator, (sizeof(key_type)+sizeof(value_type))*Hashmap->ItemCapacity*2, MEMORY_CLEAR); \
value_type* NewValues = NewKeys+(Hashmap->ItemCapacity*2); \
Memory_Copy(NewKeys, Hashmap->Keys, sizeof(key_type)*Hashmap->ItemCapacity); \
Memory_Copy(NewValues, Hashmap->Values, sizeof(value_type)*Hashmap->ItemCapacity); \
Hashmap->ItemCapacity *= 2; \
} \
u32 Hash = Hashmap->Hash_Function(Key); \
u32 SlotMask = Hashmap->SlotCount-1; \
u32 SlotIndex = SlotMask & Hash; \
hash_slot* Slot = Hashmap->Slots + SlotIndex; \
hash_slot_entry* SlotEntry = Hashmap__Allocate_Slot_Entry(Hashmap->Arena, &Hashmap->FreeSlotEntries); \
SlotEntry->ItemIndex = Hashmap->ItemCount++; \
SlotEntry->Hash = Hash; \
if(!Slot->First) Slot->First = Slot->Last = SlotEntry; \
else \
{ \
Slot->Last->Next = SlotEntry; \
SlotEntry->Prev = Slot->Last; \
Slot->Last = SlotEntry; \
} \
Slot->Count++; \
Hashmap->Keys[SlotEntry->ItemIndex] = *Key; \
Hashmap->Values[SlotEntry->ItemIndex] = *Value; \
} \
\
value_type Hashmap_Get_Value_##alias(type* Hashmap, key_type* Key) \
{ \
hash_slot_entry* SlotEntry = Hashmap__Find_Slot_Entry_##alias(Hashmap, Key, NULL); \
Assert(SlotEntry); \
return Hashmap->Values[SlotEntry->ItemIndex]; \
} \
\
value_type* Hashmap_Get_Ptr_##alias(type* Hashmap, key_type* Key) \
{ \
hash_slot_entry* SlotEntry = Hashmap__Find_Slot_Entry_##alias(Hashmap, Key, NULL); \
if(SlotEntry) return Hashmap->Values + SlotEntry->ItemIndex; \
return NULL; \
} \
\
void Hashmap_Remove_##alias(type* Hashmap, key_type* Key) \
{ \
hash_slot* Slot; \
hash_slot_entry* SlotEntry = Hashmap__Find_Slot_Entry_##alias(Hashmap, Key, &Slot); \
Assert(SlotEntry); \
} \
type G__Unused_Hashmap_Sentinal_##alias
