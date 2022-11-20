#define Implement_Vector_Functions(type, value_type, alias) \
type Vector_Create_##alias(allocator* Allocator, uint64_t InitialCapacity) \
{ \
type Result; \
Zero_Struct(&Result, type); \
Result.Allocator = Allocator; \
Result.Ptr = Allocate_Array(Allocator, value_type, InitialCapacity); \
Result.Capacity = InitialCapacity; \
return Result; \
} \
\
void Vector_Push_##alias(type* Vector, value_type* Value) \
{ \
if(Vector->Count == Vector->Capacity) \
{ \
value_type* Ptr = Allocate_Array(Vector->Allocator, value_type, Vector->Capacity*2); \
Memory_Copy(Ptr, Vector->Ptr, Vector->Capacity*sizeof(value_type)); \
Free_Array(Vector->Allocator, Vector->Ptr); \
Vector->Ptr = Ptr; \
Vector->Capacity *= 2; \
} \
Vector->Ptr[Vector->Count++] = *Value; \
} \
\
value_type Vector_Get_Value_##alias(type* Vector, uint64_t Index) \
{ \
Assert(Index < Vector->Count); \
return Vector->Ptr[Index]; \
} \
\
value_type* Vector_Get_Ptr_##alias(type* Vector, uint64_t Index) \
{ \
Assert(Index < Vector->Count); \
return Vector->Ptr + Index; \
} \
\
void Vector_Free_##alias(type* Vector) \
{ \
Free(Vector->Allocator, Vector->Ptr); \
Vector->Allocator = NULL; \
Vector->Count = 0; \
Vector->Capacity = 0; \
Vector->Ptr = NULL; \
} \
type G__Unused_Vector_Sentinal_##alias

Implement_Vector_Functions(u8_vector,  u8,  U8);
Implement_Vector_Functions(u16_vector, u16, U16);
Implement_Vector_Functions(u32_vector, u32, U32);
Implement_Vector_Functions(u64_vector, u64, U64);
Implement_Vector_Functions(s8_vector,  s8,  S8);
Implement_Vector_Functions(s16_vector, s16, S16);
Implement_Vector_Functions(s32_vector, s32, S32);
Implement_Vector_Functions(s64_vector, s64, S64);
Implement_Vector_Functions(f32_vector, f32, F32);
Implement_Vector_Functions(f64_vector, f64, F64);