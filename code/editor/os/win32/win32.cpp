#include "win32.h"

#define GET_X_LPARAM(lp)    ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)    ((int)(short)HIWORD(lp))
#define Win32_Message(message) (WM_USER+WIN32_MESSAGE_##message)
#define OS_Monitor_ID(index) (((u64)index << 32) | UINT32_MAX)
#define OS_Monitor_Index(ID) (u32)(ID >> 32)

#define WIN32_GLOBAL_WINDOW_CLASS L"AK_Engine Main Window Class"

internal os_keyboard_key Win32_Translate_Key(int VKCode);

internal string Win32_Get_Executable_Path(allocator* Allocator) {
    DWORD MemorySize = 1024;
    
    for(int Iterations = 0; Iterations < 32; Iterations++) {
        scratch Scratch = Scratch_Get();
        wchar_t* Buffer = (wchar_t*)Scratch_Push(&Scratch, sizeof(wchar_t)*MemorySize);
        DWORD Size = GetModuleFileNameW(nullptr, Buffer, MemorySize);
        if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            return string(Allocator, wstring(Buffer, Size));
        }
        MemorySize *= 2;
    }

    return string();
}

string OS_Get_Executable_Path() {
    os* OS = OS_Get();
    return OS->ExecutablePath;
}

void OS_Message_Box(string Message, string Title) {
    scratch Scratch = Scratch_Get();
    wstring TitleW(&Scratch, Title);
    wstring MessageW(&Scratch, Message);
    MessageBoxW(NULL, MessageW.Str, TitleW.Str, MB_OK);
}

os_process_id OS_Exec_Process(string App, string Parameters) {
    os* OS = OS_Get();
    Assert(OS);

    STARTUPINFOW StartupInfo = {
        .cb = sizeof(StartupInfo)
    };

    PROCESS_INFORMATION ProcessInformation = {};

    scratch Scratch = Scratch_Get();

    string CmdLine = String_Concat(&Scratch, {String_Lit("C:\\Windows\\System32\\cmd.exe /c "), App, String_Lit(" "), Parameters});
    wstring CmdLineW(&Scratch, CmdLine);

    if(!CreateProcessW(nullptr, (LPWSTR)CmdLineW.Str, nullptr, nullptr, FALSE, 0, nullptr, 
                       nullptr, &StartupInfo, &ProcessInformation)) {
        return 0;
    }
    
    async_handle<os_process> Handle = Async_Pool_Allocate(&OS->ProcessPool);
    os_process* Process = Async_Pool_Get(&OS->ProcessPool, Handle);
    Process->Information = ProcessInformation;

    return Handle.ID;
}

internal os_process* OS_Get_Process(os_process_id ID) {
    os* OS = OS_Get();
    Assert(OS);
    async_handle<os_process> Handle(ID);
    os_process* Process = Async_Pool_Get(&OS->ProcessPool, Handle);
    Assert(Process);
    return Process;
}

void OS_Wait_For_Process(os_process_id ID) {
    os_process* Process = OS_Get_Process(ID);
    if(!Process) return;
    WaitForSingleObject(Process->Information.hProcess, INFINITE);
}

os_process_status OS_Process_Status(os_process_id ID) {
    os_process_status Result = OS_PROCESS_STATUS_NONE;
    os_process* Process = OS_Get_Process(ID);
    if(!Process) return Result;

    DWORD ExitCode;
    if(GetExitCodeProcess(Process->Information.hProcess, &ExitCode)) {
        Result = ExitCode == STILL_ACTIVE ? OS_PROCESS_STATUS_ACTIVE : OS_PROCESS_STATUS_EXIT; 
    }
    return Result;
}

int OS_Process_Exit(os_process_id ID) {
    DWORD ExitCode = (DWORD)-1;
    os_process* Process = OS_Get_Process(ID);
    if(GetExitCodeProcess(Process->Information.hProcess, &ExitCode)) {
        if(ExitCode == STILL_ACTIVE) {
            WaitForSingleObject(Process->Information.hProcess, INFINITE);
        }
        
        GetExitCodeProcess(Process->Information.hProcess, &ExitCode);
        Assert(ExitCode != STILL_ACTIVE);
    }
    os* OS = OS_Get();
    Async_Pool_Free(&OS->ProcessPool, async_handle<os_process>(ID));
    return (int)ExitCode;
}

