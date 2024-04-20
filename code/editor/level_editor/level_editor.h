#ifndef LEVEL_EDITOR_H
#define LEVEL_EDITOR_H

struct level_editor {
    heap*                               Heap;
    arena*                              Arena;
    gdi_context*                        GDIContext;
    os_window_id                        WindowID;
    gdi_handle<gdi_swapchain>           Swapchain;
    gdi_handle<gdi_render_pass>         UIRenderPass;
    array<gdi_handle<gdi_texture_view>> SwapchainViews;
    array<gdi_handle<gdi_framebuffer>>  SwapchainFramebuffers;
    ui                                  UI;

    /*Dynamic per frame data*/
    b32   IsOpen;
    uvec2 WindowSize;
};

struct level_editor_create_info {
    gdi_context*                GDIContext;
    array<os_event_subscriber>* EventSubscribers;
    os_window_id                WindowID;
    gdi_handle<gdi_render_pass> UIRenderPass;
};

bool Level_Editor_Init(level_editor* LevelEditor, const level_editor_create_info& CreateInfo);
bool Level_Editor_Is_Open(level_editor* LevelEditor);
void Level_Editor_Update(level_editor* LevelEditor);
void Level_Editor_Shutdown(level_editor* LevelEditor);

#endif