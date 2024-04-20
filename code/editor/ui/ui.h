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

struct ui {
    //Main arena
    arena*            Arena;

    //Box cache
    ui_box*           FirstFreeBox;
    u64               BoxHashTableCount;
    ui_box_hash_slot* BoxHashTable;

    //Build Hierarchy
    ui_box* Root;
};

bool UI_Init(ui* UI, allocator* Allocator);

#endif