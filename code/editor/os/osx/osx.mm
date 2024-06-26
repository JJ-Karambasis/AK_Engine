#include "osx.h"

internal void OSX_Create_Menu_Bar();
internal os_keyboard_key OSX_Translate_Key(int VKCode);
internal NSUInteger OSX_Translate_Key_To_Modifier_Flag(os_keyboard_key Key);

internal string OSX_Get_Executable_Path(allocator* Allocator) {
    local_persist char TempBuffer[1];
    u32 BufferSize = 1;
    _NSGetExecutablePath(TempBuffer, &BufferSize);

    char* Buffer = (char*)Allocator_Allocate_Memory(Allocator, BufferSize);
    _NSGetExecutablePath(Buffer, &BufferSize);

    return string(Buffer, BufferSize-1);
}

string OS_Get_Executable_Path() {
    osx_os* OS = OSX_Get();
    return OS->ExecutablePath;
}

bool   OS_Directory_Exists(string Directory);
bool   OS_File_Exists(string File);
void   OS_Message_Box(string Message, string Title);

span<os_monitor_id> OS_Get_Monitors() {
    osx_os* OS = OSX_Get();
    return OS->MonitorIDs;
}

os_monitor_id OS_Get_Primary_Monitor() {
    osx_os* OS = OSX_Get();
    return OS->MonitorIDs[0];
}

const os_monitor_info* OS_Get_Monitor_Info(os_monitor_id ID) {
    osx_os* OS = OSX_Get();
    os_monitor* Monitor = (os_monitor*)(uptr)(ID);
    return &Monitor->MonitorInfo;
}


internal os_window* OS_Window_Get(os_window_id WindowID) {
    osx_os* OS = OSX_Get();
    os_window* Window = Async_Pool_Get(&OS->WindowPool, async_handle<os_window>(WindowID));
    return Window;
}

internal bool OS_Open_Window_Internal(os_window* Window, const os_open_window_info& OpenInfo) {
    __block bool WindowCreated = false;
    dispatch_sync(dispatch_get_main_queue(), ^{
        // Update the UI on the main thread
        Window->Delegate = [[osx_window_delegate alloc] initWithOSXWindow:Window];
        if(Window->Delegate == nil) {
            WindowCreated = false;
            return;
        }

        os_monitor* Monitor = (os_monitor*)(uptr)OpenInfo.Monitor;
        Assert(Monitor);
        
        NSRect FrameRect;

        //If we are maximized we ignore the position and size parameters

        point2i Origin;
        dim2i Dim;
        if(OpenInfo.Flags & OS_WINDOW_FLAG_MAXIMIZE_BIT) {
            rect2i Rect = Monitor->MonitorInfo.Rect;
            Origin = point2i(Rect.P1.x, Rect.P1.y);
            Dim = Rect2i_Get_Dim(Rect);
            FrameRect = NSMakeRect(Rect.P1.x, Rect.P1.y, Dim.width, Dim.height);
        } else {
            Origin = OpenInfo.Pos;
            Origin += Monitor->MonitorInfo.Rect.P1;
            Dim = OpenInfo.Size;

            point2i ActualOrigin = Origin;

            //Since our coordinate system is top down and osx is bottom up. The origin
            //is in the wrong corner.
            ActualOrigin.y = (Rect2i_Get_Height(Monitor->MonitorInfo.Rect) - ActualOrigin.y) - OpenInfo.Size.height;

            //Make sure coordinate system is top down and not bottom up
            FrameRect = NSMakeRect(ActualOrigin.x, ActualOrigin.y, OpenInfo.Size.width, OpenInfo.Size.height);
        }
        AK_Atomic_Store_U64_Relaxed(&Window->PosPacked, (u64)Pack_S64(Origin.x, Origin.y));
        AK_Atomic_Store_U64_Relaxed(&Window->SizePacked, (u64)Pack_S64(Dim.width, Dim.height));

        NSUInteger StyleMask = (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable | NSWindowStyleMaskBorderless);

        Window->Window = [[osx_window alloc] 
            initWithContentRect:FrameRect
            styleMask:StyleMask
            backing:NSBackingStoreBuffered
            defer:NO];

        if(Window->Window == nil) {
            WindowCreated = false;
            return;
        }

        [Window->Window setCollectionBehavior: NSWindowCollectionBehaviorFullScreenPrimary];

        if(OpenInfo.Flags & OS_WINDOW_FLAG_MAXIMIZE_BIT) {
            [Window->Window zoom:nil];
        }

        Window->View = [[osx_window_view alloc] initWithOSXWindow:Window];
        if(Window->View == nil) {
            WindowCreated = false;
            return;
        }

        [Window->Window setContentView:Window->View];
        [Window->Window makeFirstResponder:Window->View];
        [Window->Window setDelegate:Window->Delegate];
        [Window->Window setAcceptsMouseMovedEvents:TRUE];
        [NSApp activateIgnoringOtherApps:YES];
        [Window->Window makeKeyAndOrderFront:nil];

        WindowCreated = true;
    });

    return WindowCreated;
}

