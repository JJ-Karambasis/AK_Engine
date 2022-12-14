#ifndef STRING_TYPES_H
#define STRING_TYPES_H

#define STR_INVALID_FIND ((uint64_t)-1)

typedef struct allocator allocator;

#define StrC_Expand(s) {(s), sizeof((s))-1}
#define StrC_Lit(s) StrC((s), sizeof((s))-1)
typedef struct strc
{
    const char* Str;
    uint64_t    Length;
} strc;

uint64_t StrC_Length(const char* Str);
bool8_t  StrC_Is_Empty(strc Str);
strc     StrC_Empty();
strc     StrC(const char* Str, uint64_t Length);
strc     StrC_Null_Term(const char* Str);
bool8_t  StrC_Equal(strc StrA, strc StrB);
int32_t  StrC_Compare(strc StrA, strc StrB);
strc     StrC_Concat(allocator* Allocator, strc StrA, strc StrB);
strc     StrC_FormatV(allocator* Allocator, strc Str, va_list Args);
strc     StrC_Format(allocator* Allocator, strc Str, ...);
strc     StrC_Copy(allocator* Allocator, strc Str);

#define Str8_Expand(s) {(const uint8_t*)(s), sizeof((s))-1}
#define Str8_Lit(s) Str8((const uint8_t*)(s), sizeof((s))-1)
typedef struct str8
{
    const uint8_t* Str;
    uint64_t       Length;
} str8;

uint64_t Str8_Length(const uint8_t* Str);
str8     Str8_Empty();
str8     Str8(const uint8_t* Str, uint64_t Length);
str8     Str8_Null_Term(const uint8_t* Str);
bool8_t  Str8_Equal(str8 StrA, str8 StrB);
bool8_t  Str8_Equal_Insensitive(str8 StrA, str8 StrB);
int32_t  Str8_Compare(str8 StrA, str8 StrB);
str8     Str8_Concat(allocator* Allocator, str8 StrA, str8 StrB);
str8     Str8_FormatV(allocator* Allocator, str8 Str, va_list Args);
str8     Str8_Format(allocator* Allocator, str8 Str, ...);
str8     Str8_Prefix(str8 Str, uint64_t Size);
str8     Str8_Postifx(str8 Str, uint64_t Size);
str8     Str8_Skip(str8 Str, uint64_t Size);
str8     Str8_Chop(str8 Str, uint64_t Size);
str8     Str8_Copy(allocator* Allocator, str8 Str);
uint64_t Str8_Find_First_Char(str8 Str, uint8_t Char);
uint64_t Str8_Find_Last_Char(str8 Str,  uint8_t Char);
bool8_t  Str8_Ends_With_Char(str8 Str, uint8_t Char);
uint64_t Str8_Find_First(str8 Str, str8 Pattern);
str8     Str8_To_Lower(allocator* Allocator, str8 Str);
str8     Str8_To_Upper(allocator* Allocator, str8 Str);
str8     Str8_Substr(str8 Str, uint64_t FirstIndex, uint64_t LastIndex);
str8     Str8_Replace(allocator* Allocator, str8 Str, str8 Pattern, str8 Replacement);

typedef struct str16
{
    const uint16_t* Str;
    uint64_t        Length;
} str16;

uint64_t Str16_Length(const uint16_t* Str);
str16    Str16(const uint16_t* Str, uint64_t Length);
str16    Str16_Null_Term(const uint16_t* Str);
bool8_t  Str16_Equal(str16 StrA, str16 StrB);
int32_t  Str16_Compare(str16 StrA, str16 StrB);
str16    Str16_Concat(allocator* Allocator, str16 StrA, str16 StrB);

typedef struct str32
{
    const uint32_t* Str;
    uint64_t        Length;
} str32;

uint64_t Str32_Length(const uint32_t* Str);
str32    Str32(const uint32_t* Str, uint64_t Length);
str32    Str32_Null_Term(const uint32_t* Str);
bool8_t  Str32_Equal(str32 StrA, str32 StrB);
int32_t  Str32_Compare(str32 StrA, str32 StrB);
str32    Str32_Concat(allocator* Allocator, str32 StrA, str32 StrB);

str8  Ascii_To_UTF8(allocator* Allocator, strc Str);
str16 Ascii_To_UTF16(allocator* Allocator, strc Str);
str32 Ascii_To_UTF32(allocator* Allocator, strc Str);

strc  UTF8_To_Ascii(allocator* Allocator, str8  Str);
str16 UTF8_To_UTF16(allocator* Allocator, str8 Str);
str32 UTF8_To_UTF32(allocator* Allocator, str8 Str);

strc  UTF16_To_Ascii(allocator* Allocator, str16  Str);
str8  UTF16_To_UTF8( allocator* Allocator, str16 Str);
str32 UTF16_To_UTF32(allocator* Allocator, str16 Str);

strc  UTF32_To_Ascii(allocator* Allocator, str32  Str);
str8  UTF32_To_UTF8( allocator* Allocator, str32 Str);
str16 UTF32_To_UTF16(allocator* Allocator, str32 Str);

uint32_t UTF8_Read(const uint8_t* At, uint32_t* Length);
uint32_t UTF8_Get_Byte_Count(uint32_t Codepoint);
void UTF8_From_Codepoint(uint32_t Codepoint, uint8_t* Buffer, uint32_t* Length);
uint32_t UTF16_Read(const uint16_t* At);

#endif