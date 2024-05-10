internal arena* UI_Build_Arena(ui* UI) {
    u32 Index = UI->BuildIndex % Array_Count(UI->BuildArenas);
    return UI->BuildArenas[Index];
}

u32 UI_Hash_From_String(u32 Seed, string String) {
    u32 Result = Seed;
    for(uptr i = 0; i < String.Size; i++) {
        //confirm: Is this hash function good enough?
        Result = ((Result << 5) + Result) + String[i];
    }
    return Result;
}

string UI_Hash_Part_From_String(string String) {
    string Result = String;

    uptr HashPart = String_Find(String, String_Lit("###"));
    if(HashPart != STR_INVALID_FIND) {
        Result = String_Substr(String, HashPart+3, String.Size);
    }
    return Result;
}

string UI_Display_Part_From_String(string String) {
    string Result = String;
    uptr HashPart = String_Find(String, String_Lit("##"));
    if(HashPart != STR_INVALID_FIND) {
        Result.Size = HashPart;
    }
    return Result;
}

ui_key UI_Key_From_String(ui_key Seed, string String) {
    ui_key Result = 0;
    if(String.Size != 0) {
        string HashPart = UI_Hash_Part_From_String(String);
        Result = UI_Hash_From_String(Seed, HashPart);
    }
    return Result;
}

ui_key UI_Key_From_StringF(ui_key Seed, const char* Format, ...) {
    scratch Scratch = Scratch_Get();

    va_list Args;
    va_start(Args, Format);
    string String(&Scratch, Format, Args);
    va_end(Args);

    ui_key Key = UI_Key_From_String(Seed, String);
    return Key;
}

ui_size::ui_size(ui_size_type _Type, f32 _Value, f32 _Strictness) :
    Type(_Type), Value(_Value), Strictness(_Strictness) {}