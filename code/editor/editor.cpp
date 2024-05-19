#include "editor.h"

#include <shader_common.h>
#include <ui_box_shader.h>

void Fatal_Error_Message() {
    OS_Message_Box("A fatal error occurred during initialization!\nPlease view the error logs for more info.", "Error");
}

GDI_LOG_DEFINE(GDI_Log_Debug) {
    Log_Debug(modules::GDI, "%.*s", Message.Size, Message.Str);
}

GDI_LOG_DEFINE(GDI_Log_Info) {
    Log_Info(modules::GDI, "%.*s", Message.Size, Message.Str);
}

GDI_LOG_DEFINE(GDI_Log_Warning) {
    Log_Warning(modules::GDI, "%.*s", Message.Size, Message.Str);
    Assert(false);
}

GDI_LOG_DEFINE(GDI_Log_Error) {
    Log_Error(modules::GDI, "%.*s", Message.Size, Message.Str);
    Assert(false);
}

struct vtx_idx_p_uv {
    u32 P;
    u32 UV;
};

template <>
struct hasher<vtx_idx_p_uv> {
    inline u32 Hash(vtx_idx_p_uv V) {
        return Hash_Combine(V.P, V.UV);
    }
};

template <>
struct comparer<vtx_idx_p_uv> {
    inline bool Equal(vtx_idx_p_uv A, vtx_idx_p_uv B) {
        return A.P == B.P && A.UV == B.UV;
    }
};

window_handle::window_handle(window* _Window) {
    Assert(_Window);
    Window = _Window;
    Generation = Window->Generation;
}

internal void Window_Handle_Resize(editor* Editor, window* Window) {
    dim2i CurrentWindowSize = OS_Window_Get_Size(Window->WindowID);
    if(CurrentWindowSize.width != 0 && CurrentWindowSize.height != 0) {
        if(CurrentWindowSize != Window->Size) {
            gdi_context* Context = Renderer_Get_Context(Editor->Renderer);

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

            span<gdi_handle<gdi_texture>> Textures = GDI_Context_Get_Swapchain_Textures(Context, Window->Swapchain);
            for(uptr i = 0; i < Textures.Count; i++) {
                Array_Push(&Window->SwapchainViews, GDI_Context_Create_Texture_View(Context, {
                    .Texture = Textures[i]
                }));

                Array_Push(&Window->Framebuffers, GDI_Context_Create_Framebuffer(Context, {
                    .Attachments = {Window->SwapchainViews[i]},
                    .RenderPass = Editor->UIRenderPass.RenderPass
                }));
            }
        }
    }
    Window->Size = CurrentWindowSize;
}

inline bool Window_Is_Open(window_handle Handle) {
    return Handle.Window && Handle.Generation && (Handle.Window->Generation == Handle.Generation);
}

