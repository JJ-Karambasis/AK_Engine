window_handle::window_handle(window* _Window) {
    Assert(_Window);
    Window = _Window;
    Generation = Window->Generation;
}

void Window_Manager_Create(window_manager* Manager, const window_manager_create_info& CreateInfo) {
    Manager->Arena = Arena_Create(CreateInfo.Allocator);
    Manager->Renderer = CreateInfo.Renderer;
    Manager->GlyphCache = CreateInfo.GlyphCache;
    Manager->UIRenderPass = CreateInfo.UIRenderPass;
    Manager->UIPipeline = CreateInfo.UIPipeline;
    Manager->UIGlobalLayout = CreateInfo.UIGlobalLayout;
    Manager->Format = CreateInfo.Format;
    Manager->UsageFlags = CreateInfo.UsageFlags;
}

bool Window_Is_Open(window_handle Handle) {
    return Handle.Window && Handle.Generation && (Handle.Window->Generation == Handle.Generation);
}

window_handle Window_Open_With_Handle(window_manager* Manager, os_window_id WindowID, gdi_handle<gdi_swapchain> Swapchain) {
    window* Window = Manager->FreeWindows;
    if(Manager->FreeWindows) SLL_Pop_Front(Manager->FreeWindows);
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
    Array_Init(&Window->Framebuffers, Window->Arena);

    Window->UI = UI_Create({
        .Allocator = Window->Arena,
        .Renderer = Manager->Renderer,
        .GlyphCache = Manager->GlyphCache,
        .Pipeline = Manager->UIPipeline,
        .GlobalLayout = Manager->UIGlobalLayout
    });

    OS_Window_Set_Data(Window->OSHandle, Window);
    Window_Resize(Manager, Window);
    return window_handle(Window);
}

window_handle Window_Open(window_manager* Manager, const os_open_window_info& OpenInfo);

void Window_Close(window_manager* Manager, window_handle Handle) {
    window* Window = Window_Get(Handle);
    if(Window) {
        DLL_Remove(Manager->FirstWindow, Manager->LastWindow, Window);
        gdi_context* Context = Renderer_Get_Context(Manager->Renderer);

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
        Window->OSHandle = 0;
        Window->Swapchain = {};
        Window->SwapchainFormat = GDI_FORMAT_NONE;
        Window->SwapchainViews = {};
        Window->Framebuffers = {};
        Window->Generation++;

        SLL_Push_Front(Manager->FreeWindows, Window);
    }
}

window* Window_Get(window_handle Handle) {
    return Window_Is_Open(Handle) ? Handle.Window : nullptr;
}

void Window_Resize(window_manager* WindowManager, window* Window) {
    gdi_context* Context = Renderer_Get_Context(WindowManager->Renderer);
    if(Window->Framebuffers.Count) {
        for(gdi_handle<gdi_framebuffer> Framebuffer : Window->Framebuffers) {
            GDI_Context_Delete_Framebuffer(Context, Framebuffer);
        }
        Array_Clear(&Window->Framebuffers);
    }

    if(Window->SwapchainViews.Count) {
        for(gdi_handle<gdi_texture_view> SwapchainView : Window->SwapchainViews) {
            GDI_Context_Delete_Texture_View(Context, SwapchainView);
        }
        Array_Clear(&Window->SwapchainViews);
    }

    if(!GDI_Context_Resize_Swapchain(Context, Window->Swapchain)) {
        Assert(false);
        return;
    }
    
    Window->Size = GDI_Context_Get_Swapchain_Size(Context, Window->Swapchain);
    if(Window->Size.width == 0 || Window->Size.height == 0) return;

    span<gdi_handle<gdi_texture>> Textures = GDI_Context_Get_Swapchain_Textures(Context, Window->Swapchain);
    for(uptr i = 0; i < Textures.Count; i++) {
        Array_Push(&Window->SwapchainViews, GDI_Context_Create_Texture_View(Context, {
            .Texture = Textures[i]
        }));

        Array_Push(&Window->Framebuffers, GDI_Context_Create_Framebuffer(Context, {
            .Attachments = {Window->SwapchainViews[i]},
            .RenderPass = WindowManager->UIRenderPass
        }));
    }
}

bool Window_Is_Resizing(window* Window) {
    return OS_Window_Is_Resizing(Window->OSHandle);
}

bool Window_Is_Focused(window* Window) {
    return OS_Window_Is_Focused(Window->OSHandle);
}