#include <engine.h>
#include "editor_modules.h"
#include <os/os_event.h>
#include "os/os.h"

void Fatal_Error_Message() {
    OS_Message_Box("A fatal error occurred during initialization!\nPlease view the error logs for more info.", "Error");
}

int main() {
    if(!Core_Create()) {
        OS_Message_Box("A fatal error occurred during initialization!", "Error");
        return 1;
    }

    if(!OS_Create()) {
        Fatal_Error_Message();
        return 1;
    }

    os_window_id MainWindowID = OS_Create_Window({
        .Width = 1920,
        .Height = 1080,
        .Title = String_Lit("AK_Engine")
    });

    gdi* GDI = GDI_Create({});
    if(!GDI) {
        Fatal_Error_Message();
        return 1;
    }

    u32 DeviceCount = GDI_Get_Device_Count(GDI);
    if(!DeviceCount) {
        Fatal_Error_Message();
        return 1;
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
    ak_job_system* JobSystemLow = Core_Create_Job_System(1024, LowPriorityThreadCount, 0);

    Log_Info(modules::Editor, "Started creating GPU context for %.*s", Device.Name.Size, Device.Name.Str);
    gdi_context* GDIContext = GDI_Create_Context(GDI, { .JobSystem = JobSystemLow });
    if(!GDIContext) {
        Fatal_Error_Message();
        return 1;
    }

    Log_Info(modules::Editor, "Finished creating GPU context for %.*s", Device.Name.Size, Device.Name.Str);

    while(MainWindowID) {
        while(const os_event* Event = OS_Next_Event()) {
            switch(Event->Type) {
                case OS_EVENT_TYPE_WINDOW_CLOSED: {
                    OS_Delete_Window(Event->WindowID);
                    if(Event->WindowID == MainWindowID) {
                        MainWindowID = 0;
                    }
                } break;
            }
        }      
    }

    OS_Delete();
    Core_Delete();
    return 0;
}

#include <core.cpp>

#if defined(OS_WIN32)
#pragma comment(lib, "user32.lib")
#endif