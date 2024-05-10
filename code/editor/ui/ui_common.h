#ifndef UI_COMMON_H
#define UI_COMMON_H

enum ui_axis2 {
    UI_AXIS2_X,
    UI_AXIS2_Y,
    UI_AXIS2_COUNT
};

//todo: At some point we need to turn this into a u64 so we don't get any
//collisions (or they will be so rare its basically never). CRC hash is 
//pretty good, but the u32 conversion could make it more possible for collisions
//to occur. This likely won't happen, but just for reliability purposes this
//should be converted to u64 and a custom hash table will need to be implemented
//so that we can hash with 64 bit values (library hash table uses u32 hashes)
typedef u32 ui_key;

u32    UI_Hash_From_String(u32 Seed, string String);
string UI_Hash_Part_From_String(string String);
string UI_Display_Part_From_String(string String);
ui_key UI_Key_From_String(ui_key Seed, string String);
ui_key UI_Key_From_StringF(ui_key Seed, const char* Format, ...);

enum ui_size_type {
    UI_SIZE_TYPE_NONE,
    UI_SIZE_TYPE_PIXELS,
    UI_SIZE_TYPE_TEXT
};

struct ui_size {
    ui_size_type Type;
    f32          Value;
    f32          Strictness;

    ui_size() = default;
    ui_size(ui_size_type Type, f32 Value, f32 Strictness);
};

#define UI_Pixels(value, strictness) ui_size(UI_SIZE_TYPE_PIXELS, value, strictness)
#define UI_Text(padding, strictness) ui_size(UI_SIZE_TYPE_TEXT, padding, strictness)

struct ui_font {
    font_id        Font;
    uba_direction  Direction = UBA_DIRECTION_LTR;
    uba_script     Script;
    text_language  Language = TEXT_LANGUAGE_UNKNOWN;
};

typedef u32 ui_text_alignment_flags;
enum {
    UI_TEXT_ALIGNMENT_NONE = 0,
    UI_TEXT_ALIGNMENT_LEFT_BIT = (1 << 0),
    UI_TEXT_ALIGNMENT_RIGHT_BIT = (1 << 1),
    UI_TEXT_ALIGNMENT_HORZ_CENTER_BIT = (1 << 2),
    UI_TEXT_ALIGNMENT_TOP_BIT = (1 << 3),
    UI_TEXT_ALIGNMENT_BOTTOM_BIT = (1 << 4),
    UI_TEXT_ALIGNMENT_VERT_CENTER_BIT = (1 << 5),
};

#define UI_TEXT_ALIGNMENT_TOP_LEFT (UI_TEXT_ALIGNMENT_TOP_BIT|UI_TEXT_ALIGNMENT_LEFT_BIT)
#define UI_TEXT_ALIGNMENT_TOP_CENTER (UI_TEXT_ALIGNMENT_TOP_BIT|UI_TEXT_ALIGNMENT_HORZ_CENTER_BIT)
#define UI_TEXT_ALIGNMENT_TOP_RIGHT (UI_TEXT_ALIGNMENT_TOP_BIT|UI_TEXT_ALIGNMENT_RIGHT_BIT)
#define UI_TEXT_ALIGNMENT_CENTER_LEFT (UI_TEXT_ALIGNMENT_LEFT_BIT|UI_TEXT_ALIGNMENT_VERT_CENTER_BIT)
#define UI_TEXT_ALIGNMENT_CENTER (UI_TEXT_ALIGNMENT_HORZ_CENTER_BIT|UI_TEXT_ALIGNMENT_VERT_CENTER_BIT)
#define UI_TEXT_ALIGNMENT_CENTER_RIGHT (UI_TEXT_ALIGNMENT_RIGHT_BIT|UI_TEXT_ALIGNMENT_VERT_CENTER_BIT)
#define UI_TEXT_ALIGNMENT_BOTTOM_LEFT (UI_TEXT_ALIGNMENT_BOTTOM_BIT|UI_TEXT_ALIGNMENT_LEFT_BIT)
#define UI_TEXT_ALIGNMENT_BOTTOM_CENTER (UI_TEXT_ALIGNMENT_BOTTOM_BIT|UI_TEXT_ALIGNMENT_HORZ_CENTER_BIT)
#define UI_TEXT_ALIGNMENT_BOTTOM_RIGHT (UI_TEXT_ALIGNMENT_BOTTOM_BIT|UI_TEXT_ALIGNMENT_RIGHT_BIT)



#endif