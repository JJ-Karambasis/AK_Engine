#ifndef TEXT_SHAPER_H
#define TEXT_SHAPER_H

struct text_shaper;

struct text_shaper_face_id {
    u64          ID;
    text_shaper* Shaper;
};

struct text_shape_pos {
    svec2 Offset;
    svec2 Advance;
};

struct text_glyph_info {
    u32 Codepoint;
    u32 Cluster;
};

struct text_shape_result {
    u32                    GlyphCount;
    const text_glyph_info* Glyphs;
    const text_shape_pos*  Positions;
};

enum text_language {
    TEXT_LANGUAGE_UNKNOWN,
    TEXT_LANGUAGE_ENGLISH,
    TEXT_LANGUAGE_ARABIC,
    TEXT_LANGUAGE_COUNT
};

struct text_shaper_buffer_properties {
    uba_direction Direction = UBA_DIRECTION_LTR;
    uba_script    Script    = UBA_SCRIPT_NONE;
    text_language Language  = TEXT_LANGUAGE_UNKNOWN;
};

struct text_shaper_create_info {
    u32 MaxFaceCount = 128;
};

struct text_shaper_shape_info {
    allocator*                    Allocator;
    string                        Text;
    text_shaper_buffer_properties Properties;
};

text_shaper*          Text_Shaper_Create(const text_shaper_create_info& CreateInfo);
void                  Text_Shaper_Delete(text_shaper* Shaper);

text_shaper_face_id   Text_Shaper_Create_Face(text_shaper* Shaper, glyph_face_id Face);
void                  Text_Shaper_Delete_Face(text_shaper* Shaper, text_shaper_face_id Face);
text_shape_result     Text_Shaper_Face_Shape(text_shaper_face_id Face, const text_shaper_shape_info& ShapeInfo);
void                  Text_Shaper_Free_Result(text_shape_result* Result, allocator* Allocator);

#include "harfbuzz/harfbuzz_text_shaper.h"

#endif