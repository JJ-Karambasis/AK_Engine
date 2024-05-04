#ifndef GLYPH_MANAGER_H
#define GLYPH_MANAGER_H

struct glyph_manager;

enum glyph_bitmap_format {
    GLYPH_BITMAP_FORMAT_GREYSCALE,
    GLYPH_BITMAP_FORMAT_COUNT
};

struct glyph_bitmap {
    glyph_bitmap_format Format;
    uvec2               Dim;
    const_buffer        Texels;
};

struct glyph_face_id {
    u64            ID;
    glyph_manager* Manager;
};

struct glyph_face_metrics {
    u32 Ascender;
    u32 Descender;
    u32 LineGap;
};

struct glyph_metrics {
    uvec2 Advance;
    svec2 Offset;
    uvec2 Dim;
};

struct glyph_manager_create_info {
    u32 MaxFaceCount = 128;
};

glyph_manager*     Glyph_Manager_Create(const glyph_manager_create_info& CreateInfo);
glyph_face_id      Glyph_Manager_Create_Face(glyph_manager* Manager, const_buffer Buffer, u32 PixelSize);
const_buffer       Glyph_Face_Get_Font_Buffer(glyph_face_id FaceID);
glyph_face_metrics Glyph_Face_Get_Metrics(glyph_face_id FaceID);
glyph_metrics      Glyph_Face_Get_Glyph_Metrics(glyph_face_id FaceID, u32 GlyphIndex);
svec2              Glyph_Face_Get_Kerning(glyph_face_id FaceID, u32 GlyphA, u32 GlyphB);
glyph_bitmap       Glyph_Face_Create_Bitmap(glyph_face_id FaceID, allocator* Allocator, u32 GlyphIndex);

#endif