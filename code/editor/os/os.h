#ifndef OS_H
#define OS_H

struct os;
typedef uint64_t os_monitor_id;
typedef uint64_t os_window_id;

#include "os_input.h"

void   OS_Message_Box(string Message, string Title);
string OS_Get_Executable_Path(allocator* Allocator);
buffer OS_Read_Entire_File(allocator* Allocator, string Filepath);

os_monitor_id OS_Get_Primary_Monitor();
uvec2         OS_Get_Monitor_Resolution(os_monitor_id MonitorID);

struct os_window_create_info {
    os_monitor_id           MonitorID = 0;
    s32                     XOffset   = 0;
    s32                     YOffset   = 0;
    u32                     Width     = 0;
    u32                     Height    = 0;
    u32                     Border    = 0;
    gdi_format              TargetFormat;
    gdi_texture_usage_flags UsageFlags;
};

os_window_id              OS_Create_Window(const os_window_create_info& CreateInfo);
void                      OS_Delete_Window(os_window_id WindowID);
gdi_handle<gdi_swapchain> OS_Window_Get_Swapchain(os_window_id WindowID, gdi_format* OutFormat = NULL);
void                      OS_Window_Get_Resolution(os_window_id WindowID, u32* Width, u32* Height);

bool   OS_Keyboard_Get_Key_State(os_keyboard_key Key);
bool   OS_Mouse_Get_Key_State(os_mouse_key Key);
svec2  OS_Mouse_Get_Position(os_window_id WindowID);
svec2  OS_Mouse_Get_Delta();
f32    OS_Mouse_Get_Scroll();

struct os_create_info {
    u32 MaxWindowCount = 32;
};

os*  OS_Create(gdi_context* GDIContext, const os_create_info& CreateInfo = {});
void OS_Delete();
os*  OS_Get();
void OS_Set(os* OS);

#endif