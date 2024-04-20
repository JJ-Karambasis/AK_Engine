#include <engine.h>
#include "editor_modules.h"
#include "os/os.h"
#include "editor_input.h"
#include "ui/ui.h"
#include "level_editor/level_editor.h"

#include <shader_common.h>
#include <shader.h>

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

    ak_job_system* JobSystemHigh = Core_Create_Job_System(1024, HighPriorityThreadCount, 1024);
    ak_job_system* JobSystemLow = Core_Create_Job_System(1024, LowPriorityThreadCount, 0);

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
    
    editor Editor = {};
    Editor.Arena = Arena_Create(Core_Get_Base_Allocator());
    Array_Init(&Editor.OSEventSubscribers, Editor.Arena);
    Window_Storage_Init(&Editor.WindowStorage, Editor.Arena, 128);
    editor_input_manager* InputManager = &Editor.InputManager;

    /*Create the main window before any other operation to know the 
      render target format. TargetFormat isn't guaranteed to be used on every
      platform, and renderpasses require this format to be known. The level editor
      will then take this handle instead of creating it itself*/
    os_window_id MainWindowID = OS_Create_Window({
        .Width = 1920,
        .Height = 1080,
        .Title = String_Lit("AK_Engine"),
        .TargetFormat = GDI_FORMAT_R8G8B8A8_UNORM,
        .UsageFlags = GDI_TEXTURE_USAGE_FLAG_COLOR_ATTACHMENT_BIT
    });

    gdi_format SwapchainFormat;
    OS_Window_Get_Swapchain(MainWindowID, &SwapchainFormat);
    gdi_handle<gdi_render_pass> UIRenderPass = GDI_Context_Create_Render_Pass(GDIContext, {
        .Attachments = {
            gdi_render_pass_attachment::Color(SwapchainFormat, GDI_LOAD_OP_CLEAR, GDI_STORE_OP_STORE)
        }
    });

    level_editor LevelEditor = {};
    if(!Level_Editor_Init(&LevelEditor, {
        .GDIContext   = GDIContext,
        .Editor       = &EventSubscribers,
        .WindowID     = MainWindowID,
        .UIRenderPass = UIRenderPass
    })) {
        Fatal_Error_Message();
        return false;
    }

    u64 Frequency = AK_Query_Performance_Frequency();
    u64 LastCounter = AK_Query_Performance_Counter();

    while(Level_Editor_Is_Open(&LevelEditor)) {
        u64 StartCounter = AK_Query_Performance_Counter();
        f64 dt = (double)(StartCounter-LastCounter)/(double)Frequency;
        LastCounter = StartCounter;
        Editor_Input_Manager_New_Frame(&InputManager, dt);

        while(const os_event* Event = OS_Next_Event()) {
            switch(Event->Type) {
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

                default: {
                    /*Events not handled by the engine can be handled by the subscribers. 
                    Subsystems can freely access these*/
                    for(const os_event_subscriber& Subscriber : EventSubscribers) {
                        Subscriber.EventCallback(Event, Subscriber.UserData);
                    }
                } break;
            }
        }

        for(window* Window = Editor->FirstWindow; Window; Window = Window->Next) {
            Window_Update_And_Render(Editor, Window);
        }

        Level_Editor_Update(&LevelEditor);
        Level_Editor_Render(&LevelEditor);

        GDI_Context_Execute(GDIContext);
    }

    Level_Editor_Shutdown(&LevelEditor);
    OS_Delete_Window(MainWindowID);
    Array_Free(&EventSubscribers);

    AK_Job_System_Delete(JobSystemHigh);
    AK_Job_System_Delete(JobSystemLow);
    OS_Delete();

    GDI_Context_Delete(GDIContext);
    GDI_Delete(GDI);
    return true;
}

void Window_Update_And_Render(editor* Editor, window* Window) {
    gdi_context* GDIContext = Window->GDIContext;
    gdi_cmd_list* CmdList = GDI_Context_Begin_Cmd_List(GDIContext, gdi_cmd_list_type::Graphics, Window->Swapchain); 
    Window->CmdList = GDI_Context_Begin_Cmd_List(GDIContext, gdi_cmd_list_type::Graphics);
    
    u32 TextureIndex = GDI_Cmd_List_Get_Swapchain_Texture_Index(CmdList, GDI_RESOURCE_STATE_COLOR);
    span<gdi_handle<gdi_texture>> SwapchainTextures = GDI_Context_Get_Swapchain_Textures(GDIContext, Window->Swapchain);

    GDI_Cmd_List_Barrier(CmdList, {
        {gdi_resource::Texture(SwapchainTextures[TextureIndex]), GDI_RESOURCE_STATE_NONE, GDI_RESOURCE_STATE_COLOR}, 
    });

    GDI_Cmd_List_Begin_Render_Pass(CmdList, {
        .RenderPass = Editor->UIRenderPass,
        .Framebuffer = Window->SwapchainFramebuffers[TextureIndex],
        .ClearValues = { 
            gdi_clear::Color(0.0f, 0.0f, 1.0f, 0.0f)
        }
    });

    //todo: Render common window properties here

    GDI_Cmd_List_Execute_Cmds(CmdList, {Window->CmdList});

    GDI_Cmd_List_End_Render_Pass(CmdList);
    GDI_Cmd_List_Barrier(CmdList, {
        {gdi_resource::Texture(SwapchainTextures[TextureIndex]), GDI_RESOURCE_STATE_COLOR, GDI_RESOURCE_STATE_PRESENT}
    });
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

#include "level_editor/level_editor.cpp"
#include "ui/ui.cpp"
#include "editor_input.cpp"
#include <engine_source.cpp>
#include <core/core.cpp>

#if defined(OS_WIN32)
#pragma comment(lib, "user32.lib")
#endif