#ifndef STRINGS_H
#define STRINGS_H

#define STR_INVALID_FIND ((uptr)-1)

struct allocator;

template <typename type>
struct span;

u32 UTF8_Read(const char* Str, u32* OutLength);
u32 UTF8_Write(char* Str, u32 Codepoint);
u32 UTF16_Read(const wchar_t* Str, u32* OutLength);
u32 UTF16_Write(wchar_t* Str, u32 Codepoint);

enum class str_case {
    Sensitive,
    Insensitive
};

#define String_Lit(str) string(str, sizeof(str)-1)

struct wstring;
struct string {
    const char* Str  = nullptr;
    uptr        Size = 0;
    string() = default;
    string(const char* Str);
    string(const char* Str, uptr Size);
    string(const char* StrBegin, const char* StrEnd);
    string(allocator* Allocator, string Str);
    string(allocator* Allocator, wstring Str);
    string(allocator* Allocator, const char* Format, va_list Args);
    string(allocator* Allocator, const char* Format, ...);
    const char& operator[](uptr Index) const;
};

bool operator==(string A, string B);
bool String_Equals(string A, string B, str_case Case = str_case::Sensitive);

string String_Concat(allocator* Allocator, string StringA, string StringB);
string String_Concat(allocator* Allocator, const span<string>& Strings);
string String_Substr(string String, uptr FirstIndex, uptr LastIndex);
uptr   String_Find_Last(string String, char Character);
bool   String_Begins_With(string String, string Substr);
uptr   String_Find(string String, string Substring);
string String_Get_Path(string String);

string String_To_Date_Format(allocator* Allocator, u32 Value);
string String_To_Millisecond_Format(allocator* Allocator, u32 Value);
void   String_Free(allocator* Allocator, string String);

#define WString_Lit(str) wstring(L ## str, (sizeof(L##str)-1)/sizeof(wchar_t))
struct wstring {
    const wchar_t* Str  = nullptr;
    uptr           Size = 0;

    wstring() = default;
    wstring(const wchar_t* Str);
    wstring(const wchar_t* Str, uptr Size);
    wstring(allocator* Allocator, wstring Str);
    wstring(allocator* Allocator, string Str);
    const wchar_t& operator[](uptr Index) const;
};

bool operator==(wstring A, wstring B);
bool WString_Equals(wstring A, wstring B, str_case Case = str_case::Sensitive);

wstring WString_Concat(allocator* Allocator, wstring StringA, wstring StringB);
wstring WString_Concat(allocator* Allocator, const span<wstring>& Strings);
wstring WString_Substr(wstring String, uptr FirstIndex, uptr LastIndex);
uptr    WString_Find_Last(wstring String, wchar_t Character);
wstring WString_Get_Filename_Without_Ext(wstring String);
void    WString_Free(allocator* Allocator, wstring String);

#endif