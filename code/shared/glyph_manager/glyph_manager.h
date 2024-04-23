#ifndef GLYPH_MANAGER_H
#define GLYPH_MANAGER_H

enum glyph_bitmap_format {
    GLYPH_BITMAP_FORMAT_GREYSCALE,
    GLYPH_BITMAP_FORMAT_COUNT
};

struct glyph_bitmap {
    glyph_bitmap_format Format;
    uvec2               Dim;
    const_buffer        Texels;
};

struct glyph_manager;
struct glyph_face;
glyph_manager* Glyph_Manager_Create();
glyph_face*    Glyph_Manager_Create_Face(glyph_manager* Manager, const_buffer Buffer);
void           Glyph_Face_Set_Size(glyph_face* Face, u32 PixelSize);
glyph_bitmap   Glyph_Face_Create_Bitmap(glyph_face* Face, allocator* Allocator, u32 Codepoint);


#endif