span<os_monitor_id> OS_Get_Monitors() {
    os* OS = OS_Get();
    return OS->MonitorIDs;
}

os_monitor_id OS_Get_Primary_Monitor() {
    os* OS = OS_Get();
    return OS->PrimaryMonitor;
}

const os_monitor_info* OS_Get_Monitor_Info(os_monitor_id ID) {
    os* OS = OS_Get();
    return &OS->Monitors[OS_Monitor_Index(ID)].MonitorInfo;
}     

internal os_window* OS_Window_Get(os_window_id WindowID) {
    os* OS = OS_Get();
    os_window* Window = Async_Pool_Get(&OS->WindowPool, async_handle<os_window>(WindowID));
    return Window;
}

internal bool OS_Open_Window_Internal(os_window* Window, const os_open_window_info& OpenInfo) {
    os* OS = OS_Get();
    win32_open_window_request OpenWindowRequest = {
        .Window = Window,
        .OpenInfo = &OpenInfo,
        .Result = true
    };

    SendMessageW(OS->BaseWindow, Win32_Message(OPEN_WINDOW), (WPARAM)&OpenWindowRequest, 0);

    return OpenWindowRequest.Result;
}

os_window_id OS_Open_Window(const os_open_window_info& OpenInfo) {
    os* OS = OS_Get();
    async_handle<os_window> Handle = Async_Pool_Allocate(&OS->WindowPool);
    os_window* Window = Async_Pool_Get(&OS->WindowPool, Handle);

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
    if(!Window) return;
    
    win32_close_window_request CloseWindowRequest = {
        .Window = Window
    };

    SendMessageW(OS_Get()->BaseWindow, Win32_Message(CLOSE_WINDOW), (WPARAM)&CloseWindowRequest, 0);
    Async_Pool_Free(&OS_Get()->WindowPool, async_handle<os_window>(WindowID));
}

bool OS_Window_Is_Open(os_window_id WindowID) {
    os* OS = OS_Get();
    return Async_Pool_Valid_Handle(&OS->WindowPool, async_handle<os_window>(WindowID));
}

void OS_Set_Main_Window(os_window_id WindowID) {
    os* OS = OS_Get();
    AK_Atomic_Store_U64_Relaxed(&OS->MainWindowID, WindowID);
}

os_window_id OS_Get_Main_Window() {
    os* OS = OS_Get();
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
    os_window* Window = OS_Window_Get(WindowID);
    if(!Window) return;

    scratch Scratch = Scratch_Get();

    wstring TitleW(&Scratch, Title);
    SendMessageW(Window->Handle, Win32_Message(SET_TITLE), (WPARAM)&TitleW, 0);
}

dim2i OS_Window_Get_Size(os_window_id WindowID) {
    os_window* Window = OS_Window_Get(WindowID);
    if(!Window) return dim2i();
    s64 SizePacked = (s64)AK_Atomic_Load_U64_Relaxed(&Window->SizePacked);
    return Unpack_S64(SizePacked);
}

point2i OS_Window_Get_Pos(os_window_id WindowID) {
    os_window* Window = OS_Window_Get(WindowID);
    if(!Window) return {};

    s64 PosPacked = (s64)AK_Atomic_Load_U64_Relaxed(&Window->PosPacked);
    return Unpack_S64(PosPacked);
}

point2i OS_Window_Get_Client_Pos(os_window_id WindowID) {
    os_window* Window = OS_Window_Get(WindowID);
    if(!Window) return {};

    s64 PosPacked = (s64)AK_Atomic_Load_U64_Relaxed(&Window->ClientPosPacked);
    return Unpack_S64(PosPacked);
}

gdi_window_data OS_Window_Get_GDI_Data(os_window_id WindowID) {
    os_window* Window = OS_Window_Get(WindowID);
    if(!Window) return {};

    return {
        .Win32 = {
            .Window = Window->Handle,
            .Instance = GetModuleHandleW(nullptr)
        } 
    };
}

bool OS_Window_Is_Resizing(os_window_id WindowID) {
    os_window* Window = OS_Window_Get(WindowID);
    if(!Window) return false;
    return AK_Atomic_Load_U32_Relaxed(&Window->IsResizing) == true32 || AK_Atomic_Load_U32_Relaxed(&Window->IsPainting) == true32;
}

