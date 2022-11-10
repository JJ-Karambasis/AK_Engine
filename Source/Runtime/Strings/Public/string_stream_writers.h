#ifndef STRING_STREAM_WRITERS_H
#define STRING_STREAM_WRITERS_H

typedef struct utf8_stream_writer
{
    uint8_t* Str;
    uint64_t Capacity;
    uint64_t Size;
} utf8_stream_writer;

typedef struct utf16_stream_writer
{
    uint16_t* Str;
    uint64_t  Capacity;
    uint64_t  Size;
} utf16_stream_writer;

utf8_stream_writer UTF8_Begin_Stream_Writer(allocator* Allocator, uint64_t Capacity);
void               UTF8_Stream_Write(utf8_stream_writer* Stream, uint32_t Codepoint);
str8               UTF8_Stream_Writer_To_String(utf8_stream_writer* Writer, bool32_t NullTerminate);

utf16_stream_writer UTF16_Begin_Stream_Writer(allocator* Allocator, uint64_t Capacity);
void                UTF16_Stream_Write(utf16_stream_writer* Stream, uint32_t Codepoint);
str16               UTF16_Stream_Writer_To_String(utf16_stream_writer* Writer, bool32_t NullTerminate);

#endif
