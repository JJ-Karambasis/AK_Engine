#ifndef OSX_H
#define OSX_H

#include <os/posix/posix.h>
#include <mach-o/dyld.h>

struct os_monitor {
    NSScreen*       Screen;
    os_monitor_info MonitorInfo;
};

struct os_window {

};

struct osx_os : os {
    fixed_array<os_monitor>    Monitors;
    fixed_array<os_monitor_id> MonitorIDs;
    async_pool<os_window>      WindowPool;
};

internal osx_os* OSX_Get();
internal osx_os* OSX_Get_Or_Set(osx_os* OS);

#endif