bool OS_Window_Is_Focused(os_window_id WindowID) {
    os_window* Window = OS_Window_Get(WindowID);
    if(!Window) return false;
    return Window->Handle == GetForegroundWindow();
}

void OS_Set_Draw_Window_Callback(os_draw_window_callback_func* Callback, void* UserData) {
    os* OS = OS_Get();
    AK_Atomic_Store_Ptr_Relaxed(&OS->DrawWindowUserData, UserData);
    AK_Atomic_Store_Ptr(&OS->DrawWindowCallbackFunc, Callback, AK_ATOMIC_MEMORY_ORDER_RELEASE);
}

bool OS_Keyboard_Get_Key_State(os_keyboard_key Key) {
    //Command is not supported on win32
    if(Key == OS_KEYBOARD_KEY_COMMAND) {
        return false;
    }

    int VKCode = (int)G_VKCodes[Key];
    return (GetAsyncKeyState(VKCode) & 0x8000) != 0;
}

bool OS_Mouse_Get_Key_State(os_mouse_key Key) {
    int VKCode = (int)G_MouseVKCodes[Key];
    return (GetAsyncKeyState(VKCode) & 0x8000) != 0;
}

point2i OS_Mouse_Get_Position() {
    os* OS = OS_Get();
    POINT P;
    GetCursorPos(&P);
    return point2i(P.x, P.y);
}


internal THREAD_CONTEXT_CALLBACK(Win32_Application_Thread) {
    HANDLE Handle = GetCurrentThread();
    Assert(SetThreadPriority(Handle, THREAD_PRIORITY_HIGHEST));

    os* OS = OS_Get();
    OS->AppResult = Application_Main();
    SendNotifyMessageW(OS->BaseWindow, Win32_Message(QUIT), 0, 0); //This can be async
    return 0;
}

internal HWND Win32_Create_Window(WNDCLASSEXW* WindowClass, DWORD Style, LONG XOffset, LONG YOffset, LONG Width, LONG Height, string Title, void* UserData) {
    DWORD ExStyle = 0;
    
    scratch Scratch = Scratch_Get();
    wstring TitleW(&Scratch, Title);
    HWND Result = CreateWindowExW(ExStyle, WindowClass->lpszClassName, TitleW.Str, Style, 
                                  XOffset, YOffset, Width, Height,
                                  NULL, NULL, WindowClass->hInstance, UserData);

    return Result;
}

#include <stdio.h>

