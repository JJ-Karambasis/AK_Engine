#ifndef WIN32_FONT_LOADER_H
#define WIN32_FONT_LOADER_H

typedef struct win32_font
{
    os_font            Base;
    IDWriteFontFamily* FontFamily;
} win32_font;

typedef struct win32_font_loader
{
    arena*                 Arena;
    IDWriteFactory*        Factory;
    IDWriteFontCollection* FontCollection;
    os_font_list           Fonts;
    arena_marker           Marker;
} win32_font_loader;

#endif