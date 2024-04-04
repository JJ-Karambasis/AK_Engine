#include <engine.h>
#include "editor_modules.h"
#include "os/os.h"
#include "editor_input.h"

#include <shader_common.h>
#include <shader.h>

#define FBX_PARSER_USE_AK_FBX
#include "parsers/fbx/fbx_parser.h"

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
    u32 LowPriorityThreadCount  = Min(TotalThreadCount-HighPriorityThreadCount, 1);

    ak_job_system* JobSystemHigh = Core_Create_Job_System(1024, HighPriorityThreadCount, 1024);
    ak_job_queue*  JobQueueLow = Core_Create_Job_Queue(1024, LowPriorityThreadCount, 0);

    Log_Info(modules::Editor, "Started creating GPU context for %.*s", Device.Name.Size, Device.Name.Str);
    gdi_context* GDIContext = GDI_Create_Context(GDI, {});
    if(!GDIContext) {
        Fatal_Error_Message();
        return false;
    }

    if(!OS_Create(GDIContext)) {
        Fatal_Error_Message();
        return false;
    }
    Log_Info(modules::Editor, "Finished creating GPU context for %.*s", Device.Name.Size, Device.Name.Str);

    uvec2 LastWindowSize;
    os_window_id MainWindowID = OS_Create_Window({
        .Width = 1920,
        .Height = 1080,
        .Title = String_Lit("AK_Engine"),
        .TargetFormat = GDI_FORMAT_R8G8B8A8_UNORM,
        .UsageFlags = GDI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT_BIT
    });

    gdi_format SwapchainFormat;
    gdi_handle<gdi_swapchain> Swapchain = OS_Window_Get_Swapchain(MainWindowID, &SwapchainFormat);
    
    gdi_handle<gdi_render_pass> RenderPass = GDI_Context_Create_Render_Pass(GDIContext, {
        .Attachments = {
            gdi_render_pass_attachment::Color(SwapchainFormat, GDI_LOAD_OP_CLEAR, GDI_STORE_OP_STORE)
        }
    });

    gdi_handle<gdi_buffer> ViewBuffer = GDI_Context_Create_Buffer(GDIContext, {
        .ByteSize = sizeof(view_data),
        .UsageFlags = GDI_BUFFER_USAGE_FLAG_CONSTANT_BUFFER_BIT
    });

    uptr ConstantBufferAlignment = GDI_Context_Get_Info(GDIContext)->ConstantBufferAlignment;
    uptr DrawSize = Align_Pow2(sizeof(draw_data), ConstantBufferAlignment);

    gdi_handle<gdi_buffer> DrawBuffer = GDI_Context_Create_Buffer(GDIContext, {
        .ByteSize = DrawSize*2,
        .UsageFlags = GDI_BUFFER_USAGE_FLAG_CONSTANT_BUFFER_BIT|GDI_BUFFER_USAGE_FLAG_DYNAMIC_BUFFER_BIT
    });

    gdi_handle<gdi_bind_group_layout> ViewBindGroupLayout = GDI_Context_Create_Bind_Group_Layout(GDIContext, {
        .Bindings = { {
                .Type       = GDI_BIND_GROUP_TYPE_CONSTANT,
                .StageFlags = GDI_SHADER_STAGE_VERTEX_BIT
            }
        }
    });

    gdi_handle<gdi_bind_group_layout> DrawBindGroupLayout = GDI_Context_Create_Bind_Group_Layout(GDIContext, {
        .Bindings = { {
                .Type       = GDI_BIND_GROUP_TYPE_CONSTANT_DYNAMIC,
                .StageFlags = GDI_SHADER_STAGE_VERTEX_BIT|GDI_SHADER_STAGE_PIXEL_BIT
            }
        }
    });

    gdi_handle<gdi_bind_group> ViewBindGroup = GDI_Context_Create_Bind_Group(GDIContext, {
        .Layout = ViewBindGroupLayout,
        .WriteInfo = {
            .Bindings = { {
                    .Type  = GDI_BIND_GROUP_TYPE_CONSTANT,
                    .BufferBinding = {
                        .Buffer = ViewBuffer,
                    }
                }
            }
        }
    });

    gdi_handle<gdi_bind_group> DrawBindGroup = GDI_Context_Create_Bind_Group(GDIContext, {
        .Layout = DrawBindGroupLayout,
        .WriteInfo = {
            .Bindings = { {
                    .Type  = GDI_BIND_GROUP_TYPE_CONSTANT_DYNAMIC,
                    .BufferBinding = {
                        .Buffer = DrawBuffer,
                        .Size   = DrawSize
                    }
                }
            }
        }
    });
    
    scratch Scratch = Scratch_Get();
    buffer VtxShader = OS_Read_Entire_File(&Scratch, String_Lit("data/shaders/shader_vs.shader"));
    buffer PxlShader = OS_Read_Entire_File(&Scratch, String_Lit("data/shaders/shader_ps.shader"));
    
    gdi_handle<gdi_pipeline> Pipeline = GDI_Context_Create_Graphics_Pipeline(GDIContext, {
        .VS = {VtxShader, String_Lit("VS_Main")},
        .PS = {PxlShader, String_Lit("PS_Main")},
        .Layouts = {ViewBindGroupLayout, DrawBindGroupLayout},
        .GraphicsState = {
            .VtxBufferBindings = { {
                    .ByteStride = sizeof(vec3),
                    .Attributes = {
                        { .Semantic = String_Lit("POSITION"), .Format = GDI_FORMAT_R32G32B32_FLOAT }
                    }
                }
            }
        },
        .RenderPass = RenderPass
    });

    arena* Arena = Arena_Create(Core_Get_Base_Allocator());

    array<gdi_handle<gdi_texture_view>> SwapchainViews(Arena);
    array<gdi_handle<gdi_framebuffer>>  SwapchainFramebuffers(Arena);

    fbx_scene* Scene = FBX_Create_Scene(Core_Get_Base_Allocator(), String_Lit("data/Box.fbx"));
    Assert(Scene);

    array<vec3> TestVertices(&Scratch);
    array<u32>  TestIndices(&Scratch);

    u32 IndexOffset = 0;
    for(const fbx_object& Object : Scene->Objects) {
        for(uptr MeshIndex : Object.MeshIndices) {
            fbx_mesh* Mesh = &Scene->Meshes[MeshIndex];
            Array_Push_Range(&TestVertices, span(Mesh->Positions));
            
            fixed_array<u32> Indices(&Scratch, Mesh->TriangleCount*3);
            for(u32 i = 0; i < Mesh->TriangleCount; i++) {
                Indices[i*3 + 0] = IndexOffset + Mesh->PositionVtxIndices[i*3 + 0];
                Indices[i*3 + 1] = IndexOffset + Mesh->PositionVtxIndices[i*3 + 1];
                Indices[i*3 + 2] = IndexOffset + Mesh->PositionVtxIndices[i*3 + 2];
            }

            Array_Push_Range(&TestIndices, span(Indices));
            IndexOffset += Safe_U32(Mesh->Positions.Count);     
        }
    }

    gdi_handle<gdi_buffer> VtxBuffer = GDI_Context_Create_Buffer(GDIContext, {
        .ByteSize = Array_Byte_Size(&TestVertices),
        .UsageFlags = GDI_BUFFER_USAGE_FLAG_VTX_BUFFER_BIT,
        .InitialData = const_buffer(span(TestVertices))
    });

    gdi_handle<gdi_buffer> IdxBuffer = GDI_Context_Create_Buffer(GDIContext, {
        .ByteSize = Array_Byte_Size(&TestIndices),
        .UsageFlags = GDI_BUFFER_USAGE_FLAG_IDX_BUFFER_BIT,
        .InitialData = const_buffer(span(TestIndices))
    });

    editor_input_manager InputManager = {}; 
    camera Camera; 

    u64 Frequency = AK_Query_Performance_Frequency();
    u64 LastCounter = AK_Query_Performance_Counter();


    while(MainWindowID) {
        u64 StartCounter = AK_Query_Performance_Counter();
        f64 dt = (double)(StartCounter-LastCounter)/(double)Frequency;
        LastCounter = StartCounter;
        Editor_Input_Manager_New_Frame(&InputManager, dt);

        while(const os_event* Event = OS_Next_Event()) {
            switch(Event->Type) {
                case OS_EVENT_TYPE_WINDOW_CLOSED: {
                    OS_Delete_Window(Event->WindowID);
                    if(Event->WindowID == MainWindowID) {
                        if(SwapchainFramebuffers.Count) {
                            for(gdi_handle<gdi_framebuffer> Framebuffer : SwapchainFramebuffers) {
                                GDI_Context_Delete_Framebuffer(GDIContext, Framebuffer);
                            }
                            Array_Clear(&SwapchainFramebuffers);
                        }

                        if(SwapchainViews.Count) {
                            for(gdi_handle<gdi_texture_view> View : SwapchainViews) {
                                GDI_Context_Delete_Texture_View(GDIContext, View);
                            }
                            Array_Clear(&SwapchainViews);
                        }
                        MainWindowID = 0;
                    }
                } break;

                case OS_EVENT_TYPE_KEY_PRESSED: 
                case OS_EVENT_TYPE_KEY_RELEASED: {
                    const os_event_keyboard* KeyEvent = (const os_event_keyboard*)Event;
                    InputManager.KeyboardInput[KeyEvent->Key].IsDown = Event->Type == OS_EVENT_TYPE_KEY_PRESSED;
                } break;

                case OS_EVENT_TYPE_MOUSE_PRESSED:
                case OS_EVENT_TYPE_MOUSE_RELEASED: {
                    const os_event_mouse* MouseEvent = (const os_event_mouse*)Event;
                    InputManager.MouseInput[MouseEvent->Key].IsDown = Event->Type == OS_EVENT_TYPE_MOUSE_PRESSED;
                } break;

                case OS_EVENT_TYPE_MOUSE_MOVED: {
                    const os_event_mouse* MouseEvent = (const os_event_mouse*)Event;
                    InputManager.MouseDelta = MouseEvent->Delta;
                } break;

                case OS_EVENT_TYPE_MOUSE_SCROLLED: {
                    const os_event_mouse* MouseEvent = (const os_event_mouse*)Event;
                    InputManager.MouseScroll = MouseEvent->Scroll;
                } break;
            }
        }    

        if(MainWindowID) {
            if(InputManager.Is_Down(OS_MOUSE_KEY_MIDDLE)) {
                if(InputManager.Is_Down(OS_KEYBOARD_KEY_SHIFT)) {
                    quat Orientation = Camera_Get_Orientation(&Camera);
                    vec3 XAxis = Quat_Rotate(Orientation, vec3(1.0f, 0.0f, 0.0f));
                    vec3 YAxis = Quat_Rotate(Orientation, vec3(0.0f, 1.0f, 0.0f));
                    Camera.Target += (XAxis*InputManager.Get_DeltaX() + YAxis*InputManager.Get_DeltaY());
                } else if(InputManager.Is_Down(OS_KEYBOARD_KEY_CONTROL)) {
                    Camera.Distance -= InputManager.Get_DeltaY();
                    if(Camera.Distance < 1e-3f) {
                        Camera.Distance = 1e-3f;
                    }
                } else {
                    Camera.Roll -= InputManager.Get_DeltaX();
                    Camera.Pitch -= InputManager.Get_DeltaY();
                }
            }

            uvec2 CurrentWindowSize;
            OS_Window_Get_Resolution(MainWindowID, &CurrentWindowSize.w, &CurrentWindowSize.h);

            matrix4 Projection;
            matrix4_affine View;

            Camera_Get_Perspective(&Camera, &Projection, Safe_Ratio(CurrentWindowSize.w, CurrentWindowSize.h));
            Camera_Get_View(&Camera, &View);

            matrix4 ViewProjection; 
            Matrix4_Transpose(&ViewProjection, View*Projection);
            
            u8* ViewBufferGPU = GDI_Context_Buffer_Map(GDIContext, ViewBuffer);
            Memory_Copy(ViewBufferGPU, &ViewProjection, sizeof(matrix4));
            GDI_Context_Buffer_Unmap(GDIContext, ViewBuffer);

            if(CurrentWindowSize != LastWindowSize) {
                if(SwapchainFramebuffers.Count) {
                    for(gdi_handle<gdi_framebuffer> Framebuffer : SwapchainFramebuffers) {
                        GDI_Context_Delete_Framebuffer(GDIContext, Framebuffer);
                    }
                    Array_Clear(&SwapchainFramebuffers);
                }

                if(SwapchainViews.Count) {
                    for(gdi_handle<gdi_texture_view> SwapchainView : SwapchainViews) {
                        GDI_Context_Delete_Texture_View(GDIContext, SwapchainView);
                    }
                    Array_Clear(&SwapchainViews);
                }

                GDI_Context_Resize_Swapchain(GDIContext, Swapchain);

                span<gdi_handle<gdi_texture>> Textures = GDI_Context_Get_Swapchain_Textures(GDIContext, Swapchain);
                for(uptr i = 0; i < Textures.Count; i++) {
                    Array_Push(&SwapchainViews, GDI_Context_Create_Texture_View(GDIContext, {
                        .Texture = Textures[i]
                    }));

                    Array_Push(&SwapchainFramebuffers, GDI_Context_Create_Framebuffer(GDIContext, {
                        .Attachments = {SwapchainViews[i]},
                        .RenderPass = RenderPass
                    }));
                }

                LastWindowSize = CurrentWindowSize;
            }

            span<gdi_handle<gdi_texture>> SwapchainTextures = GDI_Context_Get_Swapchain_Textures(GDIContext, Swapchain);

            gdi_cmd_list* CmdList = GDI_Context_Begin_Cmd_List(GDIContext, gdi_cmd_list_type::Graphics, Swapchain);
            u32 TextureIndex = GDI_Cmd_List_Get_Swapchain_Texture_Index(CmdList, GDI_RESOURCE_STATE_COLOR);

            GDI_Cmd_List_Barrier(CmdList, {
                {gdi_resource::Texture(SwapchainTextures[TextureIndex]), GDI_RESOURCE_STATE_NONE, GDI_RESOURCE_STATE_COLOR}
            });

            GDI_Cmd_List_Begin_Render_Pass(CmdList, {
                .RenderPass = RenderPass,
                .Framebuffer = SwapchainFramebuffers[TextureIndex],
                .ClearValues = { 
                    gdi_clear::Color(0.0f, 0.0f, 1.0f, 0.0f)
                }
            });

            GDI_Cmd_List_Set_Vtx_Buffers(CmdList, {VtxBuffer});
            GDI_Cmd_List_Set_Idx_Buffer(CmdList, IdxBuffer, GDI_FORMAT_R32_UINT);
            GDI_Cmd_List_Set_Pipeline(CmdList, Pipeline);

            draw_data DrawData = {};

            matrix4_affine Model;
            Matrix4_Affine_Transform(&Model, vec3(-1.0f, 0.0f, 0.0f), vec3(0.5f, 0.5f, 0.5f));
            Matrix4_Affine_Transpose(&DrawData.Model, Model);
            DrawData.Color = vec4::Red();

            u8* Data = GDI_Context_Buffer_Map(GDIContext, DrawBuffer);
            Memory_Copy(Data, &DrawData, sizeof(DrawData));

            Matrix4_Affine_Transform(&Model, vec3(1.0f, 0.0f, 0.0f), vec3(0.5f, 0.5f, 0.5f));
            Matrix4_Affine_Transpose(&DrawData.Model, Model);
            DrawData.Color = vec4::Green();

            Memory_Copy(Data+DrawSize, &DrawData, sizeof(DrawData));

            GDI_Cmd_List_Set_Bind_Groups(CmdList, 0, {ViewBindGroup});

            GDI_Cmd_List_Set_Dynamic_Bind_Groups(CmdList, 1, {DrawBindGroup}, {0});
            GDI_Cmd_List_Draw_Indexed_Instance(CmdList, Safe_U32(TestIndices.Count), 0, 0, 1, 0);

            GDI_Cmd_List_Set_Dynamic_Bind_Groups(CmdList, 1, {DrawBindGroup}, {DrawSize});
            GDI_Cmd_List_Draw_Indexed_Instance(CmdList, Safe_U32(TestIndices.Count), 0, 0, 1, 0);

            GDI_Context_Buffer_Unmap(GDIContext, DrawBuffer);

            GDI_Cmd_List_End_Render_Pass(CmdList);

            GDI_Cmd_List_Barrier(CmdList, {
                {gdi_resource::Texture(SwapchainTextures[TextureIndex]), GDI_RESOURCE_STATE_COLOR, GDI_RESOURCE_STATE_PRESENT}
            });

            GDI_Context_Execute(GDIContext);
        }
    }

    AK_Job_System_Delete(JobSystemHigh);
    AK_Job_Queue_Delete(JobQueueLow);
    OS_Delete();

    GDI_Context_Delete(GDIContext);
    GDI_Delete(GDI);
    return true;
}

int main() {
    if(!Core_Create()) {
        OS_Message_Box("A fatal error occurred during initialization!", "Error");
        return 1;
    }

    bool Result = Editor_Main();

    Core_Delete();
    return Result ? 0 : 1;
}

#include "parsers/fbx/fbx_parser.cpp"

#include "editor_input.cpp"
#include <engine_source.cpp>
#include <core/core.cpp>

#if defined(OS_WIN32)
#pragma comment(lib, "user32.lib")
#endif