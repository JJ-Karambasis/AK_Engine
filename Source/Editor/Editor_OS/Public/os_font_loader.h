#ifndef OS_FONT_LOADER_H
#define OS_FONT_LOADER_H

typedef struct os_font_loader os_font_loader;

typedef struct os_font
{
    struct os_font* Next;
} os_font;

typedef struct os_font_list
{
    os_font* First;
    os_font* Last;
    uint64_t Count;
} os_font_list;

os_font_loader* OS_Font_Loader_Create(allocator* Allocator);
os_font_list    OS_Font_Loader_Get_All(os_font_loader* Loader);
os_font*        OS_Font_Loader_Get_Font(os_font_loader* FontLoader, str8 Name);
buffer          OS_Font_Load_Data(os_font_loader* FontLoader, os_font* Font, allocator* Allocator);
void            OS_Font_Loader_Delete(os_font_loader* Loader);

#endif