#ifndef WINDOWS_H
#define WINDOWS_H

#define INPUT_DT_MAX 0.2

struct ui;
struct ui_pipeline;
struct window;
struct window_handle {
    window* Window;
    u64     Generation;

    window_handle() = default;
    window_handle(window* Window);
};

struct window_input {
    input   KeyboardInput[OS_KEYBOARD_KEY_COUNT];
    input   MouseInput[OS_MOUSE_KEY_COUNT];
    point2i MousePosition;
    vec2    MouseDelta;
    f32     MouseScroll;
    f64     dt;

    bool Is_Key_Down(os_keyboard_key Key);
    bool Is_Key_Pressed(os_keyboard_key Key);
    bool Is_Key_Released(os_keyboard_key Key);

    bool Is_Mouse_Down(os_mouse_key Key);
    bool Is_Mouse_Pressed(os_mouse_key Key);
    bool Is_Mouse_Released(os_mouse_key Key);
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
    u64                                 Counter;
    window_input                        Input;

    //Window links
    window* Prev;
    window* Next;
};

struct window_manager {
    arena*                            Arena;
    renderer*                         Renderer;
    glyph_cache*                      GlyphCache;
    gdi_handle<gdi_render_pass>       UIRenderPass;
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
    gdi_handle<gdi_render_pass>       UIRenderPass;
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
void          Window_Resize(window_manager* Manager, window* Window);
bool          Window_Is_Resizing(window* Window);
bool          Window_Is_Focused(window* Window);
void          Window_New_Frame(window* Window);
void          Windows_Render(window_manager* Manager, span<window*> WindowsToRender);

#endif