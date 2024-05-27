#include "editor.h"

#include <shader_common.h>
#include <ui_shader.h>

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

internal void Create_UI_Handles(editor* Editor, gdi_format Format) {
    gdi_context* Context = Renderer_Get_Context(Editor->Renderer);
    gdi_handle<gdi_bind_group_layout> TextureLayout = Renderer_Get_Texture_Layout(Editor->Renderer);

    Editor->UIRenderPass = GDI_Context_Create_Render_Pass(Context, {
        .Attachments = {
            gdi_render_pass_attachment::Color(Format, GDI_LOAD_OP_CLEAR, GDI_STORE_OP_STORE)
        }
    });

    packages* Packages = Editor->Packages;

    scratch Scratch = Scratch_Get();
    package* UIShaderPackage = Packages_Get_Package(Packages, String_Lit("shaders"), String_Lit("ui")); 
    resource* UIVtxShaderResource = Packages_Get_Resource(Packages, UIShaderPackage, String_Lit("ui_vtx"));
    resource* UIPxlShaderResource = Packages_Get_Resource(Packages, UIShaderPackage, String_Lit("ui_pxl"));

    const_buffer VtxShader = Packages_Load_Entire_Resource(Packages, UIVtxShaderResource, &Scratch);
    const_buffer PxlShader = Packages_Load_Entire_Resource(Packages, UIPxlShaderResource, &Scratch);

    Editor->UIPipeline = GDI_Context_Create_Graphics_Pipeline(Context, {
        .VS = {VtxShader, String_Lit("VS_Main")},
        .PS = {PxlShader, String_Lit("PS_Main")},
        .Layouts = {Editor->GlobalLayout, TextureLayout},
        .GraphicsState = {
            .VtxBufferBindings = { {
                    .ByteStride = sizeof(ui_vertex),
                    .InputRate = GDI_VTX_INPUT_RATE_VTX,
                    .Attributes = { { 
                            .Semantic = String_Lit("Position"),
                            .ByteOffset = offsetof(ui_vertex, P),
                            .Format = GDI_FORMAT_R32G32_FLOAT
                        }, {
                            .Semantic = String_Lit("Texcoord"),
                            .ByteOffset = offsetof(ui_vertex, UV),
                            .Format = GDI_FORMAT_R32G32_FLOAT
                        }, {
                            .Semantic = String_Lit("Color"),
                            .ByteOffset = offsetof(ui_vertex, C),
                            .Format = GDI_FORMAT_R32G32B32A32_FLOAT
                        }
                    }
                }
            },
            .Topology = GDI_TOPOLOGY_TRIANGLE_LIST,
            .BlendStates = { { 
                    .BlendEnabled = true,
                    .SrcColor = GDI_BLEND_ONE,
                    .DstColor = GDI_BLEND_ONE_MINUS_SRC_ALPHA
                }
            }
        },
        .RenderPass = Editor->UIRenderPass
    });
}

internal void Window_Resize(editor* Editor, window* Window) {
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
    
    Window->Size = GDI_Context_Get_Swapchain_Size(Context, Window->Swapchain);
    if(Window->Size.width == 0 || Window->Size.height == 0) return;

    span<gdi_handle<gdi_texture>> Textures = GDI_Context_Get_Swapchain_Textures(Context, Window->Swapchain);
    for(uptr i = 0; i < Textures.Count; i++) {
        Array_Push(&Window->SwapchainViews, GDI_Context_Create_Texture_View(Context, {
            .Texture = Textures[i]
        }));

        Array_Push(&Window->Framebuffers, GDI_Context_Create_Framebuffer(Context, {
            .Attachments = {Window->SwapchainViews[i]},
            .RenderPass = Editor->UIRenderPass
        }));
    }

}