os_window_id OS_Open_Window(const os_open_window_info& OpenInfo) {
    osx_os* OSX = OSX_Get();
    async_handle<os_window> Handle = Async_Pool_Allocate(&OSX->WindowPool);
    os_window* Window = Async_Pool_Get(&OSX->WindowPool, Handle);

    if(!OS_Open_Window_Internal(Window, OpenInfo)) {
        OS_Close_Window(Handle.ID);
        return 0;
    }

    Window->ID = Handle.ID; 

    if(OpenInfo.Flags & OS_WINDOW_FLAG_MAIN_BIT) {
        OS_Set_Main_Window(Window->ID);
    }

    OS_Window_Set_Data(Window->ID, OpenInfo.UserData);
    OS_Window_Set_Title(Window->ID, OpenInfo.Title);

    return Handle.ID;
}

void OS_Close_Window(os_window_id WindowID) {
    os_window* Window = OS_Window_Get(WindowID);
    if(Window) {
        osx_os* OS = OSX_Get();

        dispatch_sync(dispatch_get_main_queue(), ^{
            Window->UserData = nullptr;
            if(Window->Window != nil) {
                [Window->Window orderOut:nil];
                [Window->Window setDelegate:nil];
                [Window->Window close];
                Window->Window = nil;
            }

            if(Window->View != nil) {
                [Window->View release];
                Window->View = nil;
            }

            if(Window->Delegate != nil) {
                [Window->Delegate release];
                Window->Delegate = nil;
            }
        });

        Async_Pool_Free(&OS->WindowPool, async_handle<os_window>(WindowID));
    }
}

bool OS_Window_Is_Open(os_window_id WindowID) {
    osx_os* OS = OSX_Get();
    return Async_Pool_Valid_Handle(&OS->WindowPool, async_handle<os_window>(WindowID));
}

void OS_Set_Main_Window(os_window_id WindowID) {
    osx_os* OS = OSX_Get();
    AK_Atomic_Store_U64_Relaxed(&OS->MainWindowID, WindowID);
}

os_window_id OS_Get_Main_Window() {
    osx_os* OS = OSX_Get();
    return (os_window_id)AK_Atomic_Load_U64_Relaxed(&OS->MainWindowID);
}

void OS_Window_Set_Data(os_window_id WindowID, void* UserData) {
    os_window* Window = OS_Window_Get(WindowID);
    if(!Window) return;
    Window->UserData = UserData;
}

void* OS_Window_Get_Data(os_window_id WindowID) {
    os_window* Window = OS_Window_Get(WindowID);
    if(!Window) return nullptr;
    return Window->UserData;
}

void OS_Window_Set_Title(os_window_id WindowID, string Title) {
    dispatch_async(dispatch_get_main_queue(), ^{
        os_window* Window = OS_Window_Get(WindowID);
        if(!Window) return;

        @autoreleasepool {
            NSString* NSTitle = [[NSString alloc] 
                initWithBytes:Title.Str
                length:Title.Size
                encoding:NSUTF8StringEncoding];
            [Window->Window setTitle:NSTitle];
        }
    });
}

dim2i OS_Window_Get_Size(os_window_id WindowID) {
    os_window* Window = OS_Window_Get(WindowID);
    if(!Window) return {};
    
    s64 SizePacked = (s64)AK_Atomic_Load_U64_Relaxed(&Window->SizePacked);
    return Unpack_S64(SizePacked);
}

