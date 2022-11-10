#ifndef EDITOR_H
#define EDITOR_H

#include <Core/core.h>
#include <Editor_UI/editor_ui.h>

typedef struct editor
{
    arena*     Arena;
    core*      Core;
    editor_ui* EditorUI;
    os_window* MainWindow;
    str8       RootPath;
    str8       DataPath;
} editor;

editor* Editor_Init();
void    Editor_Update();
void    Editor_Shutdown();
void    Editor_Set(editor* Editor);
editor* Editor_Get();

#endif