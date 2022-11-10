#ifndef STRING_STREAM_READERS_H
#define STRING_STREAM_READERS_H

typedef struct utf8_stream_reader
{
    const uint8_t* Str;
    uint64_t       Size;
    uint64_t       Used;
} utf8_stream_reader;

typedef struct utf16_stream_reader
{
    const uint16_t* Str;
    uint64_t        Size;
    uint64_t        Used;
} utf16_stream_reader;

utf8_stream_reader UTF8_Begin_Stream_Reader_Ptr(const uint8_t* Str, uint64_t Length);
utf8_stream_reader UTF8_Begin_Stream_Reader(str8 Str);
bool8_t            UTF8_Stream_Reader_Is_Valid(utf8_stream_reader* Reader);
uint32_t           UTF8_Stream_Reader_Consume(utf8_stream_reader* Reader);

utf16_stream_reader UTF16_Begin_Stream_Reader_Ptr(const uint16_t* Str, uint64_t Length);
utf16_stream_reader UTF16_Begin_Stream_Reader(str16 Str);
bool8_t             UTF16_Stream_Reader_Is_Valid(utf16_stream_reader* Reader);
uint32_t            UTF16_Stream_Reader_Consume(utf16_stream_reader* Reader);

#endif