point2i OS_Window_Get_Pos(os_window_id WindowID) {
    os_window* Window = OS_Window_Get(WindowID);
    if(!Window) return {};

    s64 PosPacked = (s64)AK_Atomic_Load_U64_Relaxed(&Window->PosPacked);
    return Unpack_S64(PosPacked);
}

bool OS_Keyboard_Get_Key_State(os_keyboard_key Key) {
    osx_os* OS = OSX_Get();
    return AK_Atomic_Load_U32_Relaxed(&OS->KeyboardStates[Key]) == true;
}

bool OS_Mouse_Get_Key_State(os_mouse_key Key) {
    osx_os* OS = OSX_Get();
    return AK_Atomic_Load_U32_Relaxed(&OS->MouseStates[Key]) == true;
}

point2i OS_Mouse_Get_Position() {
    osx_os* OS = OSX_Get();
    s64 MousePosPacked = (s64)AK_Atomic_Load_U64_Relaxed(&OS->MousePosPacked);
    return Unpack_S64(MousePosPacked);
}

internal THREAD_CONTEXT_CALLBACK(OSX_Applcation_Thread) {
    osx_os* OS = OSX_Get();
    OS->AppResult = Application_Main();
    dispatch_async(dispatch_get_main_queue(), ^{
        [NSApp terminate:nil];
    });
    return 0;
}

@implementation osx_delegate

- (void)applicationWillTerminate:(NSNotification*)Notification {
    Core_Delete();
}

@end

@implementation osx_window_delegate

- (instancetype)initWithOSXWindow:(os_window *)_Window {
    self = [super init];
    if(self != nil)
        Window = _Window;
    return self;
}

- (BOOL)windowShouldClose:(id)sender {
    osx_os* OS = OSX_Get();
    os_event* Event = OS_Event_Stream_Allocate_Event(OS->EventStream, OS_EVENT_TYPE_WINDOW_CLOSED);
    Event->Window = Window->ID;
    return NO;
}

- (void)windowDidResize:(NSNotification *)Notification {
    NSRect WindowRect = [Window->Window frame];
    s64 PackedDim = ((s64)WindowRect.size.width) | (((s64)WindowRect.size.height) << 32);
    AK_Atomic_Store_U64_Relaxed(&Window->SizePacked, (u64)Pack_S64(WindowRect.size.width, WindowRect.size.height));
}

- (void)windowDidMove:(NSNotification *)Notification {
    NSScreen* Screen = [Window->Window screen];
    NSRect WindowRect = [Window->Window frame];
    NSRect ScreenRect = [Screen frame];

    point2i Origin = point2i(WindowRect.origin.x, WindowRect.origin.y);
    Origin.y = (ScreenRect.size.height - Origin.y) - WindowRect.size.height;
    AK_Atomic_Store_U64_Relaxed(&Window->PosPacked, (u64)Pack_S64(Origin.x, Origin.y));
}

@end

@implementation osx_window_view

- (instancetype)initWithOSXWindow:(os_window *)_Window {
    self = [super init];
    if(self != nil)
        Window = _Window;
    return self;
}

