#ifndef EDITOR_H
#define EDITOR_H

#include <Core/core.h>
#include <Engine_Console/engine_console.h>
#include <GPU/gpu.h>
#include <Fonts/fonts.h>

typedef struct editor_window
{
    os_window*            Window;
    gpu_display*          Display;
    struct editor_window* Next;
} editor_window;

typedef struct editor
{
    arena*              Arena;
    core*               Core;
    str8                RootPath;
    str8                DataPath;
    gpu_context*        ContextGPU;
    gpu_device_context* DeviceGPU;
    editor_window*      FreeWindows;
    editor_window*      MainWindow;
    glyph_generator*    GlyphGenerator;
    glyph_cache*        GlyphCache;
    os_font_loader*     FontLoader;
    engine_console*     EngineConsole;
} editor;

editor*        Editor_Init();
void           Editor_Update();
void           Editor_Shutdown();
editor_window* Editor_Create_Window(uint32_t Width, uint32_t Height, str8 WindowTitle, uint64_t WindowFlags);
void           Editor_Delete_Window(editor_window* Window);
void           Editor_Set(editor* Editor);
editor*        Editor_Get();

#endif