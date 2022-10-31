#ifndef STRING_TYPES_H
#define STRING_TYPES_H

#define StrC_Lit(s) StrC((s), sizeof((s))-1)
typedef struct strc
{
    const char* Str;
    uint64_t    Length;
} strc;

uint64_t StrC_Length(const char* Str);
strc     StrC(const char* Str, uint64_t Length);
strc     StrC_Null_Term(const char* Str);
bool8_t  StrC_Equal(strc StrA, strc StrB);
int32_t  StrC_Compare(strc StrA, strc StrB);
strc     StrC_Concat(allocator* Allocator, strc StrA, strc StrB);
strc     StrC_FormatV(allocator* Allocator, strc Str, va_list Args);
strc     StrC_Format(allocator* Allocator, strc Str, ...);

#define Str8_Lit(s) Str8((const uint8_t*)(s), sizeof((s))-1)
typedef struct str8
{
    const uint8_t* Str;
    uint64_t       Length;
} str8;

uint64_t Str8_Length(const uint8_t* Str);
str8     Str8(const uint8_t* Str, uint64_t Length);
str8     Str8_Null_Term(const uint8_t* Str);
bool8_t  Str8_Equal(str8 StrA, str8 StrB);
int32_t  Str8_Compare(str8 StrA, str8 StrB);
str8     Str8_Concat(allocator* Allocator, str8 StrA, str8 StrB);
str8     Str8_FormatV(allocator* Allocator, str8 Str, va_list Args);
str8     Str8_Format(allocator* Allocator, str8 Str, ...);

#endif