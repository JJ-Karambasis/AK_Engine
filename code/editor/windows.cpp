window_handle::window_handle(window* _Window) {
    Assert(_Window);
    Window = _Window;
    Generation = Window->Generation;
}

bool window_input::Is_Key_Down(os_keyboard_key Key) {
    Assert(Key < OS_KEYBOARD_KEY_COUNT);
    return KeyboardInput[Key].Is_Down();
}

bool window_input::Is_Key_Pressed(os_keyboard_key Key) {
    Assert(Key < OS_KEYBOARD_KEY_COUNT);
    return KeyboardInput[Key].Is_Pressed();
}

bool window_input::Is_Key_Released(os_keyboard_key Key) {
    Assert(Key < OS_KEYBOARD_KEY_COUNT);
    return KeyboardInput[Key].Is_Released();
}

bool window_input::Is_Mouse_Down(os_mouse_key Key) {
    Assert(Key < OS_MOUSE_KEY_COUNT);
    return MouseInput[Key].Is_Down();
}

bool window_input::Is_Mouse_Pressed(os_mouse_key Key) {
    Assert(Key < OS_MOUSE_KEY_COUNT);
    return MouseInput[Key].Is_Pressed();
}

bool window_input::Is_Mouse_Released(os_mouse_key Key) {
    Assert(Key < OS_MOUSE_KEY_COUNT);
    return MouseInput[Key].Is_Released();
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
    Window->Input.MousePosition = point2i(-20000, -20000);
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

    Window->Counter = AK_Query_Performance_Counter();
}

bool Window_Is_Resizing(window* Window) {
    return OS_Window_Is_Resizing(Window->OSHandle);
}

bool Window_Is_Focused(window* Window) {
    return OS_Window_Is_Focused(Window->OSHandle);
}

void Window_New_Frame(window* Window) {
    window_input* Input = &Window->Input;
    
    u64 NewCounter = AK_Query_Performance_Counter();
    u64 Frequency = AK_Query_Performance_Frequency();
    Input->dt = (f64)(NewCounter - Window->Counter)/(f64)Frequency;
    Input->dt = Min(Input->dt, INPUT_DT_MAX);
    Window->Counter = NewCounter;

    for(input& KeyboardInput : Input->KeyboardInput) { Input_New_Frame(&KeyboardInput); }
    for(input& MouseInput : Input->MouseInput) { Input_New_Frame(&MouseInput); }
    Input->MouseScroll = 0.0f;
}

internal void Windows_Render_Internal(window_manager* WindowManager, span<window*> WindowsToRender, render_graph_id RenderGraph) {
    scratch Scratch = Scratch_Get();
    gdi_context* Context = Renderer_Get_Context(WindowManager->Renderer);

    fixed_array<gdi_handle<gdi_swapchain>> Swapchains(&Scratch, WindowsToRender.Count);
    for(uptr i = 0; i < WindowsToRender.Count; i++) {
        window* Window = WindowsToRender[i];
        Assert(Window->Size.width != 0 && Window->Size.height != 0); 

         s32 Texture = GDI_Context_Get_Swapchain_Texture_Index(Context, Window->Swapchain);
        if(Texture == -1) {
            return;
        }

        Assert(Texture >= 0);

        im_renderer* UIRenderer = Window->UI->Renderer;

        Render_Task_Attach_Render_Pass(WindowManager->Renderer, UIRenderer->RenderTask, {
            .RenderPass = WindowManager->UIRenderPass,
            .Framebuffer = Window->Framebuffers[(uptr)Texture],
            .ClearValues = {
                gdi_clear::Color(0.0f, 0.0f, 1.0f, 1.0f)
            }
        },
        {GDI_RESOURCE_STATE_COLOR});

        Swapchains[i] = Window->Swapchain;
        Render_Graph_Add_Task(RenderGraph, UIRenderer->RenderTask, 0);
    }

    Renderer_Execute(WindowManager->Renderer, RenderGraph, Swapchains);
}

void Windows_Render(window_manager* WindowManager, span<window*> WindowsToRender) {
    render_graph_id RenderGraph = Renderer_Create_Graph(WindowManager->Renderer);
    Windows_Render_Internal(WindowManager, WindowsToRender, RenderGraph);
    Renderer_Delete_Graph(WindowManager->Renderer, RenderGraph);
}