#ifndef GLYPH_FONT_H
#define GLYPH_FONT_H

typedef struct glyph_generator glyph_generator;
typedef struct glyph_face glyph_face;
typedef struct text_shaper text_shaper;

typedef struct glyph_font
{
    arena*           Arena;
    glyph_generator* Generator;
    glyph_face*      Face;
    text_shaper*     Shaper;
} glyph_font;

glyph_font* Glyph_Font_Create(allocator* Allocator, glyph_generator* Generator, buffer FontBuffer);
void        Glyph_Font_Delete(glyph_font* Font);

#endif