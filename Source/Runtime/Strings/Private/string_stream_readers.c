utf8_stream_reader UTF8_Begin_Stream_Reader_Ptr(const uint8_t* Str, uint64_t Length)
{
    utf8_stream_reader Reader;
    Zero_Struct(&Reader, utf8_stream_reader);
    Reader.Str = Str;
    Reader.Size = Length;
    return Reader;
}

utf8_stream_reader UTF8_Begin_Stream_Reader(str8 Str)
{
    return UTF8_Begin_Stream_Reader_Ptr(Str.Str, Str.Length);
}

bool8_t UTF8_Stream_Reader_Is_Valid(utf8_stream_reader* Reader)
{
    return Reader->Used < Reader->Size;
}

uint32_t UTF8_Stream_Reader_Consume(utf8_stream_reader* Reader)
{
    Assert(Reader->Used <= Reader->Size);
    uint32_t Length = 0;
    uint32_t Result = UTF8_Read(Reader->Str + Reader->Used, &Length);
    Reader->Used += Length;
    return Result;
}

utf16_stream_reader UTF16_Begin_Stream_Reader_Ptr(const uint16_t* Str, uint64_t Length)
{
    utf16_stream_reader Reader;
    Zero_Struct(&Reader, utf16_stream_reader);
    Reader.Str = Str;
    Reader.Size = Length;
    return Reader;
}

utf16_stream_reader UTF16_Begin_Stream_Reader(str16 Str)
{
    return UTF16_Begin_Stream_Reader_Ptr(Str.Str, Str.Length);
}

bool8_t UTF16_Stream_Reader_Is_Valid(utf16_stream_reader* Reader)
{
    return Reader->Used < Reader->Size;
}

uint32_t UTF16_Stream_Reader_Consume(utf16_stream_reader* Reader)
{
    const uint16_t* At = Reader->Str + Reader->Used;
    uint32_t Result = *At;
    
    Reader->Used++;
    if (0xD800 <= At[0] && At[0] < 0xDC00 && 0xDC00 <= At[1] && At[1] < 0xE000){
        Result = ((At[0] - 0xD800) << 10) | (At[1] - 0xDC00);
        Reader->Used++;
    }
    
    Assert(Reader->Used <= Reader->Size);
    return Result;
}