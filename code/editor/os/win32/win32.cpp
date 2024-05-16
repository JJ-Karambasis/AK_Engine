#include "win32.h"

#define OS_Monitor_ID(index) (((u64)index << 32) | UINT32_MAX)
#define OS_Monitor_Index(ID) (u32)(ID >> 32)

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

vec2i OS_Mouse_Get_Delta() {
    os* OS = OS_Get();
    s64 MouseDeltaPacked = (s64)AK_Atomic_Load_U64_Relaxed(&OS->MouseDeltaPacked);
    return vec2i((s32)MouseDeltaPacked, (s32)(MouseDeltaPacked >> 32));
}

f32 OS_Mouse_Get_Scroll() {
    os* OS = OS_Get();
    u32 ScrollU32 = AK_Atomic_Load_U32_Relaxed(&OS->ScrollU32);
    return U32_To_F32(ScrollU32);
}

#define Win32_Message(message) (WM_USER+WIN32_MESSAGE_##message)

internal THREAD_CONTEXT_CALLBACK(Win32_Application_Thread) {
    os* OS = OS_Get();
    OS->AppResult = Application_Main();
    SendNotifyMessageW(OS->BaseWindow, Win32_Message(QUIT), 0, 0);
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

internal LRESULT Win32_OS_Base_Window_Proc(HWND BaseWindow, UINT Message, WPARAM WParam, LPARAM LParam) {
    LRESULT Result = 0;
    switch(Message) {
        case Win32_Message(QUIT): {
            PostQuitMessage(0);
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

    WNDCLASSEXW BaseWindowClass = {
        .cbSize = sizeof(WNDCLASSEXW),
        .lpfnWndProc = Win32_OS_Base_Window_Proc,
        .hInstance = GetModuleHandleW(NULL),
        .lpszClassName = L"AK_Engine Base Window Class"        
    };

    if(!RegisterClassExW(&BaseWindowClass)) {
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

    OS_Event_Manager_Create(&OS.EventManager, 1);

    Async_Pool_Create(&OS.ProcessPool, OS.Arena, OS_MAX_PROCESS_COUNT);

    if(!Thread_Manager_Create_Thread(Win32_Application_Thread, nullptr)) {
        return 1;
    }

    for(;;) {
        MSG Message;
        if(!PeekMessageW(&Message, NULL, 0, 0, PM_REMOVE)) {
            if(OS.EventStream) {
                OS_Event_Manager_Push_Back_Stream(&OS.EventManager, OS.EventStream);
            }
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
                            s64 PackedDelta = ((s64)Delta.x) | (((s64)Delta.y) << 32);
                            AK_Atomic_Store_U64_Relaxed(&OS.MouseDeltaPacked, (u64)PackedDelta);
                        }

                        if(Mouse->usButtonFlags & RI_MOUSE_WHEEL) {
                            f32 Scroll = (f32)((SHORT)Mouse->usButtonData/WHEEL_DELTA);
                            u32 ScrollU32 = F32_To_U32(Scroll);
                            AK_Atomic_Store_U32_Relaxed(&OS.ScrollU32, ScrollU32);
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
#include <core/core.cpp>
#include <math/math.cpp>
#include <os_event.cpp>