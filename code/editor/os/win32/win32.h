#ifndef WIN32_H
#define WIN32_H

#include <core/core.h>
#include <math/math.h>
#include <os.h>
#include <os_event.h>

struct os_process {
    PROCESS_INFORMATION Information;
};

struct os_monitor {
    HMONITOR        Monitor;
    wstring         DeviceName;
    os_monitor_info MonitorInfo;
};

struct os_window {
    os_window_id  ID;
    HWND          Handle;
    ak_atomic_u64 PosPacked;
    ak_atomic_u64 SizePacked;
    void*         UserData;
};

struct os {
    arena*                 Arena;
    string                 ExecutablePath;
    async_pool<os_process> ProcessPool;
    os_event_manager       EventManager;
    os_event_stream*       EventStream;

    HWND                  BaseWindow;
    array<os_monitor>     Monitors;
    array<os_monitor_id>  MonitorIDs;
    os_monitor_id         PrimaryMonitor;
    async_pool<os_window> WindowPool;
    ak_atomic_u64         MainWindowID;
    bool                  AppResult;
};

enum win32_message {
    WIN32_MESSAGE_QUIT,
    WIN32_MESSAGE_SET_TITLE,
    WIN32_MESSAGE_OPEN_WINDOW,
    WIN32_MESSAGE_CLOSE_WINDOW,
    WIN32_MESSAGE_COUNT
};

struct win32_open_window_request {
    os_window*                 Window;
    const os_open_window_info* OpenInfo;
    bool                       Result;
};

struct win32_close_window_request {
    os_window* Window;
};

#define WIN32_MOUSE_USAGE 2
#define WIN32_USAGE_PAGE 1

global const DWORD G_MouseVKCodes[] = {
    VK_LBUTTON,
    VK_MBUTTON,
    VK_RBUTTON
};

global const DWORD G_VKCodes[] = {
    'A',
    'B',
    'C',
    'D',
    'E',
    'F',
    'G',
    'H',
    'I', 
    'J',
    'K',
    'L',
    'M',
    'N',
    'O',
    'P',
    'Q',
    'R',
    'S',
    'T',
    'U',
    'V',
    'W',
    'X',
    'Y',
    'Z',
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    VK_SPACE,
    VK_TAB,
    VK_ESCAPE,
    VK_UP,
    VK_DOWN,
    VK_LEFT,
    VK_RIGHT,
    VK_BACK,
    VK_RETURN,
    VK_DELETE,
    VK_INSERT,
    VK_SHIFT,
    VK_CONTROL,
    VK_MENU,
    VK_F1,
    VK_F2,
    VK_F3,
    VK_F4,
    VK_F5,
    VK_F6,
    VK_F7,
    VK_F8,
    VK_F9,
    VK_F10,
    VK_F11,
    VK_F12,
    (os_keyboard_key)-1 //Command not supported on win32. Always returns false, usually command and control are used in conjunction
};

static_assert(Array_Count(G_VKCodes) == OS_KEYBOARD_KEY_COUNT);
static_assert(Array_Count(G_MouseVKCodes) == OS_MOUSE_KEY_COUNT);

#endif