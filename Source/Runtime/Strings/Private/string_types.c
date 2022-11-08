#define STB_SPRINTF_IMPLEMENTATION
#include <stb/stb_sprintf.h>

uint64_t StrC_Length(const char* Str)
{
    uint64_t Result = 0;
    while(*Str++) Result++;
    return Result;
}

strc StrC(const char* Str, uint64_t Length)
{
    strc Result;
    Result.Str    = Str;
    Result.Length = Length;
    return Result;
}

strc StrC_Null_Term(const char* Str)
{
    return StrC(Str, StrC_Length(Str));
}

bool8_t  StrC_Equal(strc StrA, strc StrB)
{
    if(StrA.Length != StrB.Length) return false;
    for(uint64_t i = 0; i < StrA.Length; i++)
        if(StrA.Str[i] != StrB.Str[i]) return false;
    return true;
}

int32_t StrC_Compare(strc StrA, strc StrB)
{
    int32_t Result = 0;
    for(uint64_t i = 0; i < StrA.Length || i < StrB.Length; i++)
    {
        char ca = (i < StrA.Length) ? StrA.Str[i] : 0;
        char cb = (i < StrB.Length) ? StrB.Str[i] : 0;
        int32_t Diff = (int32_t)(ca)-(int32_t)(cb);
        if(Diff != 0)
        {
            Result = Diff > 0 ? 1 : -1;
            break;
        }
    }
    return Result;
}

strc StrC_Concat(allocator* Allocator, strc StrA, strc StrB)
{
    strc_list List;
    Zero_Struct(&List, strc_list);
    
    strc_node NodeA;
    strc_node NodeB;
    Zero_Struct(&NodeA, strc_node);
    Zero_Struct(&NodeB, strc_node);
    NodeA.Str = StrA;
    NodeB.Str = StrB;
    
    StrC_List_Push_Node(&NodeA);
    StrC_List_Push_Node(&NodeB);
    return StrC_List_Join(Allocator, &List);
}

strc StrC_FormatV(allocator* Allocator, strc Format, va_list Args)
{
    local char TempBuffer[1];
    uint32_t Length = stbsp_vsnprintf(TempBuffer, 1, Format.Str, Args);
    char* Buffer = (char*)Allocate_Array_No_Clear(Allocator, char, Length+1);
    stbsp_vsnprintf(Buffer, sizeof(char)*(Length+1), Format.Str, Args);
    return StrC(Buffer, Length);
}

strc StrC_Format(allocator* Allocator, strc Str, ...)
{
    va_list List;
    va_start(List, Str.Str);
    strc Result = StrC_FormatV(Allocator, Str, List);
    va_end(List);
    return Result;
}

uint64_t Str8_Length(const uint8_t* Str)
{
    uint64_t Result = 0;
    while(*Str++) Result++;
    return Result;
}

str8  Str8(const uint8_t* Str, uint64_t Length)
{
    str8 Result;
    Result.Str    = Str;
    Result.Length = Length;
    return Result;
}

str8 Str8_Null_Term(const uint8_t* Str)
{
    return Str8(Str, Str8_Length(Str));
}

bool8_t Str8_Equal(str8 StrA, str8 StrB)
{
    if(StrA.Length != StrB.Length) return false;
    for(uint64_t i = 0; i < StrA.Length; i++)
        if(StrA.Str[i] != StrB.Str[i]) return false;
    return true;
}

int32_t Str8_Compare(str8 StrA, str8 StrB)
{
    int32_t Result = 0;
    for(uint64_t i = 0; i < StrA.Length || i < StrB.Length; i++)
    {
        uint8_t ca = (i < StrA.Length) ? StrA.Str[i] : 0;
        uint8_t cb = (i < StrB.Length) ? StrB.Str[i] : 0;
        int32_t Diff = (int32_t)(ca)-(int32_t)(cb);
        if(Diff != 0)
        {
            Result = Diff > 0 ? 1 : -1;
            break;
        }
    }
    return Result;
}

str8 Str8_Concat(allocator* Allocator, str8 StrA, str8 StrB)
{
    str8_list List;
    Zero_Struct(&List, str8_list);
    
    str8_node NodeA;
    str8_node NodeB;
    Zero_Struct(&NodeA, str8_node);
    Zero_Struct(&NodeB, str8_node);
    NodeA.Str = StrA;
    NodeB.Str = StrB;
    
    Str8_List_Push_Node(&NodeA);
    Str8_List_Push_Node(&NodeB);
    return Str8_List_Join(Allocator, &List);
}

