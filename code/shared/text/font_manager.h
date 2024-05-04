#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

struct font_manager;

struct font_manager_create_info {
    u32 MaxFontCount = 1024; 
};

struct font_id {
    u64 ID;
    font_manager* Manager;
};

font_manager* Font_Manager_Create(const font_manager_create_info& CreateInfo);
void          Font_Manager_Delete(font_manager* FontManager);
font_id       Font_Manager_Create_Font(font_manager* FontManager, const_buffer FontBuffer, u32 FontSize);
void          Font_Manager_Delete_Font(font_manager* FontManager, font_id Font);

const glyph_face_metrics* Font_Get_Metrics(font_id Font);
u32                       Font_Get_Size(font_id Font);
const_buffer              Font_Get_Buffer(font_id Font);
glyph_metrics             Font_Get_Glyph_Metrics(font_id Font, u32 GlyphIndex);
glyph_bitmap              Font_Rasterize_Glyph(font_id Font, allocator* Allocator, u32 GlyphIndex);  
text_shape_result         Font_Shape(font_id Font, const text_shaper_shape_info& ShapeInfo);
svec2                     Font_Get_Kerning(font_id Font, u32 GlyphA, u32 GlyphB);

#endif