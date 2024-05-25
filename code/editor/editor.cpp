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
    if(CurrentWindowSize.width != 0 && CurrentWindowSize.height != 0) {
        if(CurrentWindowSize != Window->Size) {
            Window_Resize(Editor, Window);
        }
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
    UI_Begin_Build(UI, window_handle(Window));

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
    ui_box* MenuBox = UI_Build_Box_From_StringF(UI, 0, "###%llux", (u64)(uptr)Window);
    UI_Push_Parent(UI, MenuBox);
    {
        f32 MenuWidth = UI_Box_Get_Dim(MenuBox).width;

        UI_Set_Next_Background_Color(UI, Color4_Yellow());
        UI_Set_Next_Text_Color(UI, Color4_Blue());
        UI_Set_Next_Pref_Width(UI, UI_Text(4, 1.0));
        UI_Set_Next_Text_Alignment(UI, UI_TEXT_ALIGNMENT_CENTER);
        UI_Build_Box_From_String(UI, UI_BOX_FLAG_DRAW_TEXT, String_Lit("Help###Menu Box 1"));
        
        f32 Width = UI_Box_Get_Dim(UI_Current_Box(UI)).width;

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

bool Editor_Render(editor* Editor, span<window*> WindowsToRender) {
    scratch Scratch = Scratch_Get();
    render_graph_id RenderGraph = Renderer_Create_Graph(Editor->Renderer);
    gdi_context* Context = Renderer_Get_Context(Editor->Renderer);

    array<gdi_handle<gdi_swapchain>> Swapchains(&Scratch, WindowsToRender.Count);
    for(uptr i = 0; i < WindowsToRender.Count; i++) {
        window* Window = WindowsToRender[i];
        if(Window->Size.width == 0 || Window->Size.height == 0) continue;

        Window_Update_UI(Editor, Window);

        s32 Texture = GDI_Context_Get_Swapchain_Texture_Index(Context, Window->Swapchain);
        if(Texture == -1) {
            Renderer_Delete_Graph(Editor->Renderer, RenderGraph);
            return false;
        }

        Assert(Texture >= 0);

        im_renderer* UIRenderer = Window->UI->Renderer;

        Render_Task_Attach_Render_Pass(Editor->Renderer, UIRenderer->RenderTask, {
            .RenderPass = Editor->UIRenderPass,
            .Framebuffer = Window->Framebuffers[(uptr)Texture],
            .ClearValues = {
                gdi_clear::Color(0.0f, 0.0f, 1.0f, 1.0f)
            }
        },
        {GDI_RESOURCE_STATE_COLOR});

        Array_Push(&Swapchains, Window->Swapchain);
        Render_Graph_Add_Task(RenderGraph, UIRenderer->RenderTask, 0);
    }

    bool Result = Renderer_Execute(Editor->Renderer, RenderGraph, Swapchains);
    Renderer_Delete_Graph(Editor->Renderer, RenderGraph);

    return Result;
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
            .UIPipeline = Editor.UIPipeline,
            .UIGlobalLayout = Editor.GlobalLayout,
            .Format = WindowFormat,
            .UsageFlags = UsageFlags
        });

        window_handle Handle = Window_Open_With_Handle(&Editor.WindowManager, MainWindowID, Swapchain);
    }
    
    editor_input_manager* InputManager = &Editor.InputManager;
    
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
                    window* Window = (window*)OS_Window_Get_Data(Event->Window);
                    Window_Close(&Editor.WindowManager, window_handle(Window));
                } break;

                case OS_EVENT_TYPE_MOUSE_DELTA: {
                    const os_mouse_delta_event* DeltaEvent = (const os_mouse_delta_event*)Event;
                    InputManager->MouseDelta = vec2(DeltaEvent->Delta);
                } break;

                case OS_EVENT_TYPE_MOUSE_SCROLL: {
                    const os_mouse_scroll_event* ScrollEvent = (const os_mouse_scroll_event*)Event;
                    InputManager->MouseScroll = ScrollEvent->Scroll;
                } break;
            }
        }

        window_manager* WindowManager = &Editor.WindowManager;
        for(window* Window = WindowManager->FirstWindow; Window; Window = Window->Next) {
            Window_Update(&Editor, Window);
        }
        Glyph_Cache_Update(Editor.GlyphCache);

        scratch Scratch = Scratch_Get();

        //Sometimes a window can fail to render because it resizes so we need
        //to keep a sepearate list of windows to render 
        bool RenderResult = true;
        array<window*> WindowsToRender(&Scratch);
        for(window* Window = WindowManager->FirstWindow; Window; Window = Window->Next) {
            Array_Push(&WindowsToRender, Window);
        }
        
        do {
            RenderResult = Editor_Render(&Editor, WindowsToRender); 
            if(!RenderResult) {
                array<window*> NewWindowsToRender(&Scratch);

                for(window* Window : WindowsToRender) {
                    gdi_swapchain_status PresentStatus = GDI_Context_Get_Swapchain_Status(Context, Window->Swapchain); 
                    if(PresentStatus == GDI_SWAPCHAIN_STATUS_RESIZE) {
                        Window_Resize(&Editor, Window);
                        Array_Push(&NewWindowsToRender, Window); 
                    } else if (PresentStatus != GDI_SWAPCHAIN_STATUS_OK) {
                        Assert(false);
                        //todo: Actual error logging
                        return false;
                    }
                }

                if(!NewWindowsToRender.Count) {
                    Assert(false);
                    //todo: Actual error logging
                    return false;
                }

                WindowsToRender = NewWindowsToRender;
            }
        } while(!RenderResult);
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
#include "editor_input.cpp"
#include <engine_source.cpp>
