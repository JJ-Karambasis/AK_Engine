#ifndef UI_H
#define UI_H

struct ui_box {
    //Hash state
    ui_box* HashNext;
    ui_box* HashPrev;

    //Hierarchy state
    ui_box* FirstChild;
    ui_box* LastChild;
    ui_box* NextSibling;
    ui_box* PrevSibling;
    ui_box* Parent;

    //Per build info
};

struct ui_box_hash_slot {
    ui_box* First;
    ui_box* Last;
};

struct ui_create_info {
    allocator*    Allocator;
    glyph_cache*  GlyphCache;
};

struct ui {
    //Main arena
    arena*  Arena;
    glyph_cache* GlyphCache;
    font_id FontID;

    //Box cache
    ui_box*           FirstFreeBox;
    u64               BoxHashTableCount;
    ui_box_hash_slot* BoxHashTable;

    //Build Hierarchy
    ui_box* Root;
};

bool UI_Init(ui* UI, const ui_create_info& CreateInfo);

void UI_Push_Font(ui* UI, font_id FontID);

#endif