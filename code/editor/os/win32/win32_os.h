#ifndef WIN32_OS_H
#define WIN32_OS_H

#include <engine.h>
#include <os.h>
#include <os/win32/win32_shared.h>

struct os_window {
    os_window_id              ID;
    gdi_handle<gdi_swapchain> Swapchain;
    s32                       XOffset;
    s32                       YOffset;
    u32                       Width;
    u32                       Height;
    s32                       BorderWidth;
    gdi_format                TargetFormat;
    gdi_format                Format;
    gdi_texture_usage_flags   UsageFlags;
    ak_event                  CreationEvent;
    HMONITOR                  Monitor;
    HWND                      Window;
};

struct os_window_storage {
    ak_async_slot_map64 SlotMap;
    os_window*          Windows;
};

struct os {
    arena*              Arena;
    gdi_context*        GDIContext;
    os_window_storage   WindowStorage;
    ak_auto_reset_event ThreadInitEvent;
    ak_atomic_u32       IsRunning;
    ak_thread           Thread;
    HWND                BaseWindow;
    WNDCLASSEXW         MainWindowClass;
    ak_atomic_u64       MouseDeltaPacked;
    ak_atomic_u32       ScrollU32;
};

enum win32_message {
    WIN32_CREATE_WINDOW_MSG,
    WIN32_DELETE_WINDOW_MSG
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
    VK_PAUSE,
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
    VK_F12
};

static_assert(Array_Count(G_VKCodes) == OS_KEYBOARD_KEY_COUNT);
static_assert(Array_Count(G_MouseVKCodes) == OS_MOUSE_KEY_COUNT);

#endif