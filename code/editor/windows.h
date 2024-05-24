#ifndef WINDOWS_H
#define WINDOWS_H

struct ui;
struct ui_pipeline;
struct window;
struct window_handle {
    window* Window;
    u64     Generation;

    window_handle() = default;
    window_handle(window* Window);
};

struct window {
    arena*                              Arena;
    u64                                 Generation;
    os_window_id                        OSHandle;
    gdi_handle<gdi_swapchain>           Swapchain;
    gdi_format                          SwapchainFormat;
    array<gdi_handle<gdi_texture_view>> SwapchainViews;
    array<gdi_handle<gdi_framebuffer>>  Framebuffers;
    ui*                                 UI;
    dim2i                               Size;

    //Window links
    window* Prev;
    window* Next;
};

struct window_manager {
    arena*                            Arena;
    renderer*                         Renderer;
    glyph_cache*                      GlyphCache;
    gdi_handle<gdi_pipeline>          UIPipeline;
    gdi_handle<gdi_bind_group_layout> UIGlobalLayout;
    gdi_format                        Format;
    gdi_texture_usage_flags           UsageFlags;

    //Windows
    window* FirstWindow;
    window* LastWindow;
    window* FreeWindows;

    //todo: Panels and views
};

struct window_manager_create_info {
    allocator*                        Allocator;
    renderer*                         Renderer;
    glyph_cache*                      GlyphCache;
    gdi_handle<gdi_pipeline>          UIPipeline;
    gdi_handle<gdi_bind_group_layout> UIGlobalLayout;
    gdi_format                        Format;
    gdi_texture_usage_flags           UsageFlags;
};

void          Window_Manager_Create(window_manager* Manager, const window_manager_create_info& CreateInfo);
bool          Window_Is_Open(window_handle Handle);
window_handle Window_Open_With_Handle(window_manager* Manager, os_window_id WindowID, gdi_handle<gdi_swapchain> Swapchain);
window_handle Window_Open(window_manager* Manager, const os_open_window_info& OpenInfo);
void          Window_Close(window_manager* Manager, window_handle Handle);
window*       Window_Get(window_handle Handle);


#endif