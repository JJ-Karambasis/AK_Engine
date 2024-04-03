#ifndef OS_H
#define OS_H

struct os;

#include "os_event.h"

void   OS_Message_Box(string Message, string Title);
buffer OS_Read_Entire_File(allocator* Allocator, string Filepath);

enum {
    OS_WINDOW_NONE = 0,
    OS_WINDOW_MAXIMIZED_BIT = (1 << 0)
};
typedef uint32_t os_window_flags;

struct os_window_create_info {
    u32                     Width = 0;
    u32                     Height = 0;
    string                  Title;
    os_window_flags         Flags = OS_WINDOW_NONE;
    gdi_format              TargetFormat;
    gdi_texture_usage_flags UsageFlags;
};

const os_event* OS_Next_Event();

os_window_id              OS_Create_Window(const os_window_create_info& CreateInfo);
void                      OS_Delete_Window(os_window_id WindowID);
gdi_handle<gdi_swapchain> OS_Window_Get_Swapchain(os_window_id WindowID, gdi_format* OutFormat = NULL);
void                      OS_Window_Get_Resolution(os_window_id WindowID, u32* Width, u32* Height);

struct os_create_info {
    u32 EventIterCount = 1;
    u32 MaxWindowCount = 32;
};

os*  OS_Create(gdi_context* GDIContext, const os_create_info& CreateInfo = {});
void OS_Delete();
os*  OS_Get();
void OS_Set(os* OS);

#endif