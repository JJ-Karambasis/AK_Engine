#define Implement_Pool_Functions(type, value_type, alias) \
type Pool_Create_##alias(allocator* Allocator, uint16_t InitialCapacity) \
{ \
type Result; \
Zero_Struct(&Result, type); \
Result.Allocator = Allocator; \
Result.Entries = Allocate_Array(Allocator, type##_entry, InitialCapacity); \
Result.Capacity = InitialCapacity; \
Result.NextKey  = 1; \
Result.FirstFreeIndex = (uint16_t)-1; \
Result.FirstIndex = (uint16_t)-1; \
return Result; \
} \
\
u64 Pool_Allocate_##alias(type* Pool) \
{ \
u16 Index; \
if(Pool->FirstFreeIndex != (u16)-1) \
{ \
Index = Pool->FirstFreeIndex; \
Pool->FirstFreeIndex = Pool->Entries[Pool->FirstFreeIndex].ID.NextEntry; \
} \
else \
{ \
if(Pool->MaxUsed == Pool->Capacity) \
{ \
u16 NewCapacity = Min(Pool->Capacity*2, MAX_U16-1); \
if(NewCapacity == Pool->Capacity) \
{ \
/*TODO(JJ): Log error */ \
return 0; \
} \
type##_entry* Entries = Allocate_Array(Pool->Allocator, type##_entry, NewCapacity); \
Memory_Copy(Entries, Pool->Entries, sizeof(type##_entry)*Pool->Capacity); \
Free(Pool->Allocator, Pool->Entries); \
Pool->Entries = Entries; \
Pool->Capacity = NewCapacity; \
} \
Index = Pool->MaxUsed++; \
} \
type##_entry* Entry = Pool->Entries + Index; \
Entry->ID.Key = Pool->NextKey++; \
if(Pool->FirstIndex != (u16)-1) \
{ \
Pool->Entries[Pool->FirstIndex].ID.PrevEntry = Index; \
} \
Entry->ID.NextEntry = Pool->FirstIndex; \
Entry->ID.PrevEntry = (u16)-1; \
Pool->FirstIndex = Index; \
if(Pool->NextKey == 0) Pool->NextKey = 1; \
Pool->Count++; \
pool_id Result; \
Zero_Struct(&Result, pool_id); \
Result.Key = Entry->ID.Key; \
Result.Index = Index; \
return Result.ID; \
} \
\
b8 Pool_Is_Allocated_##alias(type* Pool, u64 ID) \
{ \
if(ID) \
{ \
pool_id PoolID; \
PoolID.ID = ID; \
type##_entry* Entry = Pool->Entries + PoolID.Index; \
b8 Result = Entry->ID.Key && Entry->ID.Key == PoolID.Key; \
return Result; \
} \
return false; \
} \
\
void Pool_Free_##alias(type* Pool, u64 ID) \
{ \
if(Pool_Is_Allocated_##alias(Pool, ID)) \
{ \
pool_id PoolID; \
PoolID.ID = ID; \
type##_entry* Entry = Pool->Entries + PoolID.Index; \
b32 IsHead = PoolID.Index == Pool->FirstIndex; \
if(!IsHead) \
{ \
Assert(Entry->ID.PrevEntry != (u16)-1); \
Pool->Entries[Entry->ID.PrevEntry].ID.NextEntry = Entry->ID.NextEntry; \
if(Entry->ID.NextEntry != (u16)-1) \
Pool->Entries[Entry->ID.NextEntry].ID.PrevEntry = Entry->ID.PrevEntry; \
} \
else \
{ \
Assert(Entry->ID.PrevEntry == (u16)-1); \
Pool->FirstIndex = Entry->ID.NextEntry; \
} \
if(Entry->ID.NextEntry != (u16)-1) \
{ \
type##_entry* NextEntry = Pool->Entries + Entry->ID.NextEntry; \
NextEntry->ID.PrevEntry = Entry->ID.PrevEntry; \
} \
Entry->ID.ID = 0; \
Entry->ID.NextEntry = Pool->FirstFreeIndex; \
Zero_Struct(&Entry->Entry, value_type); \
Pool->FirstFreeIndex = PoolID.Index; \
Pool->Count--; \
} \
} \
\
value_type Pool_Get_Value_##alias(type* Pool, u64 ID) \
{ \
Assert(Pool_Is_Allocated_##alias(Pool, ID)); \
pool_id PoolID; \
PoolID.ID = ID; \
return Pool->Entries[PoolID.Index].Entry; \
} \
\
value_type* Pool_Get_Ptr_##alias(type* Pool, u64 ID) \
{ \
if(!Pool_Is_Allocated_##alias(Pool, ID)) return NULL; \
pool_id PoolID; \
PoolID.ID = ID; \
return &Pool->Entries[PoolID.Index].Entry; \
} \
\
void Pool_Delete_##alias(type* Pool) \
{ \
Free(Pool->Allocator, Pool->Entries); \
Pool->Entries = NULL; \
Pool->Capacity = 0; \
Pool->Count = 0; \
} \
\
type##_iter Pool_Begin_Iter_##alias(type* Pool) \
{ \
type##_iter Iter; \
Iter.Pool = Pool; \
return Iter; \
} \
\
value_type* Pool_Get_First_##alias(type##_iter* Iter) \
{ \
Iter->Index = Iter->Pool->FirstIndex; \
if(Iter->Index == (u16)-1) return NULL; \
return &Iter->Pool->Entries[Iter->Index].Entry; \
} \
\
value_type* Pool_Get_Next_##alias(type##_iter* Iter) \
{ \
Iter->Index = Iter->Pool->Entries[Iter->Index].ID.NextEntry; \
if(Iter->Index == (u16)-1) return NULL;\
return &Iter->Pool->Entries[Iter->Index].Entry; \
} \
type G__Unused_Pool_Sentinal_##alias

