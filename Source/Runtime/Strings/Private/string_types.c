#define STB_SPRINTF_IMPLEMENTATION
#include <stb/stb_sprintf.h>

uint64_t StrC_Length(const char* Str)
{
    uint64_t Result = 0;
    while(*Str++) Result++;
    return Result;
}

bool8_t StrC_Is_Empty(strc Str)
{
    return !Str.Str || !Str.Length;
}

strc StrC_Empty()
{
    strc Result;
    Result.Str = NULL;
    Result.Length = 0;
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

str8 Str8_Empty()
{
    str8 Result;
    Result.Str = NULL;
    Result.Length = 0;
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
    {
        if(StrA.Str[i] != StrB.Str[i]) 
            return false;
    }
    return true;
}

bool8_t Str8_Equal_Insensitive(str8 StrA, str8 StrB)
{
    if(StrA.Length != StrB.Length) return false;
    for(uint64_t i = 0; i < StrA.Length; i++) 
    {
        if(To_Lower8(StrA.Str[i]) != To_Lower8(StrB.Str[i])) 
            return false;
    }
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

uint64_t Str8_Find_First_Char(str8 Str, uint8_t Char)
{
    for(uint64_t i = 0; i < Str.Length; i++)
        if(Str.Str[i] == Char) return i;
    return STR_INVALID_FIND;
}

uint64_t Str8_Find_Last_Char(str8 Str,  uint8_t Char)
{
    for(uint64_t i = Str.Length-1; i != STR_INVALID_FIND; i--)
        if(Str.Str[i] == Char) return i;
    return STR_INVALID_FIND;
}

bool8_t Str8_Ends_With_Char(str8 Str, uint8_t Char)
{
    if(!Str.Length) return false;
    return Str.Str[Str.Length-1] == Char;
}

uint64_t Str8_Find_First(str8 Str, str8 Pattern)
{
    if(Pattern.Length > 0)
    {
        if(Str.Length >= Pattern.Length)
        {
            uint64_t i = 0;
            uint8_t C = Pattern.Str[0];
            uint64_t OnePastLast = Str.Length-Pattern.Length+1;
            for(; i < OnePastLast; i++)
            {
                if(Str.Str[i] == C)
                {
                    str8 Source = Str8_Prefix(Str8_Skip(Str, i), Pattern.Length);
                    if(Str8_Equal(Source, Pattern))
                        return i;
                }
            }
        }
    }
    return STR_INVALID_FIND;
}

str8 Str8_To_Lower(allocator* Allocator, str8 Str)
{
    uint8_t* Buffer = Allocate_Array_No_Clear(Allocator, uint8_t, Str.Length+1);
    for(uint64_t i = 0; i < Str.Length; i++)
        Buffer[i] = To_Lower8(Str.Str[i]);
    Buffer[Str.Length] = 0;
    return Str8(Buffer, Str.Length);
}

str8 Str8_To_Upper(allocator* Allocator, str8 Str)
{
    uint8_t* Buffer = Allocate_Array_No_Clear(Allocator, uint8_t, Str.Length+1);
    for(uint64_t i = 0; i < Str.Length; i++)
        Buffer[i] = To_Upper8(Str.Str[i]);
    Buffer[Str.Length] = 0;
    return Str8(Buffer, Str.Length);
}

str8 Str8_Substr(str8 Str, uint64_t FirstIndex, uint64_t LastIndex)
{
    Assert(FirstIndex < Str.Length);
    Assert(LastIndex <= Str.Length);
    Assert(FirstIndex < LastIndex);
    return Str8(Str.Str+FirstIndex, LastIndex-FirstIndex);
}

str8 Str8_Replace(allocator* Allocator, str8 Str, str8 Pattern, str8 Replacement)
{
    str8_list List;
    Zero_Struct(&List, str8_list);
    
    str8 Source = Str;
    for(;;)
    {
        uint64_t i = Str8_Find_First(Source, Pattern);
        if(i != STR_INVALID_FIND)
        {
            Str8_List_Push(&List, Allocator, Str8_Prefix(Source, i));
            if(i < Source.Length)
            {
                Str8_List_Push(&List, Allocator, Replacement);
                Source = Str8_Skip(Source, i + Pattern.Length);
            }
            else break;
        }
        else break;
    }
    
    if(!List.Count) return Str;
    return Str8_List_Join(Allocator, &List);
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

static const uint8_t G_ClassUTF8[32] = 
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};

uint32_t UTF8_Read(const uint8_t* At, uint32_t* Length)
{
    uint32_t Result = 0xFFFFFFFF;
    
    uint8_t Byte = At[0];
    uint8_t ByteClass = G_ClassUTF8[Byte >> 3];
    
    switch(ByteClass)
    {
        case 1:
        {
            Result = Byte;
        } break;
        
        case 2:
        {
            uint8_t NextByte = At[1];
            if(G_ClassUTF8[NextByte >> 3] == 0)
            {
                Result = (Byte & BITMASK_5) << 6;
                Result |= (NextByte & BITMASK_6);
            }
        } break;
        
        case 3:
        {
            uint8_t NextBytes[2] = {At[1], At[2]};
            if(G_ClassUTF8[NextBytes[0] >> 3] == 0 &&
               G_ClassUTF8[NextBytes[1] >> 3] == 0)
            {
                Result = (Byte & BITMASK_4) << 12;
                Result |= ((NextBytes[0] & BITMASK_6) << 6);
                Result |= (NextBytes[1] & BITMASK_6);
            }
        } break;
        
        case 4:
        {
            uint32_t NextBytes[3] = {At[1], At[2], At[3]};
            if(G_ClassUTF8[NextBytes[0] >> 3] == 0 &&
               G_ClassUTF8[NextBytes[1] >> 3] == 0 &&
               G_ClassUTF8[NextBytes[2] >> 3] == 0)
            {
                Result = (Byte & BITMASK_3) << 18;
                Result |= ((NextBytes[0] & BITMASK_6) << 12);
                Result |= ((NextBytes[1] & BITMASK_6) << 6);
                Result |= (NextBytes[2] & BITMASK_6);
            }
        } break;
    }
    
    if(Length) *Length = ByteClass;
    return Result;
}

uint32_t UTF8_Get_Byte_Count(uint32_t Codepoint)
{
    uint32_t Result = 0;
    if (Codepoint <= 0x7F)
    {
        Result = 1;
    }
    else if (Codepoint <= 0x7FF)
    {
        Result = 2;
    }
    else if (Codepoint <= 0xFFFF)
    {
        Result = 3;
    }
    else if (Codepoint <= 0x10FFFF)
    {
        Result = 4;
    }
    else
    {
        Assert(false);
    }
    return Result;
}

void UTF8_From_Codepoint(uint32_t Codepoint, uint8_t* Buffer, uint32_t* Length)
{
    uint32_t Offset = 0;
    if (Codepoint <= 0x7F)
    {
        Buffer[0] = (uint8_t)Codepoint;
        Offset = 1;
    }
    else if (Codepoint <= 0x7FF)
    {
        Buffer[0] = (BITMASK_2 << 6) | ((Codepoint >> 6) & BITMASK_5);
        Buffer[1] = BIT_8 | (Codepoint & BITMASK_6);
        Offset = 2;
    }
    else if (Codepoint <= 0xFFFF)
    {
        Buffer[0] = (BITMASK_3 << 5) | ((Codepoint >> 12) & BITMASK_4);
        Buffer[1] = BIT_8 | ((Codepoint >> 6) & BITMASK_6);
        Buffer[2] = BIT_8 | ( Codepoint       & BITMASK_6);
        Offset = 3;
    }
    else if (Codepoint <= 0x10FFFF)
    {
        Buffer[0] = (BITMASK_4 << 3) | ((Codepoint >> 18) & BITMASK_3);
        Buffer[1] = BIT_8 | ((Codepoint >> 12) & BITMASK_6);
        Buffer[2] = BIT_8 | ((Codepoint >>  6) & BITMASK_6);
        Buffer[3] = BIT_8 | ( Codepoint        & BITMASK_6);
        Offset = 4;
    }
    else
    {
        Assert(false);
        Buffer[0] = '?';
        Offset = 1;
    }
    
    if(Length) *Length = Offset;
}

uint32_t UTF16_Read(const uint16_t* At)
{
    uint32_t Result = *At;
    
    if (0xD800 <= At[0] && At[0] < 0xDC00 && 0xDC00 <= At[1] && At[1] < 0xE000){
        Result = ((At[0] - 0xD800) << 10) | (At[1] - 0xDC00);
    }
    return Result;
}