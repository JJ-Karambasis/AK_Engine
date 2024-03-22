#ifndef OS_H
#define OS_H

struct os;

void OS_Message_Box(string Message, string Title);

enum {
    OS_WINDOW_NONE = 0,
    OS_WINDOW_MAXIMIZED_BIT = (1 << 0)
};
typedef uint32_t os_window_flags;

struct os_window_create_info {
    u32             Width = 0;
    u32             Height = 0;
    string          Title;
    os_window_flags Flags = OS_WINDOW_NONE;
};

const os_event* OS_Next_Event();

os_window_id OS_Create_Window(const os_window_create_info& CreateInfo);
void         OS_Delete_Window(os_window_id WindowID);

struct os_create_info {
    u32 EventIterCount = 1;
    u32 MaxWindowCount = 32;
};

os*  OS_Create(const os_create_info& CreateInfo = {});
void OS_Delete();
os*  OS_Get();
void OS_Set(os* OS);

#endif