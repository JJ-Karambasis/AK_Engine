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

static const uint8_t G_ClassUTF8[32] = 
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};

uint32_t UTF8_Stream_Reader_Consume(utf8_stream_reader* Reader)
{
    uint32_t Result = 0xFFFFFFFF;
    
    uint8_t Byte = Reader->Str[Reader->Used];
    uint8_t ByteClass = G_ClassUTF8[Byte >> 3];
    Assert((Reader->Used+ByteClass) <= Reader->Size);
    
    switch(ByteClass)
    {
        case 1:
        {
            Result = Byte;
        } break;
        
        case 2:
        {
            uint8_t NextByte = Reader->Str[Reader->Used+1];
            if(G_ClassUTF8[NextByte >> 3] == 0)
            {
                Result = (Byte & BITMASK_5) << 6;
                Result |= (NextByte & BITMASK_6);
            }
        } break;
        
        case 3:
        {
            uint8_t NextBytes[2] = {Reader->Str[Reader->Used+1], Reader->Str[Reader->Used+2]};
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
            uint32_t NextBytes[3] = {Reader->Str[Reader->Used+1], Reader->Str[Reader->Used+2], Reader->Str[Reader->Used+3]};
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
    
    Reader->Used += ByteClass;
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