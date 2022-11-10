#ifndef EDITOR_UI_H
#define EDITOR_UI_H

#include <freetype/freetype.h>
#include <freetype/ftsystem.h>
#include <freetype/ftmodapi.h>

typedef struct editor_ui
{
    arena*     Arena;
    FT_Library Library;
} editor_ui;

editor_ui* EditorUI_Init(arena* Arena);
void       EditorUI_Shutdown();
void       EditorUI_Set(editor_ui* UI);
editor_ui* EditorUI_Get();

#endif