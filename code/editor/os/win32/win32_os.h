#ifndef WIN32_OS_H
#define WIN32_OS_H

#include <engine.h>
#include <os.h>
#include <os/win32/win32_shared.h>

//To prevent memory allocations, the window title will store some small cache first
enum {
    OS_WINDOW_TITLE_HEAP_ALLOCATED_BIT = (1 << 0)
};

struct os_window_title {
    char TitleData[64];
    uptr Size;
    u32  Flags;
};

struct os_window {
    os_window_id              ID;
    gdi_handle<gdi_swapchain> Swapchain;
    uint32_t                  Width;
    uint32_t                  Height;
    os_window_title           Title;
    os_window_flags           Flags;
    gdi_format                TargetFormat;
    gdi_format                Format;
    gdi_texture_usage_flags   UsageFlags;
    ak_event                  CreationEvent;
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
    os_event_manager    EventManager;
    os_event_stream*    EventStream;
};

enum win32_message {
    WIN32_CREATE_WINDOW_MSG,
    WIN32_DELETE_WINDOW_MSG
};

#endif