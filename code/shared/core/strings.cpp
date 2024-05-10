global const u8 G_ClassUTF8[32] = 
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};

u32 UTF8_Read(const char* Str, u32* OutLength) {
    u32 Result = 0xFFFFFFFF;
    
    u8 Byte = (u8)Str[0];
    u8 ByteClass = G_ClassUTF8[Byte >> 3];
    
    local_persist const u32 BITMASK_3 = 0x00000007;
    local_persist const u32 BITMASK_4 = 0x0000000f;
    local_persist const u32 BITMASK_5 = 0x0000001f;
    local_persist const u32 BITMASK_6 = 0x0000003f;
    
    switch(ByteClass)
    {
        case 1:
        {
            Result = Byte;
        } break;
        
        case 2:
        {
            u8 NextByte = (u8)Str[1];
            if(G_ClassUTF8[NextByte >> 3] == 0)
            {
                Result = (Byte & BITMASK_5) << 6;
                Result |= (NextByte & BITMASK_6);
            }
        } break;
        
        case 3:
        {
            u8 NextBytes[2] = {(u8)Str[1], (u8)Str[2]};
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
            u8 NextBytes[3] = {(u8)Str[1], (u8)Str[2], (u8)Str[3]};
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
    
    if(OutLength) *OutLength = ByteClass;
    return Result;
}

u32 UTF8_Write(char* Str, u32 Codepoint) {
    local_persist const u32 BIT_8 = 0x00000080;
    local_persist const u32 BITMASK_2 = 0x00000003;
    local_persist const u32 BITMASK_3 = 0x00000007;
    local_persist const u32 BITMASK_4 = 0x0000000f;
    local_persist const u32 BITMASK_5 = 0x0000001f;
    local_persist const u32 BITMASK_6 = 0x0000003f;
    
    u32 Result = 0;
    if (Codepoint <= 0x7F)
    {
        Str[0] = (char)Codepoint;
        Result = 1;
    }
    else if (Codepoint <= 0x7FF)
    {
        Str[0] = (char)((BITMASK_2 << 6) | ((Codepoint >> 6) & BITMASK_5));
        Str[1] = (char)(BIT_8 | (Codepoint & BITMASK_6));
        Result = 2;
    }
    else if (Codepoint <= 0xFFFF)
    {
        Str[0] = (char)((BITMASK_3 << 5) | ((Codepoint >> 12) & BITMASK_4));
        Str[1] = (char)(BIT_8 | ((Codepoint >> 6) & BITMASK_6));
        Str[2] = (char)(BIT_8 | ( Codepoint       & BITMASK_6));
        Result = 3;
    }
    else if (Codepoint <= 0x10FFFF)
    {
        Str[0] = (char)((BITMASK_4 << 3) | ((Codepoint >> 18) & BITMASK_3));
        Str[1] = (char)(BIT_8 | ((Codepoint >> 12) & BITMASK_6));
        Str[2] = (char)(BIT_8 | ((Codepoint >>  6) & BITMASK_6));
        Str[3] = (char)(BIT_8 | ( Codepoint        & BITMASK_6));
        Result = 4;
    }
    else
    {
        Str[0] = '?';
        Result = 1;
    }
    
    return Result;
}

u32 UTF16_Read(const wchar_t* Str, u32* OutLength) {
    u32 Offset = 1;
    u32 Result = *Str;
    if (0xD800 <= Str[0] && Str[0] < 0xDC00 && 0xDC00 <= Str[1] && Str[1] < 0xE000)
    {
        Result = (u32)(((Str[0] - 0xD800) << 10) | (Str[1] - 0xDC00));
        Offset = 2;
    }
    if(OutLength) *OutLength = Offset;
    return Result;
}

u32 UTF16_Write(wchar_t* Str, u32 Codepoint) {
    local_persist const u32 BITMASK_10 = 0x000003ff;
    
    u32 Result = 0;
    if(Codepoint == 0xFFFFFFFF)
    {
        Str[0] = (wchar_t)'?';
        Result = 1;
    }
    else if(Codepoint < 0x10000)
    {
        Str[0] = (wchar_t)Codepoint;
        Result = 1;
    }
    else
    {
        Codepoint -= 0x10000;
        Str[0] = (wchar_t)(0xD800 + (Codepoint >> 10));
        Str[1] = (wchar_t)(0xDC00 + (Codepoint & BITMASK_10));
        Result = 2;
    }
    
    return Result;
}

string::string(const char* _Str) : Str(_Str), Size(strlen(_Str)) { }

string::string(const char* _Str, uptr _Size) : Str(_Str), Size(_Size) { }

string::string(const char* StrBegin, const char* StrEnd) : Str(StrBegin), Size((uptr)(StrEnd-StrBegin)) { }

string::string(allocator* Allocator, string String) {
    char* Buffer = (char*)Allocator_Allocate_Memory(Allocator, sizeof(char)*(String.Size+1));
    Buffer[String.Size] = 0;
    Memory_Copy(Buffer, String.Str, String.Size*sizeof(char));
    Str = Buffer;
    Size = String.Size;
}

string::string(allocator* Allocator, wstring Str) {
    scratch Scratch = Scratch_Get();

    const wchar_t* WStrAt = Str.Str;
    const wchar_t* WStrEnd = WStrAt+Str.Size;

    char* StrStart = (char*)Scratch_Push(&Scratch, (Str.Size*4)+1, DEFAULT_ALIGNMENT);
    char* StrEnd = StrStart + Str.Size*4;
    char* StrAt = StrStart;

    for(;;) {
        Assert(StrAt <= StrEnd);
        if(WStrAt >= WStrEnd) {
            Assert(WStrAt == WStrEnd);
            break;
        }

        u32 Length;
        u32 Codepoint = UTF16_Read(WStrAt, &Length);
        WStrAt += Length;
        StrAt += UTF8_Write(StrAt, Codepoint);
    }

    *StrAt = 0;
    *this = string(Allocator, string(StrStart, (uptr)(StrAt-StrStart)));
}

string::string(allocator* Allocator, const char* Format, va_list Args) {
    static char TempBuffer[1];
    uptr LogLength = (uptr)stbsp_vsnprintf(TempBuffer, 1, Format, Args);
    char* Buffer = (char*)Allocator_Allocate_Memory(Allocator, LogLength+1);
    stbsp_vsnprintf(Buffer, (int)(LogLength+1), Format, Args);
    *this = string(Buffer, LogLength);
}

string::string(allocator* Allocator, const char* Format, ...) {
    va_list List;
    va_start(List, Format);
    *this = string(Allocator, Format, List);
    va_end(List);
}

const char& string::operator[](uptr Index) const {
    Assert(Index < Size);
    return Str[Index];
}

bool operator==(string A, string B) {
    return String_Equals(A, B);
}

bool String_Equals(string A, string B, str_case Case) {
    if(A.Size != B.Size) return false;

    if(Case == str_case::Sensitive) {
        for(uptr i = 0; i < A.Size; i++) {
            char CharA = A[i];
            char CharB = B[i];
            if(CharA != CharB) {
                return false;
            }
        }
    } else {
        for(uptr i = 0; i < A.Size; i++) {
            char CharA = To_Lower(A[i]);
            char CharB = To_Lower(B[i]);
            if(CharA != CharB) {
                return false;
            }
        }
    }

    return true;
}

inline bool String_Is_Null_Or_Empty(string String) {
    return !String.Size || !String.Str;
}

string String_Concat(allocator* Allocator, string StringA, string StringB) {
    uptr TotalSize = StringA.Size+StringB.Size;
    char* Buffer = (char*)Allocator_Allocate_Memory(Allocator, (TotalSize+1)*sizeof(char));
    Memory_Copy(Buffer, StringA.Str, StringA.Size*sizeof(char));
    Memory_Copy(Buffer+StringA.Size, StringB.Str, StringB.Size*sizeof(char));
    Buffer[TotalSize] = 0;
    return string(Buffer, TotalSize);
}

string String_Concat(allocator* Allocator, const span<string>& Strings) {
    uptr TotalSize = 0;
    for(const string& Str : Strings) {
        TotalSize += Str.Size;
    }
    char* Buffer = (char*)Allocator_Allocate_Memory(Allocator, (TotalSize+1)*sizeof(char));
    char* BufferAt = Buffer;

    for(const string& Str : Strings) {
        Memory_Copy(BufferAt, Str.Str, Str.Size*sizeof(char));
        BufferAt += Str.Size;
    }
    *BufferAt = 0;
    return string(Buffer, TotalSize);
}

string String_Substr(string String, uptr FirstIndex, uptr LastIndex) {
    Assert(FirstIndex <= LastIndex);
    if(FirstIndex < String.Size && LastIndex <= String.Size) {
        const char* StrAt = String.Str+FirstIndex;
        uptr NewSize = LastIndex-FirstIndex;
        return string(StrAt, NewSize);
    }
    return String;
}

uptr String_Find_Last(string String, char Character) {
    if(String.Size == 0) return STR_INVALID_FIND;

    const char* End = String.Str + (String.Size-1);
    for(uptr i = String.Size; i != 0; i--) {
        uptr Index = i-1;
        if(String.Str[Index] == Character) {
            return Index;
        }
    }

    return STR_INVALID_FIND;
}

bool String_Begins_With(string String, string Substr) {
    if(String.Size < Substr.Size) return false;

    for(uptr i = 0; i < Substr.Size; i++) {
        if(String[i] != Substr[i]) {
            return false;
        }
    }

    return true;
}

uptr String_Find(string String, string Substring) {
    uptr StopOffset = Max(String.Size+1, Substring.Size)-Substring.Size;
    uptr PStart = 0;
    uptr PEnd = StopOffset;

    if(Substring.Size > 0) {
        char FirstChar = Substring[0];
        string SubstringTail = String_Substr(Substring, 1, Substring.Size);
        for(; PStart < PEnd; PStart++) {
            char HaystackChar = String[PStart];
            if(HaystackChar == FirstChar) {
                if(String_Begins_With(String_Substr(String, PStart+1, String.Size), 
                                      SubstringTail)) {
                    break;
                }
            }
        }
    }

    uptr Result = STR_INVALID_FIND;
    if(PStart < PEnd) {
        Result = PStart;
    }
    return Result;
}

string String_Get_Path(string String) {
    uptr LastIndexDoubleSlash = String_Find_Last(String, '\\')+1;
    uptr LastIndexSlash = String_Find_Last(String, '/')+1;

    uptr LastIndex = 0;
    if(LastIndexDoubleSlash != 0 && LastIndexSlash != 0) {
        LastIndex = LastIndexDoubleSlash > LastIndexSlash ? LastIndexDoubleSlash : LastIndexSlash;
    } else if(LastIndexDoubleSlash != 0) {
        LastIndex = LastIndexDoubleSlash;
    } else if(LastIndexSlash != 0) {
        LastIndex = LastIndexSlash;
    }

    return String_Substr(String, 0, LastIndex);
}

inline string String_To_Date_Format(allocator* Allocator, u32 Value) {
    return Value < 10 ? string(Allocator, "0%d", Value) : string(Allocator, "%d", Value);
}

string String_To_Millisecond_Format(allocator* Allocator, u32 Value) {
    if(Value < 10) {
        return string(Allocator, "00%d", Value);
    } else if(Value < 100) {
        return string(Allocator, "%0%d", Value);
    } else {
        return string(Allocator, "%d", Value);
    }
}

void String_Free(allocator* Allocator, string String) {
    if(String.Str) {
        Allocator_Free_Memory(Allocator, (void*)String.Str);
    }
}

wstring::wstring(const wchar_t* _Str) : Str(_Str), Size(wcslen(_Str)) { } 

wstring::wstring(const wchar_t* _Str, uptr _Size) : Str(_Str), Size(_Size) { }

wstring::wstring(allocator* Allocator, wstring String) {
    wchar_t* Buffer = (wchar_t*)Allocator_Allocate_Memory(Allocator, sizeof(wchar_t)*(String.Size+1));
    Buffer[String.Size] = 0;
    Memory_Copy(Buffer, String.Str, String.Size*sizeof(wchar_t));
    Str = Buffer;
    Size = String.Size;
}

wstring::wstring(allocator* Allocator, string String) {
    scratch Scratch = Scratch_Get();

    const char* StrAt = String.Str;
    const char* StrEnd = StrAt+String.Size;

    wchar_t* WStrStart = (wchar_t*)Scratch_Push(&Scratch, (String.Size+1)*sizeof(wchar_t), DEFAULT_ALIGNMENT);
    wchar_t* WStrEnd = WStrStart + String.Size;
    wchar_t* WStrAt = WStrStart;

    for(;;) {
        Assert(WStrAt <= WStrEnd);
        if(StrAt >= StrEnd) {
            Assert(StrAt == StrEnd);
            break;
        }

        u32 Length;
        u32 Codepoint = UTF8_Read(StrAt, &Length);
        StrAt += Length;
        WStrAt += UTF16_Write(WStrAt, Codepoint);
    }

    *WStrAt = 0;
    *this = wstring(Allocator, wstring(WStrStart, (uptr)(WStrAt-WStrStart)));
}

const wchar_t& wstring::operator[](uptr Index) const {
    Assert(Index < Size);
    return Str[Index];
}

bool operator==(wstring A, wstring B) {
    return WString_Equals(A, B);
}

bool WString_Equals(wstring A, wstring B, str_case Case) {
    if(A.Size != B.Size) return false;

    if(Case == str_case::Sensitive) {
        for(uptr i = 0; i < A.Size; i++) {
            wchar_t CharA = A[i];
            wchar_t CharB = B[i];
            if(CharA != CharB) {
                return false;
            }
        }
    } else {
        for(uptr i = 0; i < A.Size; i++) {
            wchar_t CharA = To_Lower(A[i]);
            wchar_t CharB = To_Lower(B[i]);
            if(CharA != CharB) {
                return false;
            }
        }
    }

    return true;
}

wstring WString_Concat(allocator* Allocator, wstring StringA, wstring StringB) {
    uptr TotalSize = StringA.Size+StringB.Size;
    wchar_t* Buffer = (wchar_t*)Allocator_Allocate_Memory(Allocator, (TotalSize+1)*sizeof(wchar_t));
    Memory_Copy(Buffer, StringA.Str, StringA.Size*sizeof(wchar_t));
    Memory_Copy(Buffer+StringA.Size, StringB.Str, StringB.Size*sizeof(wchar_t));
    Buffer[TotalSize] = 0;
    return wstring(Buffer, TotalSize);
}

wstring WString_Concat(allocator* Allocator, const span<wstring>& Strings) {
    uptr TotalSize = 0;
    for(const wstring& Str : Strings) {
        TotalSize += Str.Size;
    }
    wchar_t* Buffer = (wchar_t*)Allocator_Allocate_Memory(Allocator, (TotalSize+1)*sizeof(wchar_t));
    wchar_t* BufferAt = Buffer;

    for(const wstring& Str : Strings) {
        Memory_Copy(BufferAt, Str.Str, Str.Size*sizeof(wchar_t));
        BufferAt += Str.Size;
    }
    *BufferAt = 0;
    return wstring(Buffer, TotalSize);
}

wstring WString_Substr(wstring String, uptr FirstIndex, uptr LastIndex) {
    Assert(FirstIndex <= LastIndex);
    if(FirstIndex < String.Size && LastIndex <= String.Size) {
        const wchar_t* StrAt = String.Str+FirstIndex;
        uptr NewSize = LastIndex-FirstIndex;
        return wstring(StrAt, NewSize);
    }
    return String;
}

uptr WString_Find_Last(wstring String, wchar_t Character) {
    if(String.Size == 0) return STR_INVALID_FIND;

    const wchar_t* End = String.Str + (String.Size-1);
    for(uptr i = String.Size; i != 0; i--) {
        uptr Index = i-1;
        if(String.Str[Index] == Character) {
            return Index;
        }
    }

    return STR_INVALID_FIND;
}

wstring WString_Get_Filename_Without_Ext(wstring String) {
    uptr LastIndexDoubleSlash = WString_Find_Last(String, '\\')+1;
    uptr LastIndexSlash = WString_Find_Last(String, '/')+1;

    uptr FirstIndex = 0;
    if(LastIndexDoubleSlash != STR_INVALID_FIND && LastIndexSlash != STR_INVALID_FIND) {
        FirstIndex = LastIndexDoubleSlash > LastIndexSlash ? LastIndexDoubleSlash : LastIndexSlash;
    } else if(LastIndexDoubleSlash != STR_INVALID_FIND) {
        FirstIndex = LastIndexDoubleSlash;
    } else if(LastIndexSlash != STR_INVALID_FIND) {
        FirstIndex = LastIndexSlash;
    }

    uptr LastIndex = WString_Find_Last(String, '.');
    if(LastIndex == STR_INVALID_FIND) {
        LastIndex = String.Size;
    }

    return WString_Substr(String, FirstIndex, LastIndex);
}

void WString_Free(allocator* Allocator, wstring String) {
    if(String.Str) {
        Allocator_Free_Memory(Allocator, (void*)String.Str);
    }
}