internal LRESULT Win32_OS_Main_Window_Proc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
    LRESULT Result = 0;
    bool DefaultMessage = false; 


    os* OS = OS_Get();
    switch(Message) {
        case Win32_Message(SET_TITLE): {
            wstring* String = (wstring*)WParam;
            SetWindowTextW(Window, String->Str);
        } break;

        case WM_CREATE: {
            CREATESTRUCTW* CreateStruct = (CREATESTRUCTW*)LParam;
            os_window* OSWindow = (os_window*)CreateStruct->lpCreateParams;
            OSWindow->Handle = Window;
            SetWindowLongPtrW(Window, GWLP_USERDATA, (LONG_PTR)OSWindow);
        } break;

        case WM_CLOSE: {
            os_window* OSWindow = (os_window*)GetWindowLongPtrW(Window, GWLP_USERDATA);
            Assert(OSWindow->Handle == Window);

            os_event* Event = OS_Event_Stream_Allocate_Event(OS->EventStream, OS_EVENT_TYPE_WINDOW_CLOSED);
            Event->Window = OSWindow->ID;
        } break;

        case WM_ERASEBKGND: {
            Result = 1;
        } break;

        case WM_ENTERSIZEMOVE:
        {
            os_window* OSWindow = (os_window*)GetWindowLongPtrW(Window, GWLP_USERDATA);
            Assert(OSWindow->Handle == Window);
            AK_Atomic_Store_U32_Relaxed(&OSWindow->IsResizing, true);
        }break;
        
        case WM_EXITSIZEMOVE:
        {
            os_window* OSWindow = (os_window*)GetWindowLongPtrW(Window, GWLP_USERDATA);
            Assert(OSWindow->Handle == Window);
            AK_Atomic_Store_U32_Relaxed(&OSWindow->IsResizing, false);
        }break;

        case WM_WINDOWPOSCHANGED: {
            os_window* OSWindow = (os_window*)GetWindowLongPtrW(Window, GWLP_USERDATA);
            Assert(OSWindow->Handle == Window);

            WINDOWPOS* WindowPos = (WINDOWPOS*)LParam;
            AK_Atomic_Store_U64_Relaxed(&OSWindow->PosPacked, (u64)Pack_S64(WindowPos->x, WindowPos->y));

            RECT WindowRect;
            GetClientRect(Window, &WindowRect);
            
            POINT Point = {WindowRect.left, WindowRect.top};
            ClientToScreen(Window, &Point);
            AK_Atomic_Store_U64_Relaxed(&OSWindow->ClientPosPacked, (u64)Pack_S64(Point.x, Point.y));

            DefaultMessage = true; //Other we will not get WM_SIZE messages
        } break;

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN: 
        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            os_keyboard_key Key = Win32_Translate_Key((int)WParam);
            if(Key != -1) { 
                os_window* OSWindow = (os_window*)GetWindowLongPtrW(Window, GWLP_USERDATA);
                Assert(OSWindow->Handle == Window);

                bool WasDown = (LParam & (1 << 30)) != 0;
                bool IsDown = (LParam & (1 << 31)) == 0;
                if(WasDown != IsDown) {
                    if(IsDown) {
                        os_keyboard_event* Event = (os_keyboard_event*)OS_Event_Stream_Allocate_Event(OS->EventStream, OS_EVENT_TYPE_KEY_PRESSED);
                        Event->Window = OSWindow->ID;
                        Event->Key = Key;
                    } else {
                        os_keyboard_event* Event = (os_keyboard_event*)OS_Event_Stream_Allocate_Event(OS->EventStream, OS_EVENT_TYPE_KEY_RELEASED);
                        Event->Window = OSWindow->ID;
                        Event->Key = Key;
                    }
                }
            }
        } break;

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN: {
            os_window* OSWindow = (os_window*)GetWindowLongPtrW(Window, GWLP_USERDATA);
            Assert(OSWindow->Handle == Window);
            os_mouse_key MouseKey = (Message == WM_LBUTTONDOWN) ? OS_MOUSE_KEY_LEFT : (Message == WM_MBUTTONDOWN ? OS_MOUSE_KEY_MIDDLE : OS_MOUSE_KEY_RIGHT); 
            os_mouse_event* MouseEvent = (os_mouse_event*)OS_Event_Stream_Allocate_Event(OS->EventStream, OS_EVENT_TYPE_MOUSE_PRESSED);
            MouseEvent->Window = OSWindow->ID;
            MouseEvent->Key = MouseKey;
        } break;

        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            os_window* OSWindow = (os_window*)GetWindowLongPtrW(Window, GWLP_USERDATA);
            Assert(OSWindow->Handle == Window);
            os_mouse_key MouseKey = (Message == WM_LBUTTONUP) ? OS_MOUSE_KEY_LEFT : (Message == WM_MBUTTONUP ? OS_MOUSE_KEY_MIDDLE : OS_MOUSE_KEY_RIGHT); 
            os_mouse_event* MouseEvent = (os_mouse_event*)OS_Event_Stream_Allocate_Event(OS->EventStream, OS_EVENT_TYPE_MOUSE_RELEASED);
            MouseEvent->Window = OSWindow->ID;
            MouseEvent->Key = MouseKey;
        } break;

        case WM_MOUSEMOVE: {
            os_window* OSWindow = (os_window*)GetWindowLongPtrW(Window, GWLP_USERDATA);
            Assert(OSWindow->Handle == Window);
            if(!OSWindow->MouseTracked) {
                TRACKMOUSEEVENT TrackEvent = {sizeof(TrackEvent), TME_LEAVE, Window, 0};
                TrackMouseEvent(&TrackEvent);
                OSWindow->MouseTracked = true;

                os_event* Event = OS_Event_Stream_Allocate_Event(OS->EventStream, OS_EVENT_TYPE_MOUSE_ENTERED);
                Event->Window = OSWindow->ID;
            }

            os_mouse_move_event* MouseEvent = (os_mouse_move_event*)OS_Event_Stream_Allocate_Event(OS->EventStream, OS_EVENT_TYPE_MOUSE_MOVE);
            MouseEvent->Window = OSWindow->ID;
            MouseEvent->Pos = point2i(GET_X_LPARAM(LParam), GET_Y_LPARAM(LParam));
        } break;

        case WM_MOUSELEAVE: {
            os_window* OSWindow = (os_window*)GetWindowLongPtrW(Window, GWLP_USERDATA);
            Assert(OSWindow->Handle == Window);
            OSWindow->MouseTracked = false;
            os_event* Event = OS_Event_Stream_Allocate_Event(OS->EventStream, OS_EVENT_TYPE_MOUSE_EXITED);
            Event->Window = OSWindow->ID;
        } break;

        case WM_MOUSEWHEEL: {
            os_window* OSWindow = (os_window*)GetWindowLongPtrW(Window, GWLP_USERDATA);
            Assert(OSWindow->Handle == Window);
            f32 Scroll = (f32)GET_WHEEL_DELTA_WPARAM(WParam)/(f32)WHEEL_DELTA;
            os_mouse_scroll_event* Event = (os_mouse_scroll_event*)OS_Event_Stream_Allocate_Event(OS->EventStream, OS_EVENT_TYPE_MOUSE_SCROLL);
            Event->Window = OSWindow->ID;
            Event->Scroll = Scroll;     
        } break;

        case WM_SIZE:
        case WM_PAINT:
        {
            os_window* OSWindow = (os_window*)GetWindowLongPtrW(Window, GWLP_USERDATA);
            Assert(OSWindow->Handle == Window);

            AK_Atomic_Store_U32_Relaxed(&OSWindow->IsPainting, true);

            RECT WindowRect;
            GetClientRect(Window, &WindowRect);
            dim2i Dim = dim2i(WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top);
            AK_Atomic_Store_U64_Relaxed(&OSWindow->SizePacked, (u64)Pack_S64(Dim.Data));

            os_draw_window_callback_func* Draw_Window = (os_draw_window_callback_func*)AK_Atomic_Load_Ptr(&OS->DrawWindowCallbackFunc, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
            if(Draw_Window) {
                void* UserData = AK_Atomic_Load_Ptr_Relaxed(&OS->DrawWindowUserData);
                PAINTSTRUCT PaintStruct;
                BeginPaint(Window, &PaintStruct);
                Draw_Window(OSWindow->ID, UserData);
                EndPaint(Window, &PaintStruct);
            } else {
                DefaultMessage = true;
            }

            AK_Atomic_Store_U32_Relaxed(&OSWindow->IsPainting, false);
        } break;

        case WM_NCCALCSIZE: {
            DefaultMessage = true;
        }break;

        default: {
            DefaultMessage = true;
        } break;
    }

    if(DefaultMessage)
        Result = DefWindowProcW(Window, Message, WParam, LParam);

    return Result;
}

