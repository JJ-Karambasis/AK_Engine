
bool UI_Init(ui* UI, const ui_create_info& CreateInfo) {
    UI->Arena = Arena_Create(CreateInfo.Allocator);
    UI->GlyphCache = CreateInfo.GlyphCache;
    return true;
}

void UI_Push_Font(ui* UI, font_id FontID) {
    UI->FontID = FontID;
}