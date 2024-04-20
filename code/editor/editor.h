#ifndef EDITOR_H
#define EDITOR_H

#include <engine.h>
#include "editor_modules.h"
#include "os/os.h"
#include "editor_input.h"
#include "ui/ui.h"
#include "level_editor/level_editor.h"

struct view {
    view* Next;
    view* Prev;
};

struct panel {
    //Tree hierarchy
    panel* FirstChild;
    panel* LastChild;
    panel* NextSibling;
    panel* PrevSibling;
    panel* Parent;

    //Split data
    ui_axis2 SplitAxis;
    f32      PercentOfParent;

    //Views
    view* FirstView;
    view* LastView;
};

struct window {
    arena*                              Arena;
    os_window_id                        WindowID;
    gdi_handle<gdi_swapchain>           Swapchain;
    array<gdi_handle<gdi_texture_view>> SwapchainViews;
    array<gdi_handle<gdi_framebuffer>>  Framebuffers;
    ui                                  UI;
    panel*                              RootPanel;
};

struct editor {
    arena*                      Arena;
    gdi_context*                GDIContext;
    gdi_handle<gdi_render_pass> UIRenderPass;
    
    array<os_event_subscriber> OSEventSubscribers;
    editor_input_manager       InputManager;
    
    window* FirstWindow;
    window* LastWindow;
    window* FreeWindows;

    panel* FreePanels;
    view*  FreeViews;
};

void Window_Update_And_Render(editor* Editor, window* Window);

#endif