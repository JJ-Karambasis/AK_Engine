#ifndef UI_COMMON_H
#define UI_COMMON_H

enum ui_axis2 {
    UI_AXIS2_X,
    UI_AXIS2_Y,
    UI_AXIS2_COUNT
};

typedef u32 ui_key;

u32    UI_Hash_From_String(u32 Seed, string String);
string UI_Hash_Part_From_String(string String);
string UI_Display_Part_From_String(string String);
ui_key UI_Key_From_String(ui_key Seed, string String);
ui_key UI_Key_From_StringF(ui_key Seed, const char* Format, ...);

enum ui_size_type {
    UI_SIZE_TYPE_NONE,
    UI_SIZE_TYPE_PIXELS
};

struct ui_size {
    ui_size_type Type;
    f32          Value;
    f32          Strictness;

    ui_size() = default;
    ui_size(ui_size_type Type, f32 Value, f32 Strictness);
};

#define UI_Pixels(value, strictness) ui_size(UI_SIZE_TYPE_PIXELS, value, strictness)

#endif