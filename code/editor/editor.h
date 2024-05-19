#ifndef EDITOR_H
#define EDITOR_H

#include <engine.h>
#include "editor_modules.h"
#include <os.h>

#include "editor_input.h"
#include "ui/ui.h"

// #include "level_editor/level_editor.h"

#include "windows.h"

struct editor {
    arena*                            Arena;
    ak_job_system*                    JobSystemHigh;
    ak_job_system*                    JobSystemLow;
    renderer*                         Renderer;
    packages*                         Packages;
    font_manager*                     FontManager;
    window_manager                    WindowManager;
    glyph_cache*                      GlyphCache;
    ui_render_pass                    UIRenderPass;
    ui_pipeline                       UIPipeline;
    editor_input_manager              InputManager;
    renderer_texture                  DefaultTexture;
    const_buffer                      MainFontBuffer;
    font_id                           MainFont;
};

void Window_Update(editor* Editor, window* Window);

#endif