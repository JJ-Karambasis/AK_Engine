#ifndef STRINGS_H
#define STRINGS_H

struct allocator;

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

#endif