window_handle Window_Open(editor* Editor, os_monitor_id MonitorID, point2i Offset, vec2i Size, u32 Border, string Name) {
    window* Window = Editor->FreeWindows;
    if(Editor->FreeWindows) SLL_Pop_Front(Editor->FreeWindows);
    else {
        Window = Arena_Push_Struct(Editor->Arena, window);
        Window->Generation = 1;
    }

    DLL_Push_Back(Editor->FirstWindow, Editor->LastWindow, Window);

    Window->Arena = Arena_Create(Core_Get_Base_Allocator(), MB(1));
    Array_Init(&Window->SwapchainViews, Window->Arena);
    Array_Init(&Window->Framebuffers, Window->Arena);

    Window->WindowID = OS_Open_Window({
        .Flags = OS_WINDOW_FLAG_NONE,
        .MonitorID = MonitorID,
        .
        .XOffset = Offset.x,
        .YOffset = Offset.y,
        .Width = Size.x,
        .Height = Size.y,
        .Border = Border,
        .TargetFormat = GDI_FORMAT_R8G8B8A8_SRGB,
        .UsageFlags = GDI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT_BIT
    });

    Window->Swapchain = OS_Window_Get_Swapchain(Window->WindowID, &Window->SwapchainFormat);
    
    //Right now we must use a valid render target format that is srgb compatible, so
    //we can create srgb views for the swapchain. Not all monitors support this though,
    //so at some point we will need to implement a workaround for this
    Assert(GDI_Get_SRGB_Format(Window->SwapchainFormat) != GDI_FORMAT_NONE);

    if(Editor->UIRenderPass.RenderPass.Is_Null()) {
        //Make sure no threads can duplicate the same render pass
        scoped_lock Lock(&Editor->WindowLock);
        if(Editor->UIRenderPass.RenderPass.Is_Null()) {
            Editor->UIRenderPass = UI_Render_Pass_Create(Editor->Renderer, Window->SwapchainFormat);
        }
    }

    if(Editor->UIPipeline.Pipeline.Is_Null()) {
        //Make sure no threads can duplicate the same pipeline
        scoped_lock Lock(&Editor->WindowLock);
        if(Editor->UIPipeline.Pipeline.Is_Null()) {
            Editor->UIPipeline = UI_Pipeline_Create(Editor->Renderer, Editor->Packages, &Editor->UIRenderPass);
        }
    }

    Window->UI = UI_Create({
        .Allocator = Window->Arena, 
        .GlyphCache = Editor->GlyphCache, 
        .Renderer = Editor->Renderer, 
        .Pipeline = &Editor->UIPipeline
    }); 

    return window_handle(Window);
}

