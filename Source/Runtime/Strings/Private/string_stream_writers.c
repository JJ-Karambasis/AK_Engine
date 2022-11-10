utf8_stream_writer UTF8_Begin_Stream_Writer(allocator* Allocator, uint64_t Capacity)
{
    utf8_stream_writer Result;
    Zero_Struct(&Result, utf8_stream_writer);
    Result.Str = Allocate_Array(Allocator, uint8_t, Capacity);
    Result.Capacity = Capacity;
    return Result;
}

void UTF8_Stream_Write(utf8_stream_writer* Stream, uint32_t Codepoint)
{
    uint8_t* At = Stream->Str + Stream->Size;
    
    uint32_t Offset = 0;
    if (Codepoint <= 0x7F)
    {
        At[0] = (uint8_t)Codepoint;
        Offset = 1;
    }
    else if (Codepoint <= 0x7FF)
    {
        At[0] = (BITMASK_2 << 6) | ((Codepoint >> 6) & BITMASK_5);
        At[1] = BIT_8 | (Codepoint & BITMASK_6);
        Offset = 2;
    }
    else if (Codepoint <= 0xFFFF)
    {
        At[0] = (BITMASK_3 << 5) | ((Codepoint >> 12) & BITMASK_4);
        At[1] = BIT_8 | ((Codepoint >> 6) & BITMASK_6);
        At[2] = BIT_8 | ( Codepoint       & BITMASK_6);
        Offset = 3;
    }
    else if (Codepoint <= 0x10FFFF)
    {
        At[0] = (BITMASK_4 << 3) | ((Codepoint >> 18) & BITMASK_3);
        At[1] = BIT_8 | ((Codepoint >> 12) & BITMASK_6);
        At[2] = BIT_8 | ((Codepoint >>  6) & BITMASK_6);
        At[3] = BIT_8 | ( Codepoint        & BITMASK_6);
        Offset = 4;
    }
    else
    {
        Assert(false);
        At[0] = '?';
        Offset = 1;
    }
    
    Stream->Size += Offset;
    Assert(Stream->Size <= Stream->Capacity);
}

str8 UTF8_Stream_Writer_To_String(utf8_stream_writer* Writer, bool32_t NullTerminate)
{
    if(NullTerminate) 
    {
        Assert(Writer->Size+1 <= Writer->Capacity);
        Writer->Str[Writer->Size] = 0;
    }
    return Str8(Writer->Str, Writer->Size);
}

utf16_stream_writer UTF16_Begin_Stream_Writer(allocator* Allocator, uint64_t Capacity)
{
    utf16_stream_writer Result;
    Zero_Struct(&Result, utf16_stream_writer);
    Result.Str = Allocate_Array(Allocator, uint16_t, Capacity);
    Result.Capacity = Capacity;
    return Result;
}

void UTF16_Stream_Write(utf16_stream_writer* Stream, uint32_t Codepoint)
{
    Assert(Codepoint != 0xFFFFFFFF);
    
    uint16_t* At = Stream->Str + Stream->Size;
    if(Codepoint == 0xFFFFFFFF)
    {
        Assert(false);
        At[0] = (uint16_t)'?';
    }
    else if(Codepoint < 0x10000)
    {
        At[0] = (uint16_t)Codepoint;
    }
    else
    {
        Codepoint -= 0x10000;
        At[0] = 0xD800 + (uint16_t)(Codepoint >> 10);
        At[1] = 0xDC00 + (Codepoint & BITMASK_10);
        Stream->Size++;
    }
    Stream->Size++;
    Assert(Stream->Size <= Stream->Capacity);
}

str16 UTF16_Stream_Writer_To_String(utf16_stream_writer* Writer, bool32_t NullTerminate)
{
    if(NullTerminate) 
    {
        Assert(Writer->Size+1 <= Writer->Capacity);
        Writer->Str[Writer->Size] = 0;
    }
    return Str16(Writer->Str, Writer->Size);
}