internal LRESULT Win32_OS_Base_Window_Proc(HWND BaseWindow, UINT Message, WPARAM WParam, LPARAM LParam) {
    LRESULT Result = 0;

    os* OS = OS_Get();
    switch(Message) {
        case Win32_Message(QUIT): {
            PostQuitMessage(0);
        } break;

        case Win32_Message(OPEN_WINDOW): {
            win32_open_window_request* OpenWindowRequest = (win32_open_window_request*)WParam;
            os_window* Window = OpenWindowRequest->Window;
            const os_open_window_info* OpenInfo = OpenWindowRequest->OpenInfo;

            os_monitor* Monitor = &OS->Monitors[OS_Monitor_Index(OpenInfo->Monitor)];

            point2i Origin;
            dim2i Dim;
            if(OpenInfo->Flags & OS_WINDOW_FLAG_MAXIMIZE_BIT) {
                rect2i Rect = Monitor->MonitorInfo.Rect;
                Origin = point2i(Rect.P1.x, Rect.P1.y);
                Dim = Rect2i_Get_Dim(Rect);
            } else {
                Origin = OpenInfo->Pos;
                Origin += Monitor->MonitorInfo.Rect.P1;
                Dim = OpenInfo->Size;
            }

            AK_Atomic_Store_U64_Relaxed(&Window->PosPacked, (u64)Pack_S64(Origin.x, Origin.y));
            AK_Atomic_Store_U64_Relaxed(&Window->SizePacked, (u64)Pack_S64(Dim.width, Dim.height));

            DWORD ExStyle = WS_EX_APPWINDOW;
            DWORD Style = WS_OVERLAPPEDWINDOW|WS_SIZEBOX;

            RECT WindowRect = {Origin.x, Origin.y, Origin.x+Dim.width, Origin.y+Dim.height};
            
            RECT Rect;
            SetRectEmpty(&Rect);
            AdjustWindowRectEx(&Rect, Style, FALSE, ExStyle);
            WindowRect.left -= Rect.left;
            WindowRect.top -= Rect.top;

            AK_Atomic_Store_U64_Relaxed(&Window->ClientPosPacked, (u64)Pack_S64(WindowRect.left, WindowRect.top));

            Window->Handle = CreateWindowExW(ExStyle, WIN32_GLOBAL_WINDOW_CLASS, L"", Style, 
                                             Origin.x, Origin.y, Dim.width, Dim.height, 
                                             nullptr, nullptr, GetModuleHandleW(nullptr), Window);
            if(!Window->Handle) {
                OpenWindowRequest->Result = false;
                return 0;
            }

            if(OpenInfo->Flags & OS_WINDOW_FLAG_MAXIMIZE_BIT) {
                ShowWindow(Window->Handle, SW_MAXIMIZE);
                UpdateWindow(Window->Handle);
            } else {
                ShowWindow(Window->Handle, SW_NORMAL);
                UpdateWindow(Window->Handle);
            }

        } break;

        case Win32_Message(CLOSE_WINDOW): {
            win32_close_window_request* CloseWindowRequest = (win32_close_window_request*)WParam;
            os_window* Window = CloseWindowRequest->Window;
            DestroyWindow(Window->Handle);
        } break;

        default: {
            Result = DefWindowProcW(BaseWindow, Message, WParam, LParam);
        } break;
    }

    return Result;
}

