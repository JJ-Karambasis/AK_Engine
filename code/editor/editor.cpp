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

ui_font Create_UI_Font(editor* Editor, const_buffer FontBuffer) {
    ui_font Result;
    Result.Face   = Glyph_Manager_Create_Face(Editor->GlyphManager, FontBuffer);
    //Result.Shaper = Text_Shaper_Create_Face(Editor->TextShaper, Result.Face);
    return Result;
}

internal void Window_Handle_Resize(editor* Editor, window* Window) {
    uvec2 CurrentWindowSize;
    OS_Window_Get_Resolution(Window->WindowID, &CurrentWindowSize.w, &CurrentWindowSize.h);
    if(CurrentWindowSize.x != 0 && CurrentWindowSize.y != 0) {
        if(CurrentWindowSize != Window->Size) {
            if(Window->Framebuffers.Count) {
                for(gdi_handle<gdi_framebuffer> Framebuffer : Window->Framebuffers) {
                    GDI_Context_Delete_Framebuffer(Editor->GDIContext, Framebuffer);
                }
                Array_Clear(&Window->Framebuffers);
            }

            if(Window->SwapchainViews.Count) {
                for(gdi_handle<gdi_texture_view> SwapchainView : Window->SwapchainViews) {
                    GDI_Context_Delete_Texture_View(Editor->GDIContext, SwapchainView);
                }
                Array_Clear(&Window->SwapchainViews);
            }

            if(!GDI_Context_Resize_Swapchain(Editor->GDIContext, Window->Swapchain)) {
                Assert(false);
                return;
            }

            span<gdi_handle<gdi_texture>> Textures = GDI_Context_Get_Swapchain_Textures(Editor->GDIContext, Window->Swapchain);
            for(uptr i = 0; i < Textures.Count; i++) {
                Array_Push(&Window->SwapchainViews, GDI_Context_Create_Texture_View(Editor->GDIContext, {
                    .Texture = Textures[i]
                }));

                Array_Push(&Window->Framebuffers, GDI_Context_Create_Framebuffer(Editor->GDIContext, {
                    .Attachments = {Window->SwapchainViews[i]},
                    .RenderPass = Editor->UIRenderPass
                }));
            }
        }
    }
    Window->Size = CurrentWindowSize;
}

inline bool Window_Is_Open(window_handle Handle) {
    return Handle.Window && Handle.Generation && (Handle.Window->Generation == Handle.Generation);
}