- (BOOL)canBecomeKeyView {
    return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event {
    return YES;
}

- (void)mouseDown:(NSEvent*)Event {
    osx_os* OS = OSX_Get();
    AK_Atomic_Store_U32_Relaxed(&OS->MouseStates[OS_MOUSE_KEY_LEFT], true);
}

- (void)mouseUp:(NSEvent*)Event {
    osx_os* OS = OSX_Get();
    AK_Atomic_Store_U32_Relaxed(&OS->MouseStates[OS_MOUSE_KEY_LEFT], false);
}

- (void)rightMouseDown:(NSEvent*)Event {
    osx_os* OS = OSX_Get();
    AK_Atomic_Store_U32_Relaxed(&OS->MouseStates[OS_MOUSE_KEY_RIGHT], true);
}

- (void)rightMouseUp:(NSEvent*)Event {
    osx_os* OS = OSX_Get();
    AK_Atomic_Store_U32_Relaxed(&OS->MouseStates[OS_MOUSE_KEY_RIGHT], false);
}

- (void)otherMouseDown:(NSEvent*)Event {
    osx_os* OS = OSX_Get();
    int ButtonNumber = [Event buttonNumber];
    if(ButtonNumber == 2)
        AK_Atomic_Store_U32_Relaxed(&OS->MouseStates[OS_MOUSE_KEY_MIDDLE], true);
}

- (void)otherMouseUp:(NSEvent*)Event {
    osx_os* OS = OSX_Get();
    int ButtonNumber = [Event buttonNumber];
    if(ButtonNumber == 2)
        AK_Atomic_Store_U32_Relaxed(&OS->MouseStates[OS_MOUSE_KEY_MIDDLE], false);
}


- (void)keyDown:(NSEvent *)Event {
    osx_os* OS = OSX_Get();
    os_keyboard_key Key = OSX_Translate_Key([Event keyCode]);
    if(Key != -1) {
        AK_Atomic_Store_U32_Relaxed(&OS->KeyboardStates[Key], true);
    }
    [self interpretKeyEvents:@[Event]];
}

- (void)keyUp:(NSEvent *)Event {
    osx_os* OS = OSX_Get();
    os_keyboard_key Key = OSX_Translate_Key([Event keyCode]);
    if(Key != -1) {
        AK_Atomic_Store_U32_Relaxed(&OS->KeyboardStates[Key], false);
    }
}

- (void)flagsChanged:(NSEvent *)Event {
    osx_os* OS = OSX_Get();

    const unsigned int ModifierFlags =
        [Event modifierFlags] & NSEventModifierFlagDeviceIndependentFlagsMask;
    os_keyboard_key Key = OSX_Translate_Key([Event keyCode]);
    if(Key != -1) {
        const NSUInteger KeyFlag = OSX_Translate_Key_To_Modifier_Flag(Key);
        if (KeyFlag & ModifierFlags)
        {
            if(AK_Atomic_Load_U32_Relaxed(&OS->KeyboardStates[Key])) {
                AK_Atomic_Store_U32_Relaxed(&OS->KeyboardStates[Key], false);
            } else {
                AK_Atomic_Store_U32_Relaxed(&OS->KeyboardStates[Key], true);
            }
        }
        else
            AK_Atomic_Store_U32_Relaxed(&OS->KeyboardStates[Key], false);
    }
}


- (void)scrollWheel:(NSEvent *)Event {
    osx_os* OS = OSX_Get();
    f32 Scroll = [Event scrollingDeltaY];

    if ([Event hasPreciseScrollingDeltas]) {
        Scroll *= 0.1;
    }

    os_mouse_scroll_event* ScrollEvent = (os_mouse_scroll_event*)OS_Event_Stream_Allocate_Event(OS->EventStream, OS_EVENT_TYPE_MOUSE_SCROLL);
    ScrollEvent->Scroll = Scroll;
}

- (void)mouseMoved:(NSEvent *)Event {
    osx_os* OS = OSX_Get();
    NSPoint Pos = [NSEvent mouseLocation];

    //Make sure coordinate system is top down and not bottom up
    NSScreen* Screen = [[Event window] screen];
    NSRect Rect = [Screen frame];

    Pos.y = Rect.size.height - Pos.y;

    vec2i Delta = vec2i([Event deltaX], [Event deltaY]);
    os_mouse_delta_event* DeltaEvent = (os_mouse_delta_event*)OS_Event_Stream_Allocate_Event(OS->EventStream, OS_EVENT_TYPE_MOUSE_DELTA);
    DeltaEvent->Delta = Delta;

    AK_Atomic_Store_U64_Relaxed(&OS->MousePosPacked, (u64)Pack_S64(Pos.x, Pos.y));
}

@end

@implementation osx_window

- (BOOL)canBecomeKeyWindow {
    // Required for NSWindowStyleMaskBorderless windows
    return YES;
}

- (BOOL)canBecomeMainWindow {
    return YES;
}

@end

int main() {
    @autoreleasepool {
        [NSApplication sharedApplication];
        Assert(NSApp != nil);

        if(!Core_Create()) {
            //todo: Logging
            return 1;
        }

        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        OSX_Create_Menu_Bar();

        osx_delegate* AppDelegate = [[osx_delegate alloc] init];
        [NSApp setDelegate:AppDelegate]; 

        NSEvent* (^Block)(NSEvent*) = ^ NSEvent* (NSEvent* Event)
        {
            if ([Event modifierFlags] & NSEventModifierFlagCommand)
                [[NSApp keyWindow] sendEvent:Event];

            return Event;
        };

        [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyUp
                 handler:Block];

        scratch Scratch = Scratch_Get();

        osx_os OS = {};
        OS.Arena = Arena_Create(Core_Get_Base_Allocator());
        OS_Set(&OS);

        string ExecutableFilePath = OSX_Get_Executable_Path(&Scratch);
        OS.ExecutablePath = string(OS.Arena, String_Get_Path(ExecutableFilePath));

        if(!Posix_Create()) {
            //todo: Logging
            return 1;
        }
        

        NSArray<NSScreen*>* Screens = [NSScreen screens];
        Array_Init(&OS.Monitors, OS.Arena, [Screens count]);
        Array_Init(&OS.MonitorIDs, OS.Arena, OS.Monitors.Count);

        for(uptr i = 0; i < OS.Monitors.Count; i++) {
            const char* Name = [[Screens[i] localizedName] UTF8String];
            NSRect Rect = [Screens[i] frame];

            point2i Origin = point2i(Rect.origin.x, Rect.origin.y);
            dim2i Dim = dim2i(Rect.size.width, Rect.size.height);

            OS.Monitors[i] = {
                .Screen = Screens[i],
                .MonitorInfo = {
                    .Name = string(OS.Arena, Name),
                    .Rect = rect2i(Origin, Origin+Dim)
                }
            };

            OS.MonitorIDs[i] = (os_monitor_id)(uptr)(&OS.Monitors[i]);
        }

        Async_Pool_Create(&OS.WindowPool, OS.Arena, OS_MAX_WINDOW_COUNT);

        if(!Thread_Manager_Create_Thread(OSX_Applcation_Thread, nullptr)) {
            return 1;
        }

        for(;;) {
            NSEvent* Event = [NSApp nextEventMatchingMask: NSEventMaskAny
                                    untilDate: nil
                                    inMode: NSDefaultRunLoopMode
                                    dequeue: YES];
            if(Event == nil) {                
                if(OS.EventStream) {
                    OS_Event_Manager_Push_Back_Stream(&OS.EventManager, OS.EventStream);
                }
                OS.EventStream = OS_Event_Manager_Allocate_Stream(&OS.EventManager);
                Event = [NSApp nextEventMatchingMask: NSEventMaskAny
                               untilDate: [NSDate distantFuture]
                               inMode: NSDefaultRunLoopMode
                               dequeue: YES];
                Assert(Event != nil);
            }

            [NSApp sendEvent: Event];
        }
    }
}

internal osx_os* OSX_Get() {
    return (osx_os*)OS_Get();
}

#include <posix/posix.cpp>

// Needed for _NSGetProgname
#include <crt_externs.h>

//todo: This is gross, copied from GLFW. We should probably use a nib when shipping
internal void OSX_Create_Menu_Bar()
{
    NSString* appName = nil;
    NSDictionary* bundleInfo = [[NSBundle mainBundle] infoDictionary];
    NSString* nameKeys[] =
    {
        @"CFBundleDisplayName",
        @"CFBundleName",
        @"CFBundleExecutable",
    };

    // Try to figure out what the calling application is called

    for (size_t i = 0;  i < sizeof(nameKeys) / sizeof(nameKeys[0]);  i++)
    {
        id name = bundleInfo[nameKeys[i]];
        if (name &&
            [name isKindOfClass:[NSString class]] &&
            ![name isEqualToString:@""])
        {
            appName = name;
            break;
        }
    }

    if (!appName)
    {
        char** progname = _NSGetProgname();
        if (progname && *progname)
            appName = @(*progname);
        else
            appName = @"GLFW Application";
    }

    NSMenu* bar = [[NSMenu alloc] init];
    [NSApp setMainMenu:bar];

    NSMenuItem* appMenuItem =
        [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    NSMenu* appMenu = [[NSMenu alloc] init];
    [appMenuItem setSubmenu:appMenu];

    [appMenu addItemWithTitle:[NSString stringWithFormat:@"About %@", appName]
                       action:@selector(orderFrontStandardAboutPanel:)
                keyEquivalent:@""];
    [appMenu addItem:[NSMenuItem separatorItem]];
    NSMenu* servicesMenu = [[NSMenu alloc] init];
    [NSApp setServicesMenu:servicesMenu];
    [[appMenu addItemWithTitle:@"Services"
                       action:NULL
                keyEquivalent:@""] setSubmenu:servicesMenu];
    [servicesMenu release];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:[NSString stringWithFormat:@"Hide %@", appName]
                       action:@selector(hide:)
                keyEquivalent:@"h"];
    [[appMenu addItemWithTitle:@"Hide Others"
                       action:@selector(hideOtherApplications:)
                keyEquivalent:@"h"]
        setKeyEquivalentModifierMask:NSEventModifierFlagOption | NSEventModifierFlagCommand];
    [appMenu addItemWithTitle:@"Show All"
                       action:@selector(unhideAllApplications:)
                keyEquivalent:@""];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:[NSString stringWithFormat:@"Quit %@", appName]
                       action:@selector(terminate:)
                keyEquivalent:@"q"];

    NSMenuItem* windowMenuItem =
        [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    [bar release];
    NSMenu* windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    [NSApp setWindowsMenu:windowMenu];
    [windowMenuItem setSubmenu:windowMenu];

    [windowMenu addItemWithTitle:@"Minimize"
                          action:@selector(performMiniaturize:)
                   keyEquivalent:@"m"];
    [windowMenu addItemWithTitle:@"Zoom"
                          action:@selector(performZoom:)
                   keyEquivalent:@""];
    [windowMenu addItem:[NSMenuItem separatorItem]];
    [windowMenu addItemWithTitle:@"Bring All to Front"
                          action:@selector(arrangeInFront:)
                   keyEquivalent:@""];

    // TODO: Make this appear at the bottom of the menu (for consistency)
    [windowMenu addItem:[NSMenuItem separatorItem]];
    [[windowMenu addItemWithTitle:@"Enter Full Screen"
                           action:@selector(toggleFullScreen:)
                    keyEquivalent:@"f"]
     setKeyEquivalentModifierMask:NSEventModifierFlagControl | NSEventModifierFlagCommand];

    // Prior to Snow Leopard, we need to use this oddly-named semi-private API
    // to get the application menu working properly.
    SEL setAppleMenuSelector = NSSelectorFromString(@"setAppleMenu:");
    [NSApp performSelector:setAppleMenuSelector withObject:appMenu];
}

internal os_keyboard_key OSX_Translate_Key(int VKCode) {
    switch(VKCode) {
        case kVK_ANSI_A: return OS_KEYBOARD_KEY_A;
        case kVK_ANSI_B: return OS_KEYBOARD_KEY_B;
        case kVK_ANSI_C: return OS_KEYBOARD_KEY_C;
        case kVK_ANSI_D: return OS_KEYBOARD_KEY_D;
        case kVK_ANSI_E: return OS_KEYBOARD_KEY_E;
        case kVK_ANSI_F: return OS_KEYBOARD_KEY_F;
        case kVK_ANSI_G: return OS_KEYBOARD_KEY_G;
        case kVK_ANSI_H: return OS_KEYBOARD_KEY_H;
        case kVK_ANSI_I: return OS_KEYBOARD_KEY_I;
        case kVK_ANSI_J: return OS_KEYBOARD_KEY_J;
        case kVK_ANSI_K: return OS_KEYBOARD_KEY_K;
        case kVK_ANSI_L: return OS_KEYBOARD_KEY_L;
        case kVK_ANSI_M: return OS_KEYBOARD_KEY_M;
        case kVK_ANSI_N: return OS_KEYBOARD_KEY_N;
        case kVK_ANSI_O: return OS_KEYBOARD_KEY_O;
        case kVK_ANSI_P: return OS_KEYBOARD_KEY_P;
        case kVK_ANSI_Q: return OS_KEYBOARD_KEY_Q;
        case kVK_ANSI_R: return OS_KEYBOARD_KEY_R;
        case kVK_ANSI_S: return OS_KEYBOARD_KEY_S;
        case kVK_ANSI_T: return OS_KEYBOARD_KEY_T;
        case kVK_ANSI_U: return OS_KEYBOARD_KEY_U;
        case kVK_ANSI_V: return OS_KEYBOARD_KEY_V;
        case kVK_ANSI_W: return OS_KEYBOARD_KEY_W;
        case kVK_ANSI_X: return OS_KEYBOARD_KEY_X;
        case kVK_ANSI_Y: return OS_KEYBOARD_KEY_Y;
        case kVK_ANSI_Z: return OS_KEYBOARD_KEY_Z;
        case kVK_ANSI_0: return OS_KEYBOARD_KEY_ZERO; 
        case kVK_ANSI_1: return OS_KEYBOARD_KEY_ONE;
        case kVK_ANSI_2: return OS_KEYBOARD_KEY_TWO;
        case kVK_ANSI_3: return OS_KEYBOARD_KEY_THREE;
        case kVK_ANSI_4: return OS_KEYBOARD_KEY_FOUR;
        case kVK_ANSI_5: return OS_KEYBOARD_KEY_FIVE;
        case kVK_ANSI_6: return OS_KEYBOARD_KEY_SIX;
        case kVK_ANSI_7: return OS_KEYBOARD_KEY_SEVEN;
        case kVK_ANSI_8: return OS_KEYBOARD_KEY_EIGHT;
        case kVK_ANSI_9: return OS_KEYBOARD_KEY_NINE;
        case kVK_Space: return OS_KEYBOARD_KEY_SPACE;
        case kVK_Tab: return OS_KEYBOARD_KEY_TAB;
        case kVK_Escape: return OS_KEYBOARD_KEY_ESCAPE;
        case kVK_UpArrow: return OS_KEYBOARD_KEY_UP;
        case kVK_DownArrow: return OS_KEYBOARD_KEY_DOWN;
        case kVK_LeftArrow: return OS_KEYBOARD_KEY_LEFT; 
        case kVK_RightArrow: return OS_KEYBOARD_KEY_RIGHT;
        case kVK_Delete: return OS_KEYBOARD_KEY_DELETE;
        case kVK_Return: return OS_KEYBOARD_KEY_RETURN;
        case kVK_ForwardDelete: return OS_KEYBOARD_KEY_DELETE;
        case kVK_Help: return OS_KEYBOARD_KEY_INSERT; 
        case kVK_Shift: return OS_KEYBOARD_KEY_SHIFT;
        case kVK_Control: return OS_KEYBOARD_KEY_CONTROL;
        case kVK_Option: return OS_KEYBOARD_KEY_ALT;
        case kVK_F1: return OS_KEYBOARD_KEY_F1;
        case kVK_F2: return OS_KEYBOARD_KEY_F2;
        case kVK_F3: return OS_KEYBOARD_KEY_F3;
        case kVK_F4: return OS_KEYBOARD_KEY_F4;
        case kVK_F5: return OS_KEYBOARD_KEY_F5;
        case kVK_F6: return OS_KEYBOARD_KEY_F6; 
        case kVK_F7: return OS_KEYBOARD_KEY_F7; 
        case kVK_F8: return OS_KEYBOARD_KEY_F8; 
        case kVK_F9: return OS_KEYBOARD_KEY_F9; 
        case kVK_F10: return OS_KEYBOARD_KEY_F10;
        case kVK_F11: return OS_KEYBOARD_KEY_F11; 
        case kVK_F12: return OS_KEYBOARD_KEY_F12;   
        case kVK_Command: return OS_KEYBOARD_KEY_COMMAND;
    }

    return -1;
}

internal NSUInteger OSX_Translate_Key_To_Modifier_Flag(os_keyboard_key Key) {
    switch (Key)
    {
        case OS_KEYBOARD_KEY_SHIFT:
            return NSEventModifierFlagShift;
        case OS_KEYBOARD_KEY_CONTROL:
            return NSEventModifierFlagControl;
        case OS_KEYBOARD_KEY_ALT:
            return NSEventModifierFlagOption;
        case OS_KEYBOARD_KEY_COMMAND:
            return NSEventModifierFlagCommand;
    }

    return 0;
}
