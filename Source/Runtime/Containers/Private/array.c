#define Implement_Array(type, alias) \
type##_array Array_Allocate_##alias(allocator* Allocator, uint64_t Count) \
{ \
type##_array Result; \
Result.Ptr = (type*)Allocate(Allocator, sizeof(type)*Count, MEMORY_NO_CLEAR); \
Result.Count = Count; \
return Result; \
} \
\
type Array_Get_Value_##alias(type##_array* Array, uint64_t Index) \
{ \
Assert(Index < Array->Count); \
return Array->Ptr[Index]; \
} \
\
type* Array_Get_Ptr_##alias(type##_array* Array, uint64_t Index) \
{ \
Assert(Index < Array->Count); \
return Array->Ptr + Index; \
} \
\
void Array_Free_##alias(allocator* Allocator, type##_array* Array) \
{ \
Free(Allocator, Array->Ptr); \
Array->Ptr = NULL; \
Array->Count = 0; \
} \
type##_array G__Unused_Array_Sentinal_##alias

Implement_Array(u8,  U8);
Implement_Array(u16, U16);
Implement_Array(u32, U32);
Implement_Array(u64, U64);
Implement_Array(s8,   S8);
Implement_Array(s16,  S16);
Implement_Array(s32,  S32);
Implement_Array(s64,  S64);
Implement_Array(f32,    F32);
Implement_Array(f64,   F64);
