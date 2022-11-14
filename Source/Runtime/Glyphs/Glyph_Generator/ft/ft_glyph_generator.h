#ifndef FT_GLYPH_GENERATOR_H
#define FT_GLYPH_GENERATOR_H

#include <freetype/freetype.h>
#include <freetype/ftsystem.h>
#include <freetype/ftmodapi.h>

typedef struct ft_font_face
{
    font_face            Face;
    FT_Face              FTFace;
    struct ft_font_face* Next;
} ft_font_face;

typedef struct ft_glyph_generator
{
    glyph_generator Generator;
    arena*          Arena;
    heap*           FreeTypeHeap;
    FT_Library      Library;
    ft_font_face*   FreeFaces;
} ft_glyph_generator;

glyph_generator* FT_Glyph_Generator_Create(allocator* Allocator);

#endif