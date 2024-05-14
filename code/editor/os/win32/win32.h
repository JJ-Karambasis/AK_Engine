#ifndef WIN32_H
#define WIN32_H

#include <core/core.h>
#include <os.h>

struct os_process {
    PROCESS_INFORMATION Information;
};

struct os_monitor {
    HMONITOR        Monitor;
    wstring         DeviceName;
    os_monitor_info MonitorInfo;
};

struct os {
    arena*                 Arena;
    bool                   AppResult;
    HWND                   BaseWindow;
    array<os_monitor>      Monitors;
    array<os_monitor_id>   MonitorIDs;
    os_monitor_id          PrimaryMonitor;
    async_pool<os_process> ProcessPool;
};

enum win32_message {
    WIN32_MESSAGE_QUIT,
    WIN32_MESSAGE_COUNT
};

#endif