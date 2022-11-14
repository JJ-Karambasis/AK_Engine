#ifndef EDITOR_UI_H
#define EDITOR_UI_H

typedef struct editor_window
{
    os_window*            Window;
    gpu_display*          Display;
    struct editor_window* Next;
} editor_window;

typedef struct editor_ui
{
    arena*           Arena;
    editor_window*   FreeWindows;
    editor_window*   MainWindow;
    gpu_cmd_buffer*  CmdBuffer;
    buffer           MainFontBuffer;
    glyph_generator* GlyphGenerator;
    glyph_cache*     GlyphCache;
} editor_ui;

editor_ui*     EditorUI_Init(editor* Editor);
editor_window* EditorUI_Create_Window(uint32_t Width, uint32_t Height, str8 WindowTitle, uint64_t WindowFlags);
void           EditorUI_Delete_Window(editor_window* Window);
void           EditorUI_Shutdown();
void           EditorUI_Update(double dt);
void           EditorUI_Set(editor_ui* UI);
editor_ui*     EditorUI_Get();

#endif