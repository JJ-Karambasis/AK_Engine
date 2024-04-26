#ifndef EDITOR_H
#define EDITOR_H

#include <engine.h>
#include "editor_modules.h"
#include "os/os.h"
#include "editor_input.h"
#include "ui/ui.h"

// #include "level_editor/level_editor.h"

struct view {
    view* Next;
    view* Prev;
};

struct panel {
    //Tree hierarchy
    u32    ChildCount;
    panel* FirstChild;
    panel* LastChild;
    panel* NextSibling;
    panel* PrevSibling;
    panel* Parent;

    //Split data
    //ui_axis2 SplitAxis;
    f32      PercentOfParent;

    //Views
    view* FirstView;
    view* LastView;
    view* ChosenView;
};

struct window;

struct window_handle {
    window* Window;
    u64     Generation;
};

struct window {
    arena*                              Arena;
    u64                                 Generation;
    os_window_id                        WindowID;
    gdi_handle<gdi_swapchain>           Swapchain;
    gdi_format                          SwapchainFormat;
    array<gdi_handle<gdi_texture_view>> SwapchainViews;
    array<gdi_handle<gdi_framebuffer>>  Framebuffers;
    ui                                  UI;
    uvec2                               Size;
    gdi_handle<gdi_buffer>              UIGlobalBuffer;
    gdi_handle<gdi_bind_group>          UIGlobalBindGroup;
    gdi_handle<gdi_buffer>              UIDynamicBuffer;
    uptr                                UIDynamicBufferSize;
    uptr                                UIDynamicBufferCount;
    gdi_handle<gdi_bind_group>          UIDynamicBindGroup;

    //Window links
    window* Prev;
    window* Next;
};

struct editor {
    arena*                            Arena;
    ak_job_system*                    JobSystemHigh;
    ak_job_system*                    JobSystemLow;
    gdi_context*                      GDIContext;
    packages*                         Packages;
    glyph_manager*                    GlyphManager;
    // text_shaper*                      TextShaper;
    glyph_cache*                      GlyphCache;
    gdi_handle<gdi_render_pass>       UIRenderPass;    
    gdi_handle<gdi_pipeline>          UIBoxPipeline;
    gdi_handle<gdi_bind_group_layout> GlobalBindGroupLayout;
    gdi_handle<gdi_bind_group_layout> DynamicBindGroupLayout;
    gdi_handle<gdi_bind_group_layout> LinearSamplerBindGroupLayout;
    editor_input_manager              InputManager;
    gpu_texture                       DefaultTexture;
    const_buffer                      MainFontBuffer;
    ui_font                           MainFont;
    
    window* FirstWindow;
    window* LastWindow;
    window* FreeWindows;

    panel* FreePanels;
    view*  FreeViews;
};

bool          Window_Is_Open(window_handle Handle);
window_handle Window_Open(editor* Editor, os_monitor_id MonitorID, svec2 Offset, uvec2 Size, u32 Border, string Name);
void          Window_Close(editor* Editor, window_handle Handle);
window*       Window_Get(window_handle Handle);
void          Window_Update(editor* Editor, window* Window);

#endif