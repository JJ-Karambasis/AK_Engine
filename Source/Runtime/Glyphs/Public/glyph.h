#ifndef GLYPH_H
#define GLYPH_H

typedef struct glyph
{
    uint32_t       Codepoint;
    uint32_t       Width;
    uint32_t       Height;
    //gpu_texture* Texture;
} glyph;

//NOTE(EVERYONE): The format of the bitmap is always RGBA
typedef struct glyph_bitmap
{
    uint32_t* Texels;
    uint32_t  Width;
    uint32_t  Height;
} glyph_bitmap;

#endif