internal BOOL CALLBACK Win32_Monitor_Enum_Proc(HMONITOR Monitor, HDC DeviceContext, LPRECT pRect, LPARAM Data) {
    os* OS = OS_Get();
    HMONITOR PrimaryMonitor = (HMONITOR)Data;

    if(PrimaryMonitor == Monitor) {
        OS->PrimaryMonitor = OS_Monitor_ID(Safe_U32(OS->Monitors.Count));
    }

    MONITORINFOEXW MonitorInfo = {};
    MonitorInfo.cbSize = sizeof(MONITORINFOEXW);

    GetMonitorInfoW(Monitor, &MonitorInfo);

    Array_Push(&OS->MonitorIDs, OS_Monitor_ID(Safe_U32(OS->Monitors.Count)));
    Array_Push(&OS->Monitors, {
        .Monitor = Monitor,
        .DeviceName = wstring(OS->Arena, MonitorInfo.szDevice),
        .MonitorInfo = {
            .Rect = rect2i(point2i(pRect->left, pRect->top), 
                           point2i(pRect->right, pRect->bottom))
        }
    });

    return TRUE;
}

int main() {
    if(!Core_Create()) {
        //todo: Logging
        return 1;
    }

    os OS = {};
    OS.Arena = Arena_Create(Core_Get_Base_Allocator());
    OS_Set(&OS);

    OS_Event_Manager_Create(&OS.EventManager, 1);
    OS.EventStream = OS_Event_Manager_Allocate_Stream(&OS.EventManager);

    {
        scratch Scratch = Scratch_Get();
        string ExecutableFilePath = Win32_Get_Executable_Path(&Scratch);
        OS.ExecutablePath = string(OS.Arena, String_Get_Path(ExecutableFilePath));
    }

    WNDCLASSEXW BaseWindowClass = {
        .cbSize = sizeof(WNDCLASSEXW),
        .lpfnWndProc = Win32_OS_Base_Window_Proc,
        .hInstance = GetModuleHandleW(NULL),
        .lpszClassName = L"AK_Engine Base Window Class"        
    };

    WNDCLASSEXW MainWindowClass = {
        .cbSize = sizeof(WNDCLASSEXW),
        .style = CS_VREDRAW|CS_HREDRAW,
        .lpfnWndProc = Win32_OS_Main_Window_Proc,
        .hInstance = GetModuleHandleW(NULL),
        .lpszClassName = WIN32_GLOBAL_WINDOW_CLASS,
    };

    if(!RegisterClassExW(&BaseWindowClass)) {
        //todo: Logging
        return 1;
    }

    if(!RegisterClassExW(&MainWindowClass)) {
        //todo: Logging
        return 1;
    }

    OS.BaseWindow = Win32_Create_Window(&BaseWindowClass, 0, 0, 0, 0, 0, String_Lit(""), NULL);
    if(!OS.BaseWindow) {
        //todo: Logging
        return 1;
    }

    RAWINPUTDEVICE RawInputDevice = {
        .usUsagePage = WIN32_USAGE_PAGE,
        .usUsage = WIN32_MOUSE_USAGE,
        .dwFlags = RIDEV_INPUTSINK,
        .hwndTarget = OS.BaseWindow
    };

    if(!RegisterRawInputDevices(&RawInputDevice, 1, sizeof(RAWINPUTDEVICE))) {
        //TODO(JJ): logging
        return 1;
    }

    Array_Init(&OS.Monitors, OS.Arena, 8);
    Array_Init(&OS.MonitorIDs, OS.Arena, 8);
    
    HMONITOR Primary = MonitorFromPoint({}, MONITOR_DEFAULTTOPRIMARY);
    EnumDisplayMonitors(nullptr, nullptr, Win32_Monitor_Enum_Proc, (LPARAM)Primary);

    for(uptr i = 0; i < OS.Monitors.Count; i++) {
        DISPLAY_DEVICEW DisplayDevice = {};
        DisplayDevice.cb = sizeof(DisplayDevice);
        BOOL Status = EnumDisplayDevicesW(OS.Monitors[i].DeviceName.Str, 0, &DisplayDevice, 0);
        Assert(Status);
        OS.Monitors[i].MonitorInfo.Name = string(OS.Arena, wstring(DisplayDevice.DeviceString));
    }

    Async_Pool_Create(&OS.ProcessPool, OS.Arena, OS_MAX_PROCESS_COUNT);
    Async_Pool_Create(&OS.WindowPool, OS.Arena, OS_MAX_WINDOW_COUNT);

    if(!Thread_Manager_Create_Thread(Win32_Application_Thread, nullptr)) {
        return 1;
    }

    for(;;) {
        MSG Message;
        if(!PeekMessageW(&Message, NULL, 0, 0, PM_REMOVE)) {
            OS_Event_Manager_Push_Back_Stream(&OS.EventManager, OS.EventStream);
            OS.EventStream = OS_Event_Manager_Allocate_Stream(&OS.EventManager);

            BOOL Status = GetMessageW(&Message, NULL, 0, 0);
            Assert(Status != -1);
        }

        switch(Message.message) {
            case WM_QUIT: {
                Core_Delete();
                return 0;
            } break;

            case WM_INPUT: {
                scratch Scratch = Scratch_Get();

                UINT RawInputSize;
                GetRawInputData((HRAWINPUT)Message.lParam, RID_INPUT, nullptr, &RawInputSize, sizeof(RAWINPUTHEADER));
                RAWINPUT* RawInput = (RAWINPUT*)Scratch_Push(&Scratch, RawInputSize); 
                GetRawInputData((HRAWINPUT)Message.lParam, RID_INPUT, RawInput, &RawInputSize, sizeof(RAWINPUTHEADER));

                switch(RawInput->header.dwType) {
                    case RIM_TYPEMOUSE: {
                        RAWMOUSE* Mouse = &RawInput->data.mouse;
                        if(Mouse->lLastX != 0 || Mouse->lLastY != 0) {
                            vec2i Delta = vec2i(Mouse->lLastX, -Mouse->lLastY);
                            os_mouse_delta_event* Event = (os_mouse_delta_event*)OS_Event_Stream_Allocate_Event(OS.EventStream, OS_EVENT_TYPE_MOUSE_DELTA);
                            Event->Delta = Delta;
                        }
                    } break;
                }
            } break;

            default: {
                TranslateMessage(&Message);
                DispatchMessageW(&Message);
            } break;
        }
    }
}