str8 Str8_FormatV(allocator* Allocator, str8 Str, va_list Args)
{
    local char TempBuffer[1];
    uint32_t Length = stbsp_vsnprintf(TempBuffer, 1, (const char*)Str.Str, Args);
    uint8_t* Buffer = (uint8_t*)Allocate_Array_No_Clear(Allocator, uint8_t, Length+1);
    stbsp_vsnprintf((char*)Buffer, sizeof(uint8_t)*(Length+1), (const char*)Str.Str, Args);
    return Str8(Buffer, Length);
}

str8 Str8_Format(allocator* Allocator, str8 Str, ...)
{
    va_list List;
    va_start(List, Str.Str);
    str8 Result = Str8_FormatV(Allocator, Str, List);
    va_end(List);
    return Result;
}

uint64_t Str16_Length(const uint16_t* Str)
{
    uint64_t Result = 0;
    while(*Str++) Result++;
    return Result;
}

str16 Str16(const uint16_t* Str, uint64_t Length)
{
    str16 Result;
    Result.Str    = Str;
    Result.Length = Length;
    return Result;
}

str16 Str16_Null_Term(const uint16_t* Str)
{
    return Str16(Str, Str16_Length(Str));
}

bool8_t Str16_Equal(str16 StrA, str16 StrB)
{
    if(StrA.Length != StrB.Length) return false;
    for(uint64_t i = 0; i < StrA.Length; i++)
        if(StrA.Str[i] != StrB.Str[i]) return false;
    return true;
}

int32_t Str16_Compare(str16 StrA, str16 StrB)
{
    int32_t Result = 0;
    for(uint64_t i = 0; i < StrA.Length || i < StrB.Length; i++)
    {
        uint16_t ca = (i < StrA.Length) ? StrA.Str[i] : 0;
        uint16_t cb = (i < StrB.Length) ? StrB.Str[i] : 0;
        int32_t Diff = (int32_t)(ca)-(int32_t)(cb);
        if(Diff != 0)
        {
            Result = Diff > 0 ? 1 : -1;
            break;
        }
    }
    return Result;
}

str16 Str16_Concat(allocator* Allocator, str16 StrA, str16 StrB)
{
    str16_list List;
    Zero_Struct(&List, str16_list);
    
    str16_node NodeA;
    str16_node NodeB;
    Zero_Struct(&NodeA, str16_node);
    Zero_Struct(&NodeB, str16_node);
    NodeA.Str = StrA;
    NodeB.Str = StrB;
    
    Str16_List_Push_Node(&NodeA);
    Str16_List_Push_Node(&NodeB);
    return Str16_List_Join(Allocator, &List);
}

uint64_t Str32_Length(const uint32_t* Str)
{
    uint64_t Result = 0;
    while(*Str++) Result++;
    return Result;
}

str32 Str32(const uint32_t* Str, uint64_t Length)
{
    str32 Result;
    Result.Str    = Str;
    Result.Length = Length;
    return Result;
}

str32 Str32_Null_Term(const uint32_t* Str)
{
    return Str32(Str, Str32_Length(Str));
}

bool8_t Str32_Equal(str32 StrA, str32 StrB)
{
    if(StrA.Length != StrB.Length) return false;
    for(uint64_t i = 0; i < StrA.Length; i++)
        if(StrA.Str[i] != StrB.Str[i]) return false;
    return true;
}

int32_t Str32_Compare(str32 StrA, str32 StrB)
{
    int32_t Result = 0;
    for(uint64_t i = 0; i < StrA.Length || i < StrB.Length; i++)
    {
        uint32_t ca = (i < StrA.Length) ? StrA.Str[i] : 0;
        uint32_t cb = (i < StrB.Length) ? StrB.Str[i] : 0;
        int32_t Diff = (int32_t)(ca)-(int32_t)(cb);
        if(Diff != 0)
        {
            Result = Diff > 0 ? 1 : -1;
            break;
        }
    }
    return Result;
}

str32 Str32_Concat(allocator* Allocator, str32 StrA, str32 StrB)
{
    str32_list List;
    Zero_Struct(&List, str32_list);
    
    str32_node NodeA;
    str32_node NodeB;
    Zero_Struct(&NodeA, str32_node);
    Zero_Struct(&NodeB, str32_node);
    NodeA.Str = StrA;
    NodeB.Str = StrB;
    
    Str32_List_Push_Node(&NodeA);
    Str32_List_Push_Node(&NodeB);
    return Str32_List_Join(Allocator, &List);
}