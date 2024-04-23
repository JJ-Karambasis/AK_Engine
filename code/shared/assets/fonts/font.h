#ifndef FONT_H
#define FONT_H

/*TODO: This font implemenetation is not complete
//      1. Cache keeps growing infinitely with no limit. An LRU cache is sufficient to limit 
//         memory usage
//      2. Font should use a single texture atlas. Right now the bitmap has a texture for
//         all glyphs along with a texture view, format, and bind group. This should just
//         hold data into texture atlas (can instance very easily)
*/

struct font_bitmap {
    gpu_texture Texture;
};

struct font_create_info {
    gdi_context*                      GDIContext;
    gdi_handle<gdi_bind_group_layout> BitmapBindGroupLayout;
    glyph_manager*                    GlyphManager;
    const_buffer                      FontBuffer;
};

struct font;
font*       Font_Create(const font_create_info& CreateInfo);
font_bitmap Font_Get_Bitmap(font* Font, u32 Codepoint, u32 Size);

#endif