void Window_Close(editor* Editor, window_handle Handle) {
    window* Window = Window_Get(Handle);
    if(Window) {
        DLL_Remove(Editor->FirstWindow, Editor->LastWindow, Window);

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

#if 0
struct panel_draggable {
    panel*   PanelA;
    ui_box*  PanelDragBox;
    panel*   PanelB;
    s32      BoxPixels;
    ui_axis2 Axis;
};

void Window_Update_Panel_Leaf(editor* Editor, ui* UI, panel* Panel) {
    local_persist const f32 PanelMenuHeight = 2.5f;

    UI_Set_Next_Fixed_Width(PanelDim.x);
    UI_Set_Next_Height(UI, UI_EM(UI, PanelMenuHeight, 1.0f));
    UI_Set_Next_Child_Layout_Axis(UI_AXIS2_X);
    ui_box* MenuBox = UI_Build_Box_From_String(UI, 0, String_Lit("###PanelMenu"));
    UI_Push_Parent(UI, MenuBox);
    {
        bool PanelListerClicked = UI_Clicked(UI_Button_Image(UI, Editor->PanelListerTexture, String_Lit("###Panel Lister")));
        f32 PanelListerWidth = UI_Get_Last_Width(UI);
        if(PanelListerClicked) {
            //todo: Implement
        }

        for(view* View = Panel->FirstView; View; View = View->Next) {
            if(Panel == Panel->ChosenView) {

            }
        }

        if(UI_Clicked(UI_Button_Image(UI, Editor->XButtonTexture, String_Lit("###PanelClose")))) {
            Panel->ShouldClose = true;
        }
    }
    UI_Pop_Parent(UI, MenuBox);

    UI_Set_Next_Height
    Panel->ChosenView->DrawCallback(Editor, UI, Panel, ChosenView);
}

void Window_Update_Panel_Tree(editor* Editor, ui* UI, panel* Panel, vec2 PanelDim, array<panel_draggable>* PanelDraggables) {
    UI_Set_Next_Fixed_Width(UI, PanelDim.w);
    UI_Set_Next_Fixed_Height(UI, PanelDim.h);

    local_persist const s32 DraggableSize = 6;
    local_persist const s32 HalfDraggableSize = DraggableSize/2;
    
    if(Panel->ChildCount) {
        UI_Set_Next_Child_Layout_Axis(UI, Panel->SplitAxis);
        ui_box* Box = UI_Build_Box_From_StringF(UI, 0, "###%I64x", (u64)(uptr)Panel);
        UI_Push_Parent(UI, Box);

        if(Panel->SplitAxis == UI_AXIS2_X) {
            f32 RemainingWidth = PanelDim.x;
            for(panel* ChildPanel = Panel->FirstChild; ChildPanel; ChildPanel = ChildPanel->NextSibling) {
                f32 Width = PanelDim.x*ChildPanel->PercentOfParent;
                
                //Due to numerical precision issues with percentages and pixels
                //we will make sure to use all the remaining width for the last panel
                if(ChildPanel == Panel->LastChild) {
                    Width = RemainingWidth;
                } else {
                    RemainingWidth -= Width;

                    //If the panel is not the last panel, that means there is a draggable
                    UI_Set_Next_XOffset(UI, -HalfDraggableSize);
                    UI_Set_Next_Fixed_Width(UI, DraggableSize);
                    ui_box* PanelDragBox = UI_Build_Box_From_StringF(UI, UI_BOX_FLAG_DRAGGABLE, "###%I64x_drag", (u64)(uptr)ChildPanel);
                    UI_Set_Next_XOffset(UI, -HalfDraggableSize);

                    f32 NextWidth = PanelDim.x*ChildPanel->NextSibling->PercentOfParent;
                    if(ChildPanel->NextSibling == Panel->LastChild) {
                        NextWidth = RemainingWidth;
                    }

                    Array_Add(PanelDraggables, {
                        .PanelA = ChildPanel,
                        .DragBox = PanelDragBox,
                        .PanelB = ChildPanel->NextSibling,
                        .BoxPixels = Width+NextWidth,
                        .Axis = UI_AXIS2_X
                    })
                }
                Window_Update_Panel_Tree(UI, ChildPanel, vec2(Width, PanelDim.y), PanelDraggables);
            }
        } else if(Panel->SplitAxis == UI_AXIS2_Y) {
            f32 RemainingHeight = PanelDim.y;
            for(panel* ChildPanel = Panel->FirstChild; ChildPanel; ChildPanel = ChildPanel->NextSibling) {
                f32 Height = PanelDim.y*ChildPanel->PercentOfParent;
                
                //Due to numerical precision issues with percentages and pixels
                //we will make sure to use all the remaining height for the last panel
                if(ChildPanel == Panel->LastChild) {
                    Height = RemainingHeight;
                } else {
                    RemainingHeight -= Height;

                    //If the panel is not the last panel, that means there is a draggable
                    UI_Set_Next_YOffset(UI, -HalfDraggableSize);
                    UI_Set_Next_Fixed_Height(UI, DraggableSize);
                    ui_box* PanelDragBox = UI_Build_Box_From_StringF(UI, UI_BOX_FLAG_DRAGGABLE, "###%I64x_drag", (u64)(uptr)ChildPanel);
                    UI_Set_Next_YOffset(UI, -HalfDraggableSize);

                    f32 NextHeight = PanelDim.y*ChildPanel->NextSibling->PercentOfParent;
                    if(ChildPanel->NextSibling == Panel->LastChild) {
                        NextHeight = RemainingHeight;
                    }

                    Array_Add(PanelDraggables, {
                        .PanelA = ChildPanel,
                        .DragBox = PanelDragBox,
                        .PanelB = ChildPanel->NextSibling,
                        .BoxPixels = Height+NextHeight,
                        .Axis = UI_AXIS2_Y
                    })
                }

                Window_Update_Panel_Tree(UI, ChildPanel, vec2(PanelDim.x, Height), PanelDraggables);
            }
        }
    } else {
        /*Panels with no children have views and therefore a menu is needed. Y layout for the
          box is necessary*/
        UI_Set_Next_Child_Layout_Axis(UI, UI_AXIS2_Y);
        ui_box* Box = UI_Build_Box_From_StringF(UI, 0, "###%I64x", (u64)(uptr)Panel);
        UI_Push_Parent(UI, Box);

        Window_Update_Panel_Leaf(UI, Panel);
    }
}
#endif

void Window_Update(editor* Editor, window* Window) {
    Window_Handle_Resize(Editor, Window);

    ui* UI = Window->UI;
    UI_Begin_Build(UI, window_handle(Window));

    UI_Push_Font(UI, {
        .Font     = Editor->MainFont,
        .Script   = UBA_SCRIPT_LATIN,
        .Language = TEXT_LANGUAGE_ENGLISH
    });

    f32 ButtonSize = 100;

    UI_Push_Fixed_Height(UI, ButtonSize);
    UI_Set_Next_Fixed_Width(UI, (f32)Window->Size.w);
    UI_Set_Next_Child_Layout_Axis(UI, UI_AXIS2_X);
    UI_Push_Background_Color(UI, Vec4_Red());
    ui_box* MenuBox = UI_Build_Box_From_StringF(UI, 0, "###%llux", (u64)(uptr)Window);
    UI_Push_Parent(UI, MenuBox);
    {
        f32 MenuWidth = UI_Box_Get_Dim(MenuBox).w;

        UI_Set_Next_Background_Color(UI, Vec4_Yellow());
        UI_Set_Next_Text_Color(UI, Vec4_Blue());
        UI_Set_Next_Pref_Width(UI, UI_Text(4, 1.0));
        UI_Set_Next_Text_Alignment(UI, UI_TEXT_ALIGNMENT_CENTER);
        UI_Build_Box_From_String(UI, UI_BOX_FLAG_DRAW_TEXT, String_Lit("Help###Menu Box 1"));
        
        f32 Width = UI_Box_Get_Dim(UI_Current_Box(UI)).w;

        f32 FinalButtonsSize = ButtonSize*3;
        f32 NextRect = Max(0.0f, MenuWidth - (FinalButtonsSize+Width));

        UI_Set_Next_Fixed_Width(UI, NextRect);
        UI_Build_Box_From_String(UI, 0, String_Lit("###Filler Box"));

        UI_Push_Fixed_Width(UI, ButtonSize);
        
        UI_Set_Next_Background_Color(UI, Vec4_Blue());
        UI_Build_Box_From_String(UI, 0, String_Lit("###Menu Box 2"));

        UI_Set_Next_Background_Color(UI, Vec4_Green());
        UI_Build_Box_From_String(UI, 0, String_Lit("###Menu Box 3"));

        UI_Set_Next_Background_Color(UI, Vec4_Magenta());
        UI_Build_Box_From_String(UI, 0, String_Lit("###Menu Box 4"));

        UI_Pop_Fixed_Width(UI);
    }
    UI_Pop_Parent(UI);
    UI_Pop_Background_Color(UI);
    UI_Pop_Fixed_Height(UI);
    UI_Pop_Font(UI);
    
    UI_End_Build(UI);

#if 0 
    UI_Begin(UI);

    //Setup main window rect
    UI_Set_Next_Fixed_Width(UI, Window->Size.w);
    UI_Set_Next_Fixed_Height(UI, Window->Size.h);
    UI_Set_Next_Child_Layout_Axis(UI, UI_AXIS2_X);
    UI->Root = UI_Build_Box_From_StringF(UI, 0, "###%I64x", (u64)(uptr)Window);
    UI_Push_Parent(UI, UI->Root);

    //Setup main font
    UI_Push_Font(UI, Editor->UIFont);
    UI_Push_Font_Size(UI, Editor->FontSize);

    Window->UpdateCallback(Editor, Window, Window->UserData);

    if(Window->PanelBox) {
        scratch Scratch = Scratch_Get();
        array<panel_draggable> PanelDraggables(&Scratch); 

        UI_Push_Parent(UI, Window->PanelBox);
        vec2 RectDim = Rect2_To_Dim(Window->PanelBox->Rect);
        Window_Update_Panel_Tree(UI, Window->RootPanel, RectDim, &PanelDraggables);

        for(panel_draggable& Draggable : PanelDraggables) {
            ui_signal Interaction = UI_Signal_From_Box(Draggable.PanelDragBox);
            if(UI_Is_Dragging(Interaction)) {
                svec2 DragAmount = UI_Get_Drag_Delta(UI);
                s32 BoxPixels = Interaction.BoxPixels;

                f32 CombinedPercentage = Draggable.PanelA->PercentOfParent + Draggable.PanelB->PercentOfParent;
                f32 NormalizedPercentagePanelA = Draggable.PanelA->PercentOfParent/CombinedPercentage;
                
                s32 PixelsPanelA = (s32)((f32)BoxPixels*NormalizedPercentagePanelA);

                s32 DragPixels = 0;
                if(Draggable.Axis == UI_AXIS_X) {
                    DragPixels = DragAmount.x;
                } else if (Draggable.Axis == UI_AXIS_Y) {
                    DragPixels = DragAmount.y;
                } else {
                    Invalid_Code();
                }

                PixelsPanelA = Max(PixelsPanelA+DragPixels, BoxPixels-1);
                NormalizedPercentagePanelA = (f32)PixelsPanelA/(f32)BoxPixels;
                Draggable.PanelA->PercentOfParent = CombinedPercentage*NormalizedPercentagePanelA;
                Draggable.PanelB->PercentOfParent = CombinedPercentage-Draggable.PanelA->PercentOfParent;
            }
        }
    }

    local_persist const f32 MenuHeight = 2.5f;
    
    //Window menu rect
    UI_Set_Next_Height(UI, UI_EM(UI, MenuHeight, 1.0f));
    UI_Set_Next_Fixed_Width(UI, Window->Size.w);
    ui_box* MenuBox = UI_Build_Box_From_StringF(UI, 0, "###MainMenu");
    UI_Push_Parent(UI, MenuBox);
    {
        //Title box
        ui_box* TitleBox = UI_Build_Box_From_String(UI, UI_BOX_FLAG_DRAW_TEXT, "%.*s\n", Window->Title.Count, Window->Title.Str);
        f32 Remaining = (f32)(MenuBox->Rect.x1-TitleBox->Rect.x1);

        local_persist const f32 ButtonWidth = 30.0f;
    }
    UI_Pop_Parent(UI);


    //Tab editor rect
    ui_box* TabMenuBox = UI_Build_Box_From_String(UI, 0, "###TabMenu"); 
    
    rect2 WindowRect = OS_Get_Window_Rect(Editor, Window->WindowID);
    rect2 WindowMenuRect = rect2(WindowRect.x0, WindowRect.y0, WindowRect.x1, WindowRect.y0+MenuHeight);

    UI_End(UI);
#endif
}

bool Application_Main() {
    gdi* GDI = GDI_Create({
        .LoggingCallbacks = {
            .LogDebug   = GDI_Log_Debug,
            .LogInfo    = GDI_Log_Info,
            .LogWarning = GDI_Log_Warning,
            .LogError   = GDI_Log_Error
        },
        .AppInfo = {
            .Name = String_Lit("AK Engine"),
        }
    });

    if(!GDI) {
        Fatal_Error_Message();
        return false;
    }

    u32 DeviceCount = GDI_Get_Device_Count(GDI);
    if(!DeviceCount) {
        Fatal_Error_Message();
        return false;
    }

    //Right now we are just grabbing the first gpu. 
    //We probably want to choose dedicated gpus over integrated 
    //ones in the future
    gdi_device Device;
    GDI_Get_Device(GDI, &Device, 0);

    u32 TotalThreadCount = AK_Get_Processor_Thread_Count();
    u32 HighPriorityThreadCount = TotalThreadCount - (TotalThreadCount / 4);
    u32 LowPriorityThreadCount  = Max(TotalThreadCount-HighPriorityThreadCount, 1);


    editor Editor = {};
    Editor.Arena = Arena_Create(Core_Get_Base_Allocator());
    string ExecutablePath = OS_Get_Executable_Path();

    {
        scratch Scratch = Scratch_Get();
        packages_type PackageType = packages_type::FileSystem;
        string PackagePath;

#if EDITOR_PACKAGE_FILE_SYSTEM
        {
            PackageType = packages_type::FileSystem;
            PackagePath = String_Concat(&Scratch, {ExecutablePath, String_Lit("packages"), String_Lit(OS_FILE_DELIMITER_STR)});
        }
#else
    #error Not Implemented
#endif
        Editor.Packages = Packages_Create({
            .Type = PackageType,
            .RootPath = PackagePath,
            .NumIOThreads = TotalThreadCount
        });
    }

    Editor.JobSystemHigh = Core_Create_Job_System(1024, HighPriorityThreadCount, 1024);
    Editor.JobSystemLow = Core_Create_Job_System(1024, LowPriorityThreadCount, 0);

    Log_Info(modules::Editor, "Started creating GPU context for %.*s", Device.Name.Size, Device.Name.Str);
    gdi_context* Context = GDI_Create_Context(GDI, {});
    if(!Context) {
        Fatal_Error_Message();
        return false;
    }

    Editor.Renderer = Renderer_Create({
        .JobSystem = Editor.JobSystemHigh,
        .Context = Context,
        .DefaultSamplerInfo = {
            .Filter = GDI_FILTER_LINEAR
        }
    });
    if(!Editor.Renderer) {
        Fatal_Error_Message();
        return false;
    }

    Editor.FontManager = Font_Manager_Create({});

    resource* FontResource = Packages_Get_Resource(Editor.Packages, String_Lit("fonts"), String_Lit("liberation mono"), String_Lit("regular"));
    Editor.MainFontBuffer = Packages_Load_Entire_Resource(Editor.Packages, FontResource, Editor.Arena);
    Editor.MainFont = Font_Manager_Create_Font(Editor.FontManager, Editor.MainFontBuffer, 40);

    {
        os_monitor_id MonitorID = OS_Get_Primary_Monitor();
        os_window_id  MainWindowID = OS_Open_Window({
            .Flags = OS_WINDOW_FLAG_MAIN_BIT|OS_WINDOW_FLAG_MAXIMIZE_BIT
        });
        if(!MainWindowID) {
            Fatal_Error_Message();
            return false;
        }

        gdi_format TargetFormat = GDI_FORMAT_R8G8B8A8_SRGB;

        scratch Scratch = Scratch_Get();
        array<gdi_format> WindowFormats = GDI_Context_Supported_Window_Formats(OS->GDIContext, OS_Window_Get_GDI_Data(MainWindowID), Scratch.Arena);
        gdi_format WindowFormat = GDI_FORMAT_NONE;
        
        for(gdi_format Format : WindowFormats) {
            if(Format == TargetFormat) {
                WindowFormat = Format;
                break;
            }
        }

        if(WindowFormat == GDI_FORMAT_NONE) {
            WindowFormat = WindowFormats[0];
        }

        gdi_texture_usage_flags UsageFlags = GDI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT_BIT; 
        gdi_handle<gdi_swapchian> Swapchain = GDI_Context_Create_Swapchain(GDIContext, {
            .WindowData   = OS_Window_Get_GDI_Data(MainWindowID)
            .TargetFormat = WindowFormat,
            .UsageFlags   = UsageFlags
        });

        if(Swapchain.Is_Null()) {
            Fatal_Error_Message();
            return false;
        }

        Editor->UIRenderPass = UI_Render_Pass_Create(Editor->Renderer, WindowFormat);
        Editor->UIPipeline = UI_Pipeline_Create(Editor->Renderer, Editor->Packages, &Editor->UIRenderPass);
        Window_Manager_Create(&Editor->WindowManager, {
            .Allocator = Core_Get_Base_Allocator(),
            .Context = GDIContext,
            .Pipeline = Editor->UIPipeline,
            .Format = WindowFormat,
            .UsageFlags = UsageFlags
        });

        Window_Open_With_Handle(&Editor->WindowManager, MainWindowID, Swapchain);
    }
    
    editor_input_manager* InputManager = &Editor.InputManager;
    
    Editor.GlyphCache = Glyph_Cache_Create({
        .Renderer = Editor.Renderer
    });
    
    if(!Editor.GlyphCache) {
        Fatal_Error_Message();
        return false;
    }

    //Default texture is pure white
    u32 DefaultTextureTexels[4] = {
        0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF
    };

    // Editor.DefaultTexture = GPU_Texture_Create(Editor.GDIContext, {
    //     .Format = GDI_FORMAT_R8G8B8A8_UNORM,
    //     .Dim = uvec2(2, 2),
    //     .Texels = const_buffer(DefaultTextureTexels),
    //     .BindGroupLayout = Editor.LinearSamplerBindGroupLayout
    // });


    u64 Frequency = AK_Query_Performance_Frequency();
    u64 LastCounter = AK_Query_Performance_Counter();

    while(OS_Window_Is_Open(OS_Get_Main_Window())) {
        u64 StartCounter = AK_Query_Performance_Counter();
        f64 dt = (double)(StartCounter-LastCounter)/(double)Frequency;
        LastCounter = StartCounter;
        Editor_Input_Manager_New_Frame(InputManager, dt);

        for(u32 KeyIndex = 0; KeyIndex < OS_KEYBOARD_KEY_COUNT; KeyIndex++) {
            if(OS_Keyboard_Get_Key_State((os_keyboard_key)KeyIndex)) {
                InputManager->KeyboardInput[KeyIndex].IsDown = true;
            }
        }

        for(u32 KeyIndex = 0; KeyIndex < OS_MOUSE_KEY_COUNT; KeyIndex++) {
            if(OS_Mouse_Get_Key_State((os_mouse_key)KeyIndex)) {
                InputManager->MouseInput[KeyIndex].IsDown = true;
            }
        }

        while(const os_event* Event = OS_Next_Event()) {
            switch(Event->Type) {
                case OS_EVENT_TYPE_WINDOW_CLOSED: {
                    window* Window = OS_Window_Get_Data(Event->Window);
                    Window_Close(&Editor.WindowManager, window_handle(Window));
                } break;

                case OS_EVENT_TYPE_MOUSE_DELTA: {
                    const os_mouse_delta_event* DeltaEvent = (const os_mouse_delta_event*)Event;
                    InputManager->MouseDelta = DeltaEvent->Delta;
                } break;

                case OS_EVENT_TYPE_MOUSE_SCROLL: {
                    const os_mouse_scroll_event* ScrollEvent = (const os_mouse_scroll_event*)Event;
                    InputManager->Scroll = ScrollEvent->Scroll;
                } break;
            }
        }

        for(window* Window = Editor.FirstWindow; Window; Window = Window->Next) {
            Window_Update(&Editor, Window);
        }

        render_graph_id RenderGraph = Renderer_Create_Graph(Editor.Renderer);
        
        scratch Scratch = Scratch_Get();
        array<window_handle> Windows(&Scratch);
        array<gdi_handle<gdi_swapchain>> Swapchains(&Scratch);
        for(window* Window = Editor.FirstWindow; Window; Window = Window->Next) {
            s32 Texture = -1;
            do {
                Texture = GDI_Context_Get_Swapchain_Texture_Index(Context, Window->Swapchain);
                if(Texture == -1) {
                    gdi_swapchain_status PresentStatus = GDI_Context_Get_Swapchain_Status(Context, Window->Swapchain); 
                    if(PresentStatus == GDI_SWAPCHAIN_STATUS_RESIZE) {
                        Window_Handle_Resize(&Editor, Window);    
                    } else if (PresentStatus != GDI_SWAPCHAIN_STATUS_OK) {
                        Assert(false);
                        //todo: Actual error logging
                        return false;
                    }
                }
            } while(Texture == -1);

            Assert(Texture >= 0);

            ui_renderer* UIRenderer = &Window->UI->Renderer;

            Render_Task_Attach_Render_Pass(Editor.Renderer, UIRenderer->RenderTask, {
                .RenderPass = Editor.UIRenderPass.RenderPass,
                .Framebuffer = Window->Framebuffers[(uptr)Texture],
                .ClearValues = {
                    gdi_clear::Color(0.0f, 0.0f, 1.0f, 1.0f)
                }
            },
            {GDI_RESOURCE_STATE_COLOR});

            Array_Push(&Windows, window_handle(Window));
            Array_Push(&Swapchains, Window->Swapchain);

            Render_Graph_Add_Task(RenderGraph, UIRenderer->RenderTask, 0);
        }
        
        Glyph_Cache_Update(Editor.GlyphCache);
        while(!Renderer_Execute(Editor.Renderer, RenderGraph, Swapchains)) {
            bool HandledError = false;
            for(window_handle WindowHandle : Windows) {
                window* Window = Window_Get(WindowHandle);
                gdi_swapchain_status PresentStatus = GDI_Context_Get_Swapchain_Status(Context, Window->Swapchain); 
                if(PresentStatus == GDI_SWAPCHAIN_STATUS_RESIZE) {
                    Window_Handle_Resize(&Editor, Window);
                    HandledError = true;    
                } else if (PresentStatus != GDI_SWAPCHAIN_STATUS_OK) {
                    Assert(false);
                    //todo: Actual error logging
                    return false;
                }
            }

            if(!HandledError) {
                Assert(false);
                //todo: Actual error logging
                return false;
            }
        }

        Renderer_Delete_Graph(Editor.Renderer, RenderGraph);
    }

    AK_Job_System_Delete(Editor.JobSystemHigh);
    AK_Job_System_Delete(Editor.JobSystemLow);
    OS_Delete();

    GDI_Context_Delete(Context);
    Arena_Delete(Editor.Arena);
    GDI_Delete(GDI);
    return true;
}

// void Window_Update_And_Render(editor* Editor, window* Window) {
//     gdi_context* GDIContext = Window->GDIContext;
//     gdi_cmd_list* CmdList = GDI_Context_Begin_Cmd_List(GDIContext, gdi_cmd_list_type::Graphics, Window->Swapchain); 
//     Window->CmdList = GDI_Context_Begin_Cmd_List(GDIContext, gdi_cmd_list_type::Graphics);
    
//     u32 TextureIndex = GDI_Cmd_List_Get_Swapchain_Texture_Index(CmdList, GDI_RESOURCE_STATE_COLOR);
//     span<gdi_handle<gdi_texture>> SwapchainTextures = GDI_Context_Get_Swapchain_Textures(GDIContext, Window->Swapchain);

//     GDI_Cmd_List_Barrier(CmdList, {
//         {gdi_resource::Texture(SwapchainTextures[TextureIndex]), GDI_RESOURCE_STATE_NONE, GDI_RESOURCE_STATE_COLOR}, 
//     });

//     GDI_Cmd_List_Begin_Render_Pass(CmdList, {
//         .RenderPass = Editor->UIRenderPass,
//         .Framebuffer = Window->SwapchainFramebuffers[TextureIndex],
//         .ClearValues = { 
//             gdi_clear::Color(0.0f, 0.0f, 1.0f, 0.0f)
//         }
//     });

//     //todo: Render common window properties here

//     GDI_Cmd_List_Execute_Cmds(CmdList, {Window->CmdList});

//     GDI_Cmd_List_End_Render_Pass(CmdList);
//     GDI_Cmd_List_Barrier(CmdList, {
//         {gdi_resource::Texture(SwapchainTextures[TextureIndex]), GDI_RESOURCE_STATE_COLOR, GDI_RESOURCE_STATE_PRESENT}
//     });
// }

//#include "level_editor/level_editor.cpp"
#include "windows.cpp"
#include "ui/ui.cpp"
#include "editor_input.cpp"
#include <engine_source.cpp>
#include <core/core.cpp>

#pragma comment(lib, "ftsystem.lib")
#pragma comment(lib, "hb.lib")
#pragma comment(lib, "sheenbidi.lib")

#if defined(OS_WIN32)
#pragma comment(lib, "user32.lib")
#endif