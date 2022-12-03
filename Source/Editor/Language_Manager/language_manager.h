#ifndef LANGUAGE_MANAGER_H
#define LANGUAGE_MANAGER_H

typedef struct language_dictionary
{
    uint32_t SlotCount;
} language_dictionary;

typedef struct language_pack
{
    str8                  Name;
    str8_list             Fonts;
    language_dictionary   Dictionary;
    struct language_pack* Next;
} language_packs;

typedef struct language_manager
{
    arena* Arena;
} language_manager;

bool8_t Language_Manager_Create(allocator* Allocator);
void    Language_Manager_Create_Language_Pack(language_manager* Manager, str8 Name, str8* Fonts, uint32_t FontCount, str8 DictionaryPath);
void    Language_Pack_Set(language_manager* Manager, str8 Name);
str16   Language_Pack_Get_Key_Value(language_manager* Manager, str8 Key);

#endif