window_handle Window_Open(editor* Editor, os_monitor_id MonitorID, svec2 Offset, uvec2 Size, u32 Border, string Name) {
    window* Window = Editor->FreeWindows;
    if(Editor->FreeWindows) SLL_Pop_Front(Editor->FreeWindows);
    else {
        Window = Arena_Push_Struct(Editor->Arena, window);
        Window->Generation = 1;
    }

    DLL_Push_Back(Editor->FirstWindow, Editor->LastWindow, Window);

    Window->Arena = Arena_Create(Core_Get_Base_Allocator());
    Array_Init(&Window->SwapchainViews, Window->Arena);
    Array_Init(&Window->Framebuffers, Window->Arena);

    Window->WindowID = OS_Create_Window({
        .MonitorID = MonitorID,
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

    window_handle Result = {
        .Window = Window,
        .Generation = Window->Generation
    };

    return Result;
}

void Window_Close(editor* Editor, window_handle Handle) {
    window* Window = Window_Get(Handle);
    if(Window) {
        DLL_Remove(Editor->FirstWindow, Editor->LastWindow, Window);

        if(Window->Framebuffers.Count) {
            for(gdi_handle<gdi_framebuffer> Framebuffer : Window->Framebuffers) {
                GDI_Context_Delete_Framebuffer(Editor->GDIContext, Framebuffer);
            }
        }

        if(Window->SwapchainViews.Count) {
            for(gdi_handle<gdi_texture_view> SwapchainView : Window->SwapchainViews) {
                GDI_Context_Delete_Texture_View(Editor->GDIContext, SwapchainView);
            }
        }
        
        OS_Delete_Window(Window->WindowID);
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

    ui* UI = &Window->UI;
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

void Window_Render(editor* Editor, window* Window) {
    if(Window->Size.x != 0 && Window->Size.y != 0) {
        gdi_context* GDIContext = Editor->GDIContext;
        
        if(Window->UIGlobalBuffer.Is_Null()) {
            Window->UIGlobalBuffer = GDI_Context_Create_Buffer(Editor->GDIContext, {
                .ByteSize = sizeof(ui_box_shader_global),
                .UsageFlags = GDI_BUFFER_USAGE_FLAG_CONSTANT_BUFFER_BIT
            });

            if(Window->UIGlobalBuffer.Is_Null()) return;
        }

        if(Window->UIGlobalBindGroup.Is_Null()) {
            Window->UIGlobalBindGroup = GDI_Context_Create_Bind_Group(Editor->GDIContext, {
                .Layout = Editor->GlobalBindGroupLayout,
                .WriteInfo = {
                    .Bindings = { { 
                                .Type = GDI_BIND_GROUP_TYPE_CONSTANT,
                                .BufferBinding = {
                                    .Buffer = Window->UIGlobalBuffer
                            }
                        }
                    }
                }
            });
            if(Window->UIGlobalBindGroup.Is_Null()) return;
        }

        //todo: When we do ui work we will know exactly how large the dynamic buffer should be
        Window->UIDynamicBufferSize = Align_Pow2(sizeof(ui_box_shader_dynamic), GDI_Context_Get_Info(Editor->GDIContext)->ConstantBufferAlignment);
        if(Window->UIDynamicBuffer.Is_Null()) {
            Window->UIDynamicBuffer = GDI_Context_Create_Buffer(Editor->GDIContext, {
                .ByteSize = Window->UIDynamicBufferSize*2,
                .UsageFlags = GDI_BUFFER_USAGE_FLAG_CONSTANT_BUFFER_BIT|GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT
            });
            if(Window->UIDynamicBuffer.Is_Null()) return;
        }

        if(Window->UIDynamicBindGroup.Is_Null()) {
            Window->UIDynamicBindGroup = GDI_Context_Create_Bind_Group(Editor->GDIContext, {
                .Layout = Editor->DynamicBindGroupLayout,
                .WriteInfo = {
                    .Bindings = { {
                            .Type = GDI_BIND_GROUP_TYPE_CONSTANT_DYNAMIC,
                            .BufferBinding = {
                                .Buffer = Window->UIDynamicBuffer,
                                .Size = Window->UIDynamicBufferSize
                            }
                        }
                    }
                }
            });
            if(Window->UIDynamicBindGroup.Is_Null()) return;
        }

        gdi_cmd_list* CmdList = GDI_Context_Begin_Cmd_List(GDIContext, gdi_cmd_list_type::Graphics, Window->Swapchain); 
        u32 TextureIndex = GDI_Cmd_List_Get_Swapchain_Texture_Index(CmdList, GDI_RESOURCE_STATE_COLOR);
        span<gdi_handle<gdi_texture>> SwapchainTextures = GDI_Context_Get_Swapchain_Textures(GDIContext, Window->Swapchain);

        GDI_Cmd_List_Barrier(CmdList, {
            {gdi_resource::Texture(SwapchainTextures[TextureIndex]), GDI_RESOURCE_STATE_NONE, GDI_RESOURCE_STATE_COLOR}, 
        });

        GDI_Cmd_List_Begin_Render_Pass(CmdList, {
            .RenderPass = Editor->UIRenderPass,
            .Framebuffer = Window->Framebuffers[TextureIndex],
            .ClearValues = { 
                gdi_clear::Color(0.0f, 0.0f, 1.0f, 0.0f)
            }
        });

        ui_box_shader_global ShaderGlobal = {
            .InvRes    = 1.0f / vec2(Window->Size),
            .InvTexRes = 1.0f / vec2(Editor->GlyphCache->Atlas.Dim)
        };

        u8* ShaderGlobalGPU = GDI_Context_Buffer_Map(Editor->GDIContext, Window->UIGlobalBuffer);
        Memory_Copy(ShaderGlobalGPU, &ShaderGlobal, sizeof(ui_box_shader_global));
        GDI_Context_Buffer_Unmap(Editor->GDIContext, Window->UIGlobalBuffer);

        gdi_handle<gdi_bind_group> BindGroups[UI_BIND_GROUP_COUNT];

        //todo: Render common window properties here    
        GDI_Cmd_List_Set_Pipeline(CmdList, Editor->UIBoxPipeline);
        GDI_Cmd_List_Set_Bind_Groups(CmdList, UI_BIND_GROUP_GLOBAL_INDEX, {Window->UIGlobalBindGroup});
        GDI_Cmd_List_Set_Bind_Groups(CmdList, UI_BIND_GROUP_TEXTURE_INDEX, {Editor->GlyphCache->Atlas.BindGroup});

        u8* ShaderDynamicGPU = GDI_Context_Buffer_Map(Editor->GDIContext, Window->UIDynamicBuffer);

        uptr Offset = 0;
        {
            GDI_Cmd_List_Set_Dynamic_Bind_Groups(CmdList, UI_BIND_GROUP_DYNAMIC_INDEX, {Window->UIDynamicBindGroup}, {Offset});
            
            ui_box_shader_dynamic Dynamic = {
                .DstP0 = vec2(10.0f, 10.0f),
                .DstP1 = vec2(50.0f, 50.0f),
                .Color = vec4(1.0f, 0.0f, 0.0f, 1.0f)
            };
            Memory_Copy(ShaderDynamicGPU, &Dynamic, sizeof(ui_box_shader_dynamic));
            ShaderDynamicGPU += Window->UIDynamicBufferSize;
            Offset += Window->UIDynamicBufferSize;

            GDI_Cmd_List_Draw_Instance(CmdList, 4, 1, 0, 0);
        }

        {
            Glyph_Face_Set_Size(Editor->MainFont.Face, 20);
            const glyph_entry* Glyph = Glyph_Cache_Get(Editor->GlyphCache, Editor->MainFont.Face, 'A');
            if(Glyph) {
                GDI_Cmd_List_Set_Dynamic_Bind_Groups(CmdList, UI_BIND_GROUP_DYNAMIC_INDEX, {Window->UIDynamicBindGroup}, {Offset});
                
                ui_box_shader_dynamic Dynamic = {
                    .DstP0 = vec2(100.0f, 100.0f),
                    .DstP1 = vec2(100.0f, 100.0f)+vec2(Glyph->P2-Glyph->P1),
                    .SrcP0 = vec2(Glyph->P1),
                    .SrcP1 = vec2(Glyph->P2),
                    .Color = vec4(1.0f, 1.0f, 1.0f, 1.0f)
                };

                Memory_Copy(ShaderDynamicGPU, &Dynamic, sizeof(ui_box_shader_dynamic));
                ShaderDynamicGPU += Window->UIDynamicBufferSize;
                Offset += Window->UIDynamicBufferSize;
                GDI_Cmd_List_Draw_Instance(CmdList, 4, 1, 0, 0);
            }
        }



        GDI_Cmd_List_End_Render_Pass(CmdList);
        GDI_Cmd_List_Barrier(CmdList, {
            {gdi_resource::Texture(SwapchainTextures[TextureIndex]), GDI_RESOURCE_STATE_COLOR, GDI_RESOURCE_STATE_PRESENT}
        });
    }
}

bool Editor_Main() {
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
    string ExecutablePath = OS_Get_Executable_Path(Editor.Arena);

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
    Editor.GDIContext = GDI_Create_Context(GDI, {});
    if(!Editor.GDIContext) {
        Fatal_Error_Message();
        return false;
    }

    Editor.GlyphManager = Glyph_Manager_Create({});
    if(!Editor.GlyphManager) {
        Fatal_Error_Message();
        return false;
    }

    // Editor.TextShaper = Text_Shaper_Create({});
    // if(!Editor.TextShaper) {
    //     Fatal_Error_Message();
    //     return false;
    // }

    if(!OS_Create(Editor.GDIContext)) {
        Fatal_Error_Message();
        return false;
    }
    Log_Info(modules::Editor, "Finished creating GPU context for %.*s", Device.Name.Size, Device.Name.Str);
    
    editor_input_manager* InputManager = &Editor.InputManager;

    os_monitor_id MonitorID = OS_Get_Primary_Monitor();
    
    uvec2 MonitorResolution = OS_Get_Monitor_Resolution(MonitorID);
    window_handle MainWindowID = Window_Open(&Editor, MonitorID, {}, MonitorResolution, 5, String_Lit("AK_Engine"));
    window* MainWindow = Window_Get(MainWindowID);

    // gdi_format SwapchainFormat;
    // OS_Window_Get_Swapchain(MainWindowID, &SwapchainFormat);
    Editor.UIRenderPass = GDI_Context_Create_Render_Pass(Editor.GDIContext, {
        .Attachments = {
            gdi_render_pass_attachment::Color(MainWindow->SwapchainFormat, GDI_LOAD_OP_CLEAR, GDI_STORE_OP_STORE)
        }
    });

    gdi_handle<gdi_sampler> LinearSampler = GDI_Context_Create_Sampler(Editor.GDIContext, {
        .Filter = GDI_FILTER_LINEAR
    });

    Editor.GlobalBindGroupLayout = GDI_Context_Create_Bind_Group_Layout(Editor.GDIContext, {
        .Bindings = { {
                .Type = GDI_BIND_GROUP_TYPE_CONSTANT,
                .StageFlags = GDI_SHADER_STAGE_VERTEX_BIT
            }
        }            
    });

    gdi_bind_group_layout_binding LinearSamplerBindGroupLayoutBindings[UI_TEXTURE_BIND_GROUP_BINDING_COUNT] = {};
    LinearSamplerBindGroupLayoutBindings[UI_TEXTURE_BIND_GROUP_TEXTURE_BINDING] = {
        .Type = GDI_BIND_GROUP_TYPE_SAMPLED_TEXTURE,
        .StageFlags = GDI_SHADER_STAGE_PIXEL_BIT
    };
    LinearSamplerBindGroupLayoutBindings[UI_TEXTURE_BIND_GROUP_SAMPLER_BINDING] = {
        .Type = GDI_BIND_GROUP_TYPE_SAMPLER,
        .StageFlags = GDI_SHADER_STAGE_PIXEL_BIT,
        .ImmutableSamplers = {LinearSampler}
    };

    Editor.LinearSamplerBindGroupLayout = GDI_Context_Create_Bind_Group_Layout(Editor.GDIContext, {
        .Bindings = LinearSamplerBindGroupLayoutBindings
    });

    Editor.DynamicBindGroupLayout = GDI_Context_Create_Bind_Group_Layout(Editor.GDIContext, {
        .Bindings = { { 
                .Type = GDI_BIND_GROUP_TYPE_CONSTANT_DYNAMIC,
                .StageFlags = GDI_SHADER_STAGE_VERTEX_BIT|GDI_SHADER_STAGE_PIXEL_BIT
            }
        }
    });

    Editor.GlyphCache = Glyph_Cache_Create({
        .Context = Editor.GDIContext,
        .AtlasBindGroupLayout = Editor.LinearSamplerBindGroupLayout
    });
    
    if(!Editor.GlyphCache) {
        Fatal_Error_Message();
        return false;
    }

    domain* ShaderDomain = Packages_Get_Domain(Editor.Packages, String_Lit("shaders"));

    {
        scratch Scratch = Scratch_Get();

        package* UIBoxShaderPackage = Packages_Get_Package(Editor.Packages, ShaderDomain, String_Lit("ui_box")); 
        resource* UIBoxVtxShaderResource = Packages_Get_Resource(Editor.Packages, UIBoxShaderPackage, String_Lit("vtx_shader"));
        resource* UIBoxPxlShaderResource = Packages_Get_Resource(Editor.Packages, UIBoxShaderPackage, String_Lit("pxl_shader"));

        const_buffer VtxShader = Packages_Load_Entire_Resource(Editor.Packages, UIBoxVtxShaderResource, &Scratch);
        const_buffer PxlShader = Packages_Load_Entire_Resource(Editor.Packages, UIBoxPxlShaderResource, &Scratch);
        
        gdi_handle<gdi_bind_group_layout> BindGroupLayouts[UI_BIND_GROUP_COUNT];
        BindGroupLayouts[UI_BIND_GROUP_GLOBAL_INDEX]  = Editor.GlobalBindGroupLayout;
        BindGroupLayouts[UI_BIND_GROUP_TEXTURE_INDEX] = Editor.LinearSamplerBindGroupLayout;
        BindGroupLayouts[UI_BIND_GROUP_DYNAMIC_INDEX] = Editor.DynamicBindGroupLayout;
        
        Editor.UIBoxPipeline = GDI_Context_Create_Graphics_Pipeline(Editor.GDIContext, {
            .VS      = {VtxShader, String_Lit("VS_Main")},
            .PS      = {PxlShader, String_Lit("PS_Main")},
            .Layouts = BindGroupLayouts,
            .GraphicsState = {
                .Topology = GDI_TOPOLOGY_TRIANGLE_STRIP,
                .BlendStates = { { 
                        .BlendEnabled = true,
                        .SrcColor     = GDI_BLEND_ONE,
                        .DstColor     = GDI_BLEND_ONE_MINUS_SRC_ALPHA
                    }
                },
            },
            .RenderPass = Editor.UIRenderPass
        });
    }

    //Default texture is pure white
    u32 DefaultTextureTexels[4] = {
        0xFFFFFFFF, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFFFFFFFF
    };

    Editor.DefaultTexture = GPU_Texture_Create(Editor.GDIContext, {
        .Format = GDI_FORMAT_R8G8B8A8_UNORM,
        .Dim = uvec2(2, 2),
        .Texels = const_buffer(DefaultTextureTexels),
        .BindGroupLayout = Editor.LinearSamplerBindGroupLayout
    });

    domain* FontDomain = Packages_Get_Domain(Editor.Packages, String_Lit("fonts"));
    {
        resource* MainFontResource = Packages_Get_Resource(Editor.Packages, FontDomain, String_Lit("liberation mono"), String_Lit("regular"));
        Editor.MainFontBuffer = Packages_Load_Entire_Resource(Editor.Packages, MainFontResource, Editor.Arena);
        Editor.MainFont = Create_UI_Font(&Editor, Editor.MainFontBuffer);
    }


    u64 Frequency = AK_Query_Performance_Frequency();
    u64 LastCounter = AK_Query_Performance_Counter();

    while(Window_Is_Open(MainWindowID)) {
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

        InputManager->MouseDelta = OS_Mouse_Get_Delta();
        InputManager->MouseScroll = OS_Mouse_Get_Scroll();

        if(InputManager->Is_Down(OS_KEYBOARD_KEY_ALT) && InputManager->Is_Pressed(OS_KEYBOARD_KEY_F4)) {
            Window_Close(&Editor, MainWindowID);
        }

        for(window* Window = Editor.FirstWindow; Window; Window = Window->Next) {
            Window_Update(&Editor, Window);
        }

        for(window* Window = Editor.FirstWindow; Window; Window = Window->Next) {
            Window_Render(&Editor, Window);
        }
        
        Glyph_Cache_Update(Editor.GlyphCache);

        while(GDI_Context_Execute(Editor.GDIContext) == GDI_EXECUTE_STATUS_RESIZE) {
            for(window* Window = Editor.FirstWindow; Window; Window = Window->Next) {
                Window_Handle_Resize(&Editor, Window);
            }
        }

    }

    AK_Job_System_Delete(Editor.JobSystemHigh);
    AK_Job_System_Delete(Editor.JobSystemLow);
    OS_Delete();

    GDI_Context_Delete(Editor.GDIContext);
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

int main() {
    if(!Core_Create()) {
        OS_Message_Box("A fatal error occurred during initialization!", "Error");
        return 1;
    }

    bool Result = Editor_Main();

    Core_Delete();
    return Result ? 0 : 1;
}

//#include "level_editor/level_editor.cpp"
#include "ui/ui.cpp"
#include "editor_input.cpp"
#include <engine_source.cpp>
#include <core/core.cpp>

#pragma comment(lib, "ftsystem.lib")
#pragma comment(lib, "hb.lib")

#if defined(OS_WIN32)
#pragma comment(lib, "user32.lib")
#endif