internal void Window_Handle_Resize(editor* Editor, window* Window) {
    dim2i CurrentWindowSize = OS_Window_Get_Size(Window->OSHandle);
    if(CurrentWindowSize != Window->Size) {
        Window_Resize(Editor, Window);
        Assert(CurrentWindowSize == Window->Size);
    }
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

void Window_Update_UI(editor* Editor, window* Window) {
    ui* UI = Window->UI;

    point2i GlobalMousePos = OS_Mouse_Get_Position();
    point2i WindowMousePos = GlobalMousePos - vec2i(OS_Window_Get_Client_Pos(Window->OSHandle));

    UI_Begin_Build(UI, Window);

    UI_Push_Font(UI, {
        .Font     = Editor->MainFont,
        .Script   = UBA_SCRIPT_LATIN,
        .Language = TEXT_LANGUAGE_ENGLISH
    });

    f32 ButtonSize = 100;

    UI_Push_Fixed_Height(UI, ButtonSize);
    UI_Set_Next_Fixed_Width(UI, (f32)Window->Size.width);
    UI_Set_Next_Child_Layout_Axis(UI, UI_AXIS2_X);
    UI_Push_Background_Color(UI, Color4_Red());
    ui_box* MenuBox = UI_Build_Box_From_StringF(UI, 0, "###%llu_Menu", (u64)(uptr)Window);
    UI_Push_Parent(UI, MenuBox);
    {
        f32 MenuWidth = UI_Box_Get_Dim(MenuBox).width;

        UI_Set_Next_Background_Color(UI, Color4_Yellow());
        UI_Set_Next_Text_Color(UI, Color4_Blue());
        UI_Set_Next_Pref_Width(UI, UI_Text(4, 1.0));
        UI_Set_Next_Text_Alignment(UI, UI_TEXT_ALIGNMENT_CENTER);
        UI_Build_Box_From_String(UI, UI_BOX_FLAG_DRAW_TEXT|UI_BOX_FLAG_MOUSE_CLICKABLE, String_Lit("Help###Menu Box 1"));

        f32 Width = UI_Box_Get_Width(UI_Current_Box(UI));
        if(UI_Hovering(UI_Current_Signal(UI))) {
            UI_Current_Box(UI)->BackgroundColor = Color4_Green();
        }

        if(UI_Pressed(UI_Current_Signal(UI))) {
            Log_Debug_Simple("Pressed");
        }

        if(UI_Released(UI_Current_Signal(UI))) {
            Log_Debug_Simple("Released");
        }

        if(UI_Down(UI_Current_Signal(UI))) {
            UI_Set_Next_Background_Color(UI, Color4_Orange());
            UI_Set_Next_Text_Color(UI, Color4_Blue());
            UI_Set_Next_Pref_Width(UI, UI_Text(4, 1.0));
            UI_Set_Next_Text_Alignment(UI, UI_TEXT_ALIGNMENT_CENTER);
            UI_Build_Box_From_String(UI, UI_BOX_FLAG_DRAW_TEXT|UI_BOX_FLAG_MOUSE_CLICKABLE, String_Lit("This is down###Menu Box Text"));
            Width += UI_Box_Get_Width(UI_Current_Box(UI));
        }

        f32 FinalButtonsSize = ButtonSize*3;
        f32 NextRect = Max(0.0f, MenuWidth - (FinalButtonsSize+Width));

        UI_Set_Next_Fixed_Width(UI, NextRect);
        UI_Build_Box_From_String(UI, 0, String_Lit("###Filler Box"));
        
        UI_Push_Fixed_Width(UI, ButtonSize);

        UI_Set_Next_Background_Color(UI, Color4_Blue());
        UI_Build_Box_From_String(UI, 0, String_Lit("###Menu Box 2"));

        UI_Set_Next_Background_Color(UI, Color4_Green());
        UI_Build_Box_From_String(UI, 0, String_Lit("###Menu Box 3"));

        UI_Set_Next_Background_Color(UI, Color4_Magenta());
        UI_Build_Box_From_String(UI, 0, String_Lit("###Menu Box 4"));

        UI_Pop_Fixed_Width(UI);
    }
    UI_Pop_Parent(UI);
    UI_Pop_Background_Color(UI);
    UI_Pop_Fixed_Height(UI);
    UI_Pop_Font(UI);
    
    UI_End_Build(UI);
}

void Window_Update(editor* Editor, window* Window) {
    Window_Handle_Resize(Editor, Window);

    

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

void Editor_Update_And_Render(editor* Editor, span<window_handle> WindowsToUpdate, bool ProcessInputs) {
    scratch Scratch = Scratch_Get();
    window_manager* WindowManager = &Editor->WindowManager;
      
    //First reset all the window inputs and check if we need to resize
    for(window_handle WindowHandle : WindowsToUpdate) {
        window* Window = Window_Get(WindowHandle);
        Window_New_Frame(Window);

        dim2i CurrentWindowSize = OS_Window_Get_Size(Window->OSHandle);
        if(CurrentWindowSize != Window->Size) {
            Window_Resize(WindowManager, Window);
            Assert(CurrentWindowSize == Window->Size);
        }
    }

    //Next process inputs if we are rendering normally 
    if(ProcessInputs) {
        while(const os_event* Event = OS_Next_Event()) {
            window* Window = (window*)OS_Window_Get_Data(Event->Window);
            switch(Event->Type) {
                case OS_EVENT_TYPE_WINDOW_CLOSED: {
                    Window_Close(WindowManager, window_handle(Window));
                } break;

                case OS_EVENT_TYPE_MOUSE_SCROLL: {
                    const os_mouse_scroll_event* ScrollEvent = (const os_mouse_scroll_event*)Event;
                    Window->Input.MouseScroll = ScrollEvent->Scroll;
                } break;

                case OS_EVENT_TYPE_KEY_PRESSED: {
                    const os_keyboard_event* KeyboardEvent = (const os_keyboard_event*)Event;
                    Window->Input.KeyboardInput[KeyboardEvent->Key].IsDown = true;
                } break;

                case OS_EVENT_TYPE_KEY_RELEASED: {
                    const os_keyboard_event* KeyboardEvent = (const os_keyboard_event*)Event;
                    Window->Input.KeyboardInput[KeyboardEvent->Key].IsDown = false;
                } break;

                case OS_EVENT_TYPE_MOUSE_PRESSED: {
                    const os_mouse_event* MouseEvent = (const os_mouse_event*)Event;
                    Window->Input.MouseInput[MouseEvent->Key].IsDown = true;
                } break;

                case OS_EVENT_TYPE_MOUSE_RELEASED: {
                    const os_mouse_event* MouseEvent = (const os_mouse_event*)Event;
                    Window->Input.MouseInput[MouseEvent->Key].IsDown = false;
                } break;

                case OS_EVENT_TYPE_MOUSE_MOVE: {
                    const os_mouse_move_event* MoveEvent = (const os_mouse_move_event*)Event;
                    Window->Input.MousePosition = MoveEvent->Pos;
                } break;

                case OS_EVENT_TYPE_MOUSE_EXITED: {
                    //Clear all keyboard data when mouse has exited since sometimes
                    Window->Input.MousePosition = point2i(-20000, -20000);
                } break;
            }
        }
    }

    //Now update the window
    array<window*> WindowsToRender(&Scratch, WindowsToUpdate.Count);
    for(window_handle WindowHandle : WindowsToUpdate) {
        //Window can be closed after event processing so make sure the window 
        //is still valid using the handle
        window* Window = Window_Get(WindowHandle);
        if(Window && Window->Size.width != 0 && Window->Size.height != 0) {
            Window_Update_UI(Editor, Window);
            Array_Push(&WindowsToRender, Window);
        }
    }

    Windows_Render(WindowManager, WindowsToRender);    
    Glyph_Cache_Update(WindowManager->GlyphCache);
}

internal OS_DRAW_WINDOW_CALLBACK_DEFINE(Application_Draw_Window) {
    editor* Editor = (editor*)UserData;

    AK_Mutex_Lock(&Editor->UpdateLock);
    
    scratch Scratch = Scratch_Get();
    array<window_handle> WindowsToUpdate(&Scratch);
    window_manager* WindowManager = &Editor->WindowManager;
    for(window* Window = WindowManager->FirstWindow; Window; Window = Window->Next) {
        Array_Push(&WindowsToUpdate, window_handle(Window));
    }

    Editor_Update_And_Render(Editor, WindowsToUpdate, false);
    // Window_Handle_Resize(Editor, Window);
    // Editor_Render(Editor, {Window});
    AK_Mutex_Unlock(&Editor->UpdateLock);
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

    Editor.GlyphCache = Glyph_Cache_Create({
        .Renderer = Editor.Renderer
    });

    Editor.GlobalLayout = GDI_Context_Create_Bind_Group_Layout(Context, {
        .Bindings = { { 
                .Type = GDI_BIND_GROUP_TYPE_CONSTANT,
                .StageFlags = GDI_SHADER_STAGE_ALL
            }
        }
    });

    Editor.FontManager = Font_Manager_Create({});

    resource* FontResource = Packages_Get_Resource(Editor.Packages, String_Lit("fonts"), String_Lit("liberation mono"), String_Lit("regular"));
    Editor.MainFontBuffer = Packages_Load_Entire_Resource(Editor.Packages, FontResource, Editor.Arena);
    Editor.MainFont = Font_Manager_Create_Font(Editor.FontManager, Editor.MainFontBuffer, 40);

    {
        os_monitor_id MonitorID = OS_Get_Primary_Monitor();
        os_window_id  MainWindowID = OS_Open_Window({
            .Flags = OS_WINDOW_FLAG_MAIN_BIT,
            .Monitor = MonitorID,
            .Size = dim2i(1920, 1080)
        });
        if(!MainWindowID) {
            Fatal_Error_Message();
            return false;
        }

        gdi_format TargetFormat = GDI_FORMAT_R8G8B8A8_SRGB;

        scratch Scratch = Scratch_Get();
        array<gdi_format> WindowFormats = GDI_Context_Supported_Window_Formats(Context, OS_Window_Get_GDI_Data(MainWindowID), Scratch.Arena);
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
        gdi_handle<gdi_swapchain> Swapchain = GDI_Context_Create_Swapchain(Context, {
            .WindowData   = OS_Window_Get_GDI_Data(MainWindowID),
            .TargetFormat = WindowFormat,
            .UsageFlags   = UsageFlags
        });

        if(Swapchain.Is_Null()) {
            Fatal_Error_Message();
            return false;
        }

        Create_UI_Handles(&Editor, WindowFormat);
        Window_Manager_Create(&Editor.WindowManager, {
            .Allocator = Core_Get_Base_Allocator(),
            .Renderer = Editor.Renderer,
            .GlyphCache = Editor.GlyphCache,
            .UIRenderPass = Editor.UIRenderPass,
            .UIPipeline = Editor.UIPipeline,
            .UIGlobalLayout = Editor.GlobalLayout,
            .Format = WindowFormat,
            .UsageFlags = UsageFlags
        });

        window_handle Handle = Window_Open_With_Handle(&Editor.WindowManager, MainWindowID, Swapchain);
    }
        
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

    AK_Mutex_Create(&Editor.UpdateLock);
    OS_Set_Draw_Window_Callback(Application_Draw_Window, &Editor);


    while(OS_Window_Is_Open(OS_Get_Main_Window())) {
        //Update can only happen once and only one one thread
        AK_Mutex_Lock(&Editor.UpdateLock);

        scratch Scratch = Scratch_Get();
        array<window_handle> WindowsToUpdate(&Scratch);
        window_manager* WindowManager = &Editor.WindowManager;
        for(window* Window = WindowManager->FirstWindow; Window; Window = Window->Next) {
            Array_Push(&WindowsToUpdate, window_handle(Window));
        }

        bool Update = true;
        for(window_handle WindowHandle : WindowsToUpdate) {
            window* Window = Window_Get(WindowHandle);
            Assert(Window);
            if(Window_Is_Resizing(Window)) {
                Update = false;
                break;
            }
        }

        if(Update) {
            Editor_Update_And_Render(&Editor, WindowsToUpdate, true);
        }

        AK_Mutex_Unlock(&Editor.UpdateLock);
    }

    AK_Job_System_Delete(Editor.JobSystemHigh);
    AK_Job_System_Delete(Editor.JobSystemLow);

    GDI_Context_Delete(Context);
    Arena_Delete(Editor.Arena);
    GDI_Delete(GDI);
    return true;
}

#include "windows.cpp"
#include "ui/ui.cpp"
#include <engine_source.cpp>
