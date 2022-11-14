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
    
    StrC_List_Push_Node(&List, &NodeA);
    StrC_List_Push_Node(&List, &NodeB);
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

strc StrC_Copy(allocator* Allocator, strc Str)
{
    char* Buffer = Allocate_Array_No_Clear(Allocator,char, Str.Length+1);
    Memory_Copy(Buffer, Str.Str, Str.Length*sizeof(char));
    Buffer[Str.Length] = 0;
    return StrC(Buffer, Str.Length);
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
    
    Str8_List_Push_Node(&List, &NodeA);
    Str8_List_Push_Node(&List, &NodeB);
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

str8 Str8_Prefix(str8 Str, uint64_t Size)
{
    Size = Min(Size, Str.Length);
    return Str8(Str.Str, Size);
}

str8 Str8_Postifx(str8 Str, uint64_t Size)
{
    Size = Min(Size, Str.Length);
    return Str8(Str.Str+(Str.Length-Size), Size);
}

str8 Str8_Skip(str8 Str, uint64_t Size)
{
    Size = Min(Size, Str.Length);
    return Str8(Str.Str+Size, Str.Length-Size);
}

str8 Str8_Chop(str8 Str, uint64_t Size)
{
    Size = Min(Size, Str.Length);
    return Str8(Str.Str, Str.Length-Size);
}

str8 Str8_Copy(allocator* Allocator, str8 Str)
{
    uint8_t* Buffer = Allocate_Array_No_Clear(Allocator, uint8_t, Str.Length+1);
    Memory_Copy(Buffer, Str.Str, Str.Length*sizeof(uint8_t));
    Buffer[Str.Length] = 0;
    return Str8(Buffer, Str.Length);
}

uint64_t Str8_Find_First(str8 Str, uint8_t Char)
{
    for(uint64_t i = 0; i < Str.Length; i++)
        if(Str.Str[i] == Char) return i;
    return STR_INVALID_FIND;
}

uint64_t Str8_Find_Last(str8 Str,  uint8_t Char)
{
    for(uint64_t i = Str.Length-1; i != STR_INVALID_FIND; i--)
        if(Str.Str[i] == Char) return i;
    return STR_INVALID_FIND;
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
    
    Str16_List_Push_Node(&List, &NodeA);
    Str16_List_Push_Node(&List, &NodeB);
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
    
    Str32_List_Push_Node(&List, &NodeA);
    Str32_List_Push_Node(&List, &NodeB);
    return Str32_List_Join(Allocator, &List);
}


str8 Ascii_To_UTF8(allocator* Allocator, strc Str)
{
    str8 Result;
    Result.Length = Str.Length;
    uint8_t* Buffer = Allocate_Array_No_Clear(Allocator, uint8_t, Result.Length+1);
    Buffer[Result.Length] = 0;
    
    for(size_t i = 0; i < Str.Length; i++)
        Buffer[i] = ((uint8_t)Str.Str[i]) & BITMASK_7;
    
    Result.Str = Buffer;
    return Result;
}

str16 Ascii_To_UTF16(allocator* Allocator, strc Str)
{
    str16 Result;
    Result.Length = Str.Length;
    uint16_t* Buffer = Allocate_Array_No_Clear(Allocator, uint16_t, Result.Length+1);
    Buffer[Result.Length] = 0;
    
    for(size_t i = 0; i < Str.Length; i++)
        Buffer[i] = ((uint16_t)Str.Str[i]) & BITMASK_7;
    
    Result.Str = Buffer;
    return Result;
}

str32 Ascii_To_UTF32(allocator* Allocator, strc Str)
{
    str32 Result;
    Result.Length = Str.Length;
    uint32_t* Buffer = Allocate_Array_No_Clear(Allocator, uint32_t, Result.Length+1);
    Buffer[Result.Length] = 0;
    
    for(size_t i = 0; i < Str.Length; i++)
        Buffer[i] = ((uint32_t)Str.Str[i]) & BITMASK_7;
    
    Result.Str = Buffer;
    return Result;
}

strc  UTF8_To_Ascii(allocator* Allocator, str8 Str)
{
    strc Result;
    Result.Length = 0;
    char* Buffer = (char*)Allocate_Array_No_Clear(Allocator, char, Str.Length+1);
    
    utf8_stream_reader Reader = UTF8_Begin_Stream_Reader(Str);
    while(UTF8_Stream_Reader_Is_Valid(&Reader))
    {
        uint32_t Codepoint = UTF8_Stream_Reader_Consume(&Reader);
        Buffer[Result.Length++] = (Codepoint <= 127) ? (char)Codepoint : '?';
        Assert(Result.Length <= Str.Length);
    }
    
    Buffer[Result.Length] = 0;
    Result.Str = Buffer;
    return Result;
}

str16 UTF8_To_UTF16(allocator* Allocator, str8 Str)
{
    utf16_stream_writer Writer = UTF16_Begin_Stream_Writer(Allocator, Str.Length+1);
    utf8_stream_reader Reader = UTF8_Begin_Stream_Reader(Str);
    
    while(UTF8_Stream_Reader_Is_Valid(&Reader))
    {
        uint32_t Codepoint = UTF8_Stream_Reader_Consume(&Reader);
        UTF16_Stream_Write(&Writer, Codepoint);
    }
    
    str16 Result = UTF16_Stream_Writer_To_String(&Writer, true);
    return Result;
}

str32 UTF8_To_UTF32(allocator* Allocator, str8 Str)
{
    str32 Result;
    Result.Length = 0;
    uint32_t* Buffer = Allocate_Array_No_Clear(Allocator, uint32_t, Str.Length+1);
    
    utf8_stream_reader Reader = UTF8_Begin_Stream_Reader(Str);
    while(UTF8_Stream_Reader_Is_Valid(&Reader))
    {
        uint32_t Codepoint = UTF8_Stream_Reader_Consume(&Reader);
        Buffer[Result.Length++] = (Codepoint == 0xFFFFFFFF) ? '?' : Codepoint;
        Assert(Result.Length <= Str.Length);
    }
    
    Buffer[Result.Length] = 0;
    Result.Str = Buffer;
    return Result;
}

strc UTF16_To_Ascii(allocator* Allocator, str16 Str)
{
    utf16_stream_reader Reader = UTF16_Begin_Stream_Reader(Str);
    
    strc Result;
    Result.Length = 0;
    char* Buffer = Allocate_Array_No_Clear(Allocator, char, Str.Length+1);
    
    while(UTF16_Stream_Reader_Is_Valid(&Reader))
    {
        uint32_t Codepoint = UTF16_Stream_Reader_Consume(&Reader);
        Buffer[Result.Length++] = (Codepoint <= 127) ? (char)Codepoint : '?';
        Assert(Result.Length <= Str.Length);
    }
    
    Buffer[Result.Length] = 0;
    Result.Str = Buffer;
    return Result;
}

str8 UTF16_To_UTF8(allocator* Allocator, str16 Str)
{
    utf16_stream_reader Reader = UTF16_Begin_Stream_Reader(Str);
    utf8_stream_writer Writer = UTF8_Begin_Stream_Writer(Allocator, (Str.Length*3)+1);
    
    while(UTF16_Stream_Reader_Is_Valid(&Reader))
    {
        uint32_t Codepoint = UTF16_Stream_Reader_Consume(&Reader);
        UTF8_Stream_Write(&Writer, Codepoint);
    }
    
    str8 Result = UTF8_Stream_Writer_To_String(&Writer, true);
    return Result;
}

str32 UTF16_To_UTF32(allocator* Allocator, str16 Str)
{
    str32 Result;
    Result.Length = 0;
    uint32_t* Buffer = (uint32_t*)Allocate_Array_No_Clear(Allocator, uint32_t, Str.Length+1);
    
    utf16_stream_reader Reader = UTF16_Begin_Stream_Reader(Str);
    while(UTF16_Stream_Reader_Is_Valid(&Reader))
    {
        uint32_t Codepoint = UTF16_Stream_Reader_Consume(&Reader);
        Buffer[Result.Length++] = Codepoint;
        Assert(Result.Length <= Str.Length);
    }
    
    Buffer[Result.Length] = 0;
    Result.Str = Buffer;
    return Result;
}

strc UTF32_To_Ascii(allocator* Allocator, str32 Str)
{
    strc Result;
    Result.Length = 0;
    char* Buffer = (char*)Allocate_Array_No_Clear(Allocator, char, Str.Length+1);
    
    for(size_t i = 0; i < Str.Length; i++)
    {
        uint32_t Codepoint = Str.Str[i];
        Buffer[Result.Length++] = (Codepoint <= 127) ? (char)Codepoint : '?';
    }
    
    Buffer[Result.Length] = 0;
    Result.Str = Buffer;
    return Result;
}

str8 UTF32_To_UTF8(allocator* Allocator, str32 Str)
{
    utf8_stream_writer Writer = UTF8_Begin_Stream_Writer(Allocator, (Str.Length*4)+1);
    
    for(size_t i = 0; i < Str.Length; i++)
    {
        uint32_t Codepoint = Str.Str[i];
        UTF8_Stream_Write(&Writer, Codepoint);
    }
    
    str8 Result = UTF8_Stream_Writer_To_String(&Writer, true);
    return Result;
}

str16 UTF32_To_UTF16(allocator* Allocator, str32 Str)
{
    utf16_stream_writer Writer = UTF16_Begin_Stream_Writer(Allocator, (Str.Length*2)+1);
    
    for(size_t i = 0; i < Str.Length; i++)
    {
        uint32_t Codepoint = Str.Str[i];
        UTF16_Stream_Write(&Writer, Codepoint);
    }
    
    str16 Result = UTF16_Stream_Writer_To_String(&Writer, true);
    return Result;
}