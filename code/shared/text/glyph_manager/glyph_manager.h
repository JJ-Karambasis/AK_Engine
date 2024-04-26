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

struct glyph_face_id {
    u64                Generation;
    struct glyph_face* Face;
};

struct glyph_manager_create_info {
    u32 MaxFaceCount = 128;
};

struct glyph_manager;
glyph_manager* Glyph_Manager_Create(const glyph_manager_create_info& CreateInfo);
glyph_face_id  Glyph_Manager_Create_Face(glyph_manager* Manager, const_buffer Buffer);
const_buffer   Glyph_Face_Get_Font_Buffer(glyph_face_id FaceID);
u32            Glyph_Face_Get_Size(glyph_face_id FaceID);
void           Glyph_Face_Set_Size(glyph_face_id FaceID, u32 PixelSize);
glyph_bitmap   Glyph_Face_Create_Bitmap(glyph_face_id FaceID, allocator* Allocator, u32 Codepoint);

#endif