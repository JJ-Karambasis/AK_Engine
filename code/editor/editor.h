#ifndef EDITOR_H
#define EDITOR_H

#include <engine.h>
#include "editor_modules.h"
#include <os.h>

#include "editor_input.h"
#include "windows.h"
#include "ui/ui.h"

// #include "level_editor/level_editor.h"


struct editor {
    arena*                            Arena;
    ak_job_system*                    JobSystemHigh;
    ak_job_system*                    JobSystemLow;
    renderer*                         Renderer;
    packages*                         Packages;
    font_manager*                     FontManager; 
    window_manager                    WindowManager;
    glyph_cache*                      GlyphCache;
    gdi_handle<gdi_bind_group_layout> GlobalLayout;
    gdi_handle<gdi_render_pass>       UIRenderPass;
    gdi_handle<gdi_pipeline>          UIPipeline;
    editor_input_manager              InputManager;
    renderer_texture                  DefaultTexture;
    const_buffer                      MainFontBuffer;
    font_id                           MainFont;
};

void Window_Update(editor* Editor, window* Window);

#endif