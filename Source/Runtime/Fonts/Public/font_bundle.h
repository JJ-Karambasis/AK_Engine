#ifndef FONT_BUNDLE_H
#define FONT_BUNDLE_H

typedef struct font_range
{
    glyph_face* Font;
    uint32_t    MinChar;
    uint32_t    MaxChar;
} font_range;

typedef struct font_bundle
{
} font_bundle;

glyph_font* Font_Bundle_Get_First(font_bundle* Bundle, uint32_t Codepoint);

#endif