global os* G_OS;
os* OS_Get() {
    Assert(G_OS);
    return G_OS;
}

void OS_Set(os* OS) {
    Assert(!G_OS);
    G_OS = OS;
}

#pragma comment(lib, "ftsystem.lib")
#pragma comment(lib, "hb.lib")
#pragma comment(lib, "sheenbidi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Dwmapi.lib")
#include <core/core.cpp>
#include <os_event.cpp>

internal os_keyboard_key Win32_Translate_Key(int VKCode) {
    switch(VKCode) {
        case 'A': return OS_KEYBOARD_KEY_A;
        case 'B': return OS_KEYBOARD_KEY_B;
        case 'C': return OS_KEYBOARD_KEY_C;
        case 'D': return OS_KEYBOARD_KEY_D;
        case 'E': return OS_KEYBOARD_KEY_E;
        case 'F': return OS_KEYBOARD_KEY_F;
        case 'G': return OS_KEYBOARD_KEY_G;
        case 'H': return OS_KEYBOARD_KEY_H;
        case 'I': return OS_KEYBOARD_KEY_I; 
        case 'J': return OS_KEYBOARD_KEY_J;
        case 'K': return OS_KEYBOARD_KEY_K;
        case 'L': return OS_KEYBOARD_KEY_L;
        case 'M': return OS_KEYBOARD_KEY_M;
        case 'N': return OS_KEYBOARD_KEY_N;
        case 'O': return OS_KEYBOARD_KEY_O;
        case 'P': return OS_KEYBOARD_KEY_P;
        case 'Q': return OS_KEYBOARD_KEY_Q;
        case 'R': return OS_KEYBOARD_KEY_R;
        case 'S': return OS_KEYBOARD_KEY_S;
        case 'T': return OS_KEYBOARD_KEY_T;
        case 'U': return OS_KEYBOARD_KEY_U;
        case 'V': return OS_KEYBOARD_KEY_V;
        case 'W': return OS_KEYBOARD_KEY_W;
        case 'X': return OS_KEYBOARD_KEY_X;
        case 'Y': return OS_KEYBOARD_KEY_Y;
        case 'Z': return OS_KEYBOARD_KEY_Z;
        case '0': return OS_KEYBOARD_KEY_ZERO;
        case '1': return OS_KEYBOARD_KEY_ONE;
        case '2': return OS_KEYBOARD_KEY_TWO;
        case '3': return OS_KEYBOARD_KEY_THREE;
        case '4': return OS_KEYBOARD_KEY_FOUR;
        case '5': return OS_KEYBOARD_KEY_FIVE;
        case '6': return OS_KEYBOARD_KEY_SIX;
        case '7': return OS_KEYBOARD_KEY_SEVEN;
        case '8': return OS_KEYBOARD_KEY_EIGHT;
        case '9': return OS_KEYBOARD_KEY_NINE;
        case VK_SPACE: return OS_KEYBOARD_KEY_SPACE;
        case VK_TAB: return OS_KEYBOARD_KEY_TAB;
        case VK_ESCAPE: return OS_KEYBOARD_KEY_ESCAPE;
        case VK_UP: return OS_KEYBOARD_KEY_UP;
        case VK_DOWN: return OS_KEYBOARD_KEY_DOWN;
        case VK_LEFT: return OS_KEYBOARD_KEY_LEFT;
        case VK_RIGHT: return OS_KEYBOARD_KEY_RIGHT;
        case VK_BACK: return OS_KEYBOARD_KEY_BACKSPACE;
        case VK_RETURN: return OS_KEYBOARD_KEY_RETURN;
        case VK_DELETE: return OS_KEYBOARD_KEY_DELETE;
        case VK_INSERT: return OS_KEYBOARD_KEY_INSERT;
        case VK_SHIFT: return OS_KEYBOARD_KEY_SHIFT;
        case VK_CONTROL: return OS_KEYBOARD_KEY_CONTROL;
        case VK_MENU: return OS_KEYBOARD_KEY_ALT;
        case VK_F1: return OS_KEYBOARD_KEY_F1;
        case VK_F2: return OS_KEYBOARD_KEY_F2;
        case VK_F3: return OS_KEYBOARD_KEY_F3;
        case VK_F4: return OS_KEYBOARD_KEY_F4;
        case VK_F5: return OS_KEYBOARD_KEY_F5;
        case VK_F6: return OS_KEYBOARD_KEY_F6;
        case VK_F7: return OS_KEYBOARD_KEY_F7;
        case VK_F8: return OS_KEYBOARD_KEY_F8;
        case VK_F9: return OS_KEYBOARD_KEY_F9;
        case VK_F10: return OS_KEYBOARD_KEY_F10;
        case VK_F11: return OS_KEYBOARD_KEY_F11;
        case VK_F12: return OS_KEYBOARD_KEY_F12;
    }

    return (os_keyboard_key)-1;
}