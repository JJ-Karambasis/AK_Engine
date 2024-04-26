#ifndef TEXT_SHAPER_H
#define TEXT_SHAPER_H

struct text_shaper_face_id {
    u64                      Generation;
    struct text_shaper_face* Face;
};

struct text_shaper_buffer_id {
    u64                        Generation;
    struct text_shaper_buffer* Buffer;
};

struct text_shape_pos {
    s32 XOffset;
    s32 YOffset;
    s32 XAdvance;
    s32 YAdvance;
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
    TEXT_LANGUAGE_COUNT
};

struct text_shaper_buffer_properties {
    uba_direction Direction = UBA_DIRECTION_LTR;
    uba_script    Script    = UBA_SCRIPT_NONE;
    text_language Language  = TEXT_LANGUAGE_UNKNOWN;
};

struct text_shaper_create_info {
    u32 FaceCount   = 128;
    u32 BufferCount = 1024;
};

struct text_shaper;
text_shaper*          Text_Shaper_Create(const text_shaper_create_info& CreateInfo);
void                  Text_Shaper_Delete(text_shaper* Shaper);
text_shaper_buffer_id Text_Shaper_Create_Buffer(text_shaper* Shaper);
void                  Text_Shaper_Delete_Buffer(text_shaper* Shaper, text_shaper_buffer_id Buffer);
void                  Text_Shaper_Buffer_Add(text_shaper_buffer_id Buffer, string String);
void                  Text_Shaper_Buffer_Clear(text_shaper_buffer_id Buffer);
void                  Text_Shaper_Buffer_Set_Properties(text_shaper_buffer_id Buffer, const text_shaper_buffer_properties& Properties);
text_shaper_face_id   Text_Shaper_Create_Face(text_shaper* Shaper, glyph_face_id Face);
void                  Text_Shaper_Delete_Face(text_shaper* Shaper, text_shaper_face_id Face);
text_shape_result     Text_Shaper_Face_Shape(text_shaper_face_id Face, text_shaper_buffer_id Buffer, allocator* Allocator);
void                  Text_Shaper_Free_Result(text_shape_result* Result, allocator* Allocator);

#include "harfbuzz/harfbuzz_text_shaper.h"

#endif