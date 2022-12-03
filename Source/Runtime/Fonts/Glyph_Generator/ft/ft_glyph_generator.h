#ifndef FT_GLYPH_GENERATOR_H
#define FT_GLYPH_GENERATOR_H

#include <freetype/freetype.h>
#include <freetype/ftsystem.h>
#include <freetype/ftmodapi.h>
#include <freetype/ftadvanc.h>

typedef struct ft_glyph_face
{
    glyph_face            Base;
    FT_Face               Face;
    struct ft_glyph_face* Next;
} ft_glyph_face;

typedef struct ft_glyph_generator
{
    glyph_generator Generator;
    arena*          Arena;
    heap*           FreeTypeHeap;
    FT_Library      Library;
    ft_glyph_face*  FreeFaces;
} ft_glyph_generator;

glyph_generator* FT_Glyph_Generator_Create(allocator* Allocator);

#endif