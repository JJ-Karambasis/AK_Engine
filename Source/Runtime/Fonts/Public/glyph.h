#ifndef GLYPH_H
#define GLYPH_H

typedef struct gpu_texture2D gpu_texture2D;

typedef struct glyph_texture
{
    uint32_t       Width;
    uint32_t       Height;
    gpu_texture2D* Texture;
} glyph_texture;

typedef struct glyph
{
    uint32_t       Codepoint;
    int32_t        XBearing;
    int32_t        YBearing;
    glyph_texture  Texture;
} glyph;

typedef struct glyph_info
{
    const glyph* Glyph;
    int32_t      XAdvance;
    int32_t      YAdvance;
    int32_t      XOffset;
    int32_t      YOffset;
} glyph_info;

typedef struct glyph_list
{
    glyph**  Glyphs;
    uint64_t Count;
} glyph_info_list;

#endif