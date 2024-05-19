
struct font {
    glyph_face_id       Face;
    text_shaper_face_id ShaperFace;
    glyph_face_metrics  Metrics;
    u32                 Size;
};

struct font_manager {
    arena*           Arena;
    glyph_manager*   GlyphManager;
    text_shaper*     TextShaper;
    async_pool<font> FontPool;
};

bool font_id::Is_Valid() const {
    if(!ID || !Manager) return false;    
    return Async_Pool_Valid_Handle(&Manager->FontPool, async_handle<font>(ID));
}

internal font* Font_Get(font_id FontID) {
    if(!FontID.ID || !FontID.Manager) return nullptr;    
    font_manager* FontManager = FontID.Manager;
    return Async_Pool_Get(&FontManager->FontPool, async_handle<font>(FontID.ID));
}

font_manager* Font_Manager_Create(const font_manager_create_info& CreateInfo) {
    arena* Arena = Arena_Create(Core_Get_Base_Allocator());
    font_manager* Result = Arena_Push_Struct(Arena, font_manager);

    Result->Arena = Arena;
    Result->GlyphManager = Glyph_Manager_Create({
        .MaxFaceCount = CreateInfo.MaxFontCount
    });
    Result->TextShaper = Text_Shaper_Create({
        .MaxFaceCount = CreateInfo.MaxFontCount
    });

    Async_Pool_Create(&Result->FontPool, Result->Arena, CreateInfo.MaxFontCount);

    return Result;
}

font_id Font_Manager_Create_Font(font_manager* FontManager, const_buffer FontBuffer, u32 FontSize) {
    async_handle<font> FontHandle = Async_Pool_Allocate(&FontManager->FontPool);
    if(FontHandle.Is_Null()) {
        return {};
    }

    font* Font = Async_Pool_Get(&FontManager->FontPool, FontHandle);

    Font->Face = Glyph_Manager_Create_Face(FontManager->GlyphManager, FontBuffer, FontSize);
    Font->ShaperFace = Text_Shaper_Create_Face(FontManager->TextShaper, Font->Face);
    Font->Metrics = Glyph_Face_Get_Metrics(Font->Face);
    Font->Size = FontSize;

    return {
        .ID = FontHandle.ID,
        .Manager = FontManager
    };
}

const glyph_face_metrics* Font_Get_Metrics(font_id FontID) {
    font* Font = Font_Get(FontID);
    if(!Font) return nullptr;

    return &Font->Metrics;
}

u32 Font_Get_Size(font_id FontID) {
    font* Font = Font_Get(FontID);
    if(!Font) return 0;
    return Font->Size;
}

const_buffer Font_Get_Buffer(font_id FontID) {
    font* Font = Font_Get(FontID);
    if(!Font) return {};
    return Glyph_Face_Get_Font_Buffer(Font->Face);
}

glyph_metrics Font_Get_Glyph_Metrics(font_id FontID, u32 GlyphIndex) {
    font* Font = Font_Get(FontID);
    if(!Font) return {};
    return Glyph_Face_Get_Glyph_Metrics(Font->Face, GlyphIndex);
}

glyph_bitmap Font_Rasterize_Glyph(font_id FontID, allocator* Allocator, u32 GlyphIndex) {
    font* Font = Font_Get(FontID);
    if(!Font) return {};
    return Glyph_Face_Create_Bitmap(Font->Face, Allocator, GlyphIndex);
}

text_shape_result Font_Shape(font_id FontID, const text_shaper_shape_info& ShapeInfo) {
    font* Font = Font_Get(FontID);
    if(!Font) return {};
    return Text_Shaper_Face_Shape(Font->ShaperFace, ShapeInfo);
}

vec2i Font_Get_Kerning(font_id FontID, u32 GlyphA, u32 GlyphB) {
    font* Font = Font_Get(FontID);
    if(!Font) return {};
    return Glyph_Face_Get_Kerning(Font->Face, GlyphA, GlyphB);
}

void          Font_Manager_Delete_Font(font_manager* FontManager, font_id Font);
