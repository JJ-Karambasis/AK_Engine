#ifndef OSX_H
#define OSX_H


/*todo:

- Make win32 and osx layer consistent
  -Coordinate system. Make this top to bottom. OSX is Bottom to Top it looks like
  -Scrolls. Not sure if scrolls are possible. Values are always from 1 -> -1 on win32
   and they are higher resolution values in osx. Might need to add a scaling factor 
   on osx or win32 but this will never be 100% consistent, and I'm not sure how much
   this might be device dependent
  -Mouse coordinate via monitors. Do we have to be hovering over windows for mouse
   events to work? If so we can grab global mouse coordinates and there are no problems
  -Mouse deltas have a similar thing going on with scrolls

- Test how supporting multiple windows might work. Imagine a multi monitor setup 
  with a window on each side. One is a level editor and another might be a material
  editor or some other editor/tool. 

- Test multi monitor configuration. Kinda sucks since I can only have one additional
  monitor on osx along with the laptop monitor but it will have to do. 

*/

#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#include <mach-o/dyld.h>
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
    os_window_id         ID;
    osx_window_delegate* Delegate;
    osx_window*          Window;
    osx_window_view*     View;
    ak_atomic_u64        PosPacked;
    ak_atomic_u64        SizePacked;
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
    ak_atomic_u64 MouseDeltaPacked;
};

internal osx_os* OSX_Get();

#endif