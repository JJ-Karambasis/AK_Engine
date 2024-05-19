void Window_Manager_Create(window_manager* Manager, const window_manager_create_info& CreateInfo) {
    Manager->Arena = Arena_Create(CreateInfo.Allocator);
    Manager->Renderer = CreateInfo.Renderer;
    Manager->Pipeline = CreateInfo.Pipeline;
    Manager->Format = CreateInfo.Format;
    Manager->UsageFlags = CreateInfo.UsageFlags;
}

bool Window_Is_Open(window_handle Handle) {
    return Handle.Window && Handle.Generation && (Handle.Window->Generation == Handle.Generation);
}

window_handle Window_Open_With_Handle(window_manager* Manager, os_window_id WindowID, gdi_handle<gdi_swapchain> Swapchain) {
    window* Window = Manager->FreeWindows;
    if(Editor->FreeWindows) SLL_Pop_Front(Manager->FreeWindows);
    else {
        Window = Arena_Push_Struct(Manager->Arena, window);
        Window->Generation = 1;
    }

    DLL_Push_Back(Manager->FirstWindow, Manager->LastWindow, Window);

    Window->Arena           = Arena_Create(Core_Get_Base_Allocator());
    Window->OSHandle        = WindowID;
    Window->Swapchain       = Swapchain;
    Window->SwapchainFormat = Manager->Format;
    Array_Init(&Window->SwapchainViews, Window->Arena);
    Array_Init(&Window->Framebuffers, WIndow->Arena);

    Window->UI = UI_Create({
        .Allocator = Window->Arena,
        .GlyphCache = Manager->GlyphCache,
        .Renderer = Manager->Renderer,
        .Pipeline = Manager->Pipeline
    });
}

window_handle Window_Open(window_manager* Manager, const os_open_window_info& OpenInfo);
void Window_Close(window_manager* Manager, window_handle Handle) {
    window* Window = Window_Get(Handle);
    if(Window) {
        DLL_Remove(Manager->FirstWindow, Manager->LastWindow, Window);
        gdi_context* Context = Renderer_Get_Context(Editor->Renderer);

        if(Window->Framebuffers.Count) {
            for(gdi_handle<gdi_framebuffer> Framebuffer : Window->Framebuffers) {
                GDI_Context_Delete_Framebuffer(Context, Framebuffer);
            }
        }

        if(Window->SwapchainViews.Count) {
            for(gdi_handle<gdi_texture_view> SwapchainView : Window->SwapchainViews) {
                GDI_Context_Delete_Texture_View(Context, SwapchainView);
            }
        }

        GDI_Context_Delete_Swapchain(Context, Window->Swapchain);
        OS_Close_Window(Window->OSHandle);        
        Arena_Delete(Window->Arena);

        Window->Arena = nullptr;
        Window->WindowID = 0;
        Window->Swapchain = {};
        Window->SwapchainFormat = GDI_FORMAT_NONE;
        Window->SwapchainViews = {};
        Window->Framebuffers = {};
        Window->Generation++;

        SLL_Push_Front(Editor->FreeWindows, Window);
    }
}

window* Window_Get(window_handle Handle) {
    return Window_Is_Open(Handle) ? Handle.Window : nullptr;
}