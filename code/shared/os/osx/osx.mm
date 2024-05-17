#import <AppKit/AppKit.h>
#include "osx.h"

internal string OSX_Get_Executable_Path(allocator* Allocator) {
    local_persist char TempBuffer[1];
    u32 BufferSize = 1;
    _NSGetExecutablePath(TempBuffer, &BufferSize);

    char* Buffer = (char*)Allocator_Allocate_Memory(Allocator, BufferSize);
    _NSGetExecutablePath(Buffer, &BufferSize);

    return string(Buffer, BufferSize-1);
}

os* OS_Create(const os_create_info& CreateInfo) {
    @autoreleasepool {
        arena* Arena = Arena_Create(Core_Get_Base_Allocator());
        osx_os* OS = Arena_Push_Struct(Arena, osx_os);
        OS->Arena = Arena;
        OS_Set(OS);

        scratch Scratch = Scratch_Get();
        string ExecutableFilePath = OSX_Get_Executable_Path(&Scratch);
        OS->ExecutablePath = string(OS->Arena, String_Get_Path(ExecutableFilePath));

        NSArray<NSScreen*>* Screens = [NSScreen screens];
        Array_Init(&OS->Monitors, OS->Arena, [Screens count]);
        Array_Init(&OS->MonitorIDs, OS->Arena, OS->Monitors.Count);

        for(uptr i = 0; i < OS->Monitors.Count; i++) {
            const char* Name = [[Screens[i] localizedName] UTF8String];
            NSRect Rect = [Screens[i] frame];

            point2i Origin = point2i(Rect.origin.x, Rect.size.height-Rect.origin.y);
            dim2i Size = dim2i(Rect.size.width, Rect.size.height);

            OS->Monitors[i] = {
                .Screen = Screens[i],
                .MonitorInfo = {
                    .Name = string(OS->Arena, Name),
                    .Rect = rect2i(Origin, Origin + Size)
                }
            };

            OS->MonitorIDs[i] = (os_monitor_id)(uptr)(&OS->Monitors[i]);
        }

        Posix_Create(OS, CreateInfo);
        return OS;
    }
}

void OS_Delete();

span<os_monitor_id> OS_Get_Monitors() {
    osx_os* OS = OSX_Get();
    Assert(OS);
    return OS->MonitorIDs;
}

os_monitor_id OS_Get_Primary_Monitor() {
    osx_os* OS = OSX_Get();
    Assert(OS);
    //On OSX the main monitor is the one located at (0, 0). This should
    //always be the first monitor
    return OS->MonitorIDs[0];
}

const os_monitor_info* OS_Get_Monitor_Info(os_monitor_id ID) {
    osx_os* OS = OSX_Get();
    Assert(OS);
    os_monitor* Monitor = (os_monitor*)(uptr)(ID);
    return &Monitor->MonitorInfo;
}

os* OS_Get() {
    return (os*)OS_Get_Or_Set(nullptr);
}

void OS_Set(os* OS) {
    OS_Get_Or_Set(OS);
}

internal osx_os* OSX_Get() {
    return OSX_Get_Or_Set(nullptr);
}

internal osx_os* OSX_Get_Or_Set(osx_os* OS) {
    return (osx_os*)OS_Get_Or_Set(OS);
}
#include <os/posix/posix.cpp>