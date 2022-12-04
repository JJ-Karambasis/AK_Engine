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
    
    uint32_t Offset;
    UTF8_From_Codepoint(Codepoint, At, &Offset);
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