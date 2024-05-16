#ifndef OSX_H
#define OSX_H

#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#include <posix/posix.h>

struct os_window;

@interface osx_delegate : NSObject<NSApplicationDelegate>
@end

@interface osx_window_delegate : NSObject<NSWindowDelegate> {
    os_window* Window;
}

- (instancetype)initWithOSXWindow:(os_window *)Window;

@end

@interface osx_window_view : NSView {
    os_window* Window;
}

- (instancetype)initWithOSXWindow:(os_window *)Window;

@end

@interface osx_window : NSWindow
@end

struct os_monitor {
    NSScreen*       Screen;
    os_monitor_info MonitorInfo;
};

struct os_window {
    osx_window_delegate* Delegate;
    osx_window*          Window;
    osx_window_view*     View;
    void*                UserData;
};

struct osx_os : os {
    fixed_array<os_monitor>    Monitors;
    fixed_array<os_monitor_id> MonitorIDs;
    async_pool<os_window>      WindowPool;
    ak_atomic_u64              MainWindowID;
    bool                       AppResult;

    //Convert keyboard states and mouse states to an atomic u8 when its 
    //implemented
    ak_atomic_u32 KeyboardStates[OS_KEYBOARD_KEY_COUNT];                
    ak_atomic_u32 MouseStates[OS_MOUSE_KEY_COUNT];
    ak_atomic_u32 ScrollU32;
    ak_atomic_u64 MousePosPacked;
};

internal osx_os* OSX_Get();

#endif