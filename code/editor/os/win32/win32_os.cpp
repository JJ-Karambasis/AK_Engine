#include "win32_os.h"

#define Win32__OS_Get_Message(message) (WM_USER+message)

internal void OS_Window_Storage__Init(os_window_storage* WindowStorage, arena* Arena, u32 WindowCapacity) {
    uptr AllocationSize = (sizeof(ak_slot64)+sizeof(uint32_t)+sizeof(os_window))*WindowCapacity;
    ak_slot64* SlotPtr = (ak_slot64*)Arena_Push(Arena, AllocationSize);
    uint32_t* SlotIndicesPtr = (uint32_t*)(SlotPtr+WindowCapacity);
    AK_Async_Slot_Map64_Init_Raw(&WindowStorage->SlotMap, SlotIndicesPtr, SlotPtr, WindowCapacity);
    WindowStorage->Windows = (os_window*)(SlotIndicesPtr+WindowCapacity);
}

internal os_window_id OS_Window_Storage__Alloc(os_window_storage* WindowStorage) {
    ak_slot64 Slot = AK_Async_Slot_Map64_Alloc_Slot(&WindowStorage->SlotMap);
    if(Slot) {
        u32 Index = AK_Slot64_Index(Slot);
        Assert(Index < AK_Async_Slot_Map64_Capacity(&WindowStorage->SlotMap));
        os_window* Window = WindowStorage->Windows + Index;
        Zero_Struct(Window);
        return (os_window_id)Slot;
    }
    return 0;
}

internal os_window* OS_Window_Storage__Get(os_window_storage* WindowStorage, os_window_id WindowID) {
    ak_slot64 Slot = (ak_slot64)WindowID;
    if(AK_Async_Slot_Map64_Is_Allocated(&WindowStorage->SlotMap, Slot)) {
        uint32_t Index = AK_Slot64_Index(Slot);
        Assert(Index < AK_Async_Slot_Map64_Capacity(&WindowStorage->SlotMap));
        os_window* Window = WindowStorage->Windows + Index;
        return Window;
    }
    return NULL;
}

internal void OS_Window_Storage__Free(os_window_storage* WindowStorage, os_window_id WindowID) {
    ak_slot64 Slot = (ak_slot64)WindowID;
    AK_Async_Slot_Map64_Free_Slot(&WindowStorage->SlotMap, Slot);
}

internal void OS_Window_Title__Alloc(os_window_title* WindowTitle, string TitleStr) {
    char* Data;
    if(TitleStr.Size <= (sizeof(WindowTitle->TitleData)-1)) {
        Data = WindowTitle->TitleData;
        WindowTitle->Flags &= ~OS_WINDOW_TITLE_HEAP_ALLOCATED_BIT;
    } else {
        char* SlowData = (char*)Allocator_Allocate_Memory(Core_Get_Base_Allocator(), TitleStr.Size+1);
        Assert(SlowData);

        Memory_Copy(WindowTitle->TitleData, &SlowData, sizeof(char*));
        Data = SlowData;
        WindowTitle->Flags &= OS_WINDOW_TITLE_HEAP_ALLOCATED_BIT;
    }
    Memory_Copy(Data, TitleStr.Str, TitleStr.Size);
    Data[TitleStr.Size] = 0;
    WindowTitle->Size = TitleStr.Size;
}

#define AK_Window_Title__Get_Ptr(user_data) (void*)(*(uptr*)(user_data))
internal void OS_Window_Title__Free(os_window_title* WindowTitle) {
    if(WindowTitle->Flags & OS_WINDOW_TITLE_HEAP_ALLOCATED_BIT) {
        void* Ptr = AK_Window_Title__Get_Ptr(WindowTitle->TitleData);
        Allocator_Free_Memory(Core_Get_Base_Allocator(), Ptr);
        WindowTitle->Flags &= ~OS_WINDOW_TITLE_HEAP_ALLOCATED_BIT;
    }
}

internal string OS_Window_Title__Get(os_window_title* WindowTitle) {
    const char* Str = (const char*)((WindowTitle->Flags & OS_WINDOW_TITLE_HEAP_ALLOCATED_BIT) ? 
                                    AK_Window_Title__Get_Ptr(WindowTitle->TitleData) : 
                                    WindowTitle->TitleData); 
    AK_Window_Title__Get_Ptr(WindowTitle->TitleData);
    return string(Str, WindowTitle->Size);
}

internal LRESULT Win32__OS_Base_Window_Proc(HWND BaseWindow, UINT Message, WPARAM WParam, LPARAM LParam) {
        os* OS = OS_Get();
    Assert(OS);

    os_window_storage* WindowStorage = &OS->WindowStorage;

    LRESULT Result = 0;
    switch(Message) {
        case Win32__OS_Get_Message(WIN32_CREATE_WINDOW_MSG): {
            //todo: error handling
            ak_slot64 Slot = (os_window_id)AK_Slot64(WParam, LParam);
            if(AK_Async_Slot_Map64_Is_Allocated(&WindowStorage->SlotMap, Slot)) {
                u32 Index = AK_Slot64_Index(Slot);
                Assert(Index < AK_Async_Slot_Map64_Capacity(&WindowStorage->SlotMap));
                os_window* Window = WindowStorage->Windows + Index;
                Assert(!Window->Window);
                string Title = OS_Window_Title__Get(&Window->Title);
                Window->Window = Win32_Create_Window(&OS->MainWindowClass, (LONG)Window->Width, (LONG)Window->Height, Title, true, Window);
                gdi_window_data WindowData = {
                    .Win32 = {
                        .Window = Window->Window,
                        .Instance = GetModuleHandleW(NULL)
                    }
                };
                
                scratch Scratch = Scratch_Get();
                array<gdi_format> WindowFormats = GDI_Context_Supported_Window_Formats(OS->GDIContext, WindowData, Scratch.Arena);
                Window->Format = GDI_FORMAT_NONE;
                gdi_format TargetFormat = Window->TargetFormat;

                for(gdi_format Format : WindowFormats) {
                    if(Format == TargetFormat) {
                        Window->Format = Format;
                        break;
                    }
                }

                if(Window->Format == GDI_FORMAT_NONE) {
                    Window->Format = WindowFormats[0];
                }

                Window->Swapchain = GDI_Context_Create_Swapchain(OS->GDIContext, {
                    .WindowData = {
                        .Win32 = {
                            .Window = Window->Window,
                            .Instance = GetModuleHandleW(NULL)
                        }
                    },
                    .TargetFormat = Window->Format,
                    .UsageFlags = Window->UsageFlags
                });


                AK_Event_Signal(&Window->CreationEvent);
            }
        } break;

        case Win32__OS_Get_Message(WIN32_DELETE_WINDOW_MSG): {
            //todo: error handling
            ak_slot64 Slot = (os_window_id)AK_Slot64(WParam, LParam);
            os_window_id WindowID = (os_window_id)Slot;

            if(AK_Async_Slot_Map64_Is_Allocated(&WindowStorage->SlotMap, Slot)) {
                u32 Index = AK_Slot64_Index(Slot);
                Assert(Index < AK_Async_Slot_Map64_Capacity(&WindowStorage->SlotMap));
                os_window* Window = WindowStorage->Windows + Index;
                Assert(Window->Window && Window->Swapchain);
                GDI_Context_Delete_Swapchain(OS->GDIContext, Window->Swapchain);
                DestroyWindow(Window->Window);
                OS_Window_Title__Free(&Window->Title);
                OS_Window_Storage__Free(WindowStorage, WindowID);
            }
        } break;

        default: {
            Result = DefWindowProcW(BaseWindow, Message, WParam, LParam);
        } break;
    }
    return Result;
}

internal LRESULT Win32__OS_Window_Proc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
    os* OS = OS_Get();
    Assert(OS);

    LRESULT Result = 0;
    switch(Message) {
        case WM_CREATE: {
            CREATESTRUCT* CreateStruct = (CREATESTRUCT*)LParam;
            SetWindowLongPtrW(Window, GWLP_USERDATA, (LONG_PTR)CreateStruct->lpCreateParams);
        } break;

        case WM_SIZE: {
            os_window* OSWindow = (os_window*)GetWindowLongPtr(Window, GWLP_USERDATA);
            if(OSWindow) {
                os_event_window_resize* Event = (os_event_window_resize*)OS_Event_Stream_Allocate_Event(OS->EventStream, OS_EVENT_TYPE_WINDOW_RESIZE);
                Event->WindowID = OSWindow->ID;
                Event->Width = LOWORD(LParam);
                Event->Height = HIWORD(LParam);
            }
        } break;

        case WM_CLOSE: {
            os_window* OSWindow = (os_window*)GetWindowLongPtrW(Window, GWLP_USERDATA);
            if(OSWindow) {
                os_event* Event = OS_Event_Stream_Allocate_Event(OS->EventStream, OS_EVENT_TYPE_WINDOW_CLOSED);
                Event->WindowID = OSWindow->ID;
            }
        } break;

        default: {
            Result = DefWindowProcW(Window, Message, WParam, LParam);
        } break;
    }
    return Result;
}

internal AK_THREAD_CALLBACK_DEFINE(Win32__OS_Thread_Callback) {
    os* OS = OS_Get();

    WNDCLASSEXW BaseWindowClass = {
        .cbSize = sizeof(WNDCLASSEXW),
        .lpfnWndProc = Win32__OS_Base_Window_Proc,
        .hInstance = GetModuleHandleW(NULL),
        .lpszClassName = L"AK_Engine Base Window Class"        
    };

    if(!RegisterClassExW(&BaseWindowClass)) {
        //todo: logging
        return -1;
    }

    OS->BaseWindow = Win32_Create_Window(&BaseWindowClass, 0, 0, String_Lit(""), false, NULL);

    OS->MainWindowClass = {
        .cbSize = sizeof(WNDCLASSEXW),
        .lpfnWndProc = Win32__OS_Window_Proc,
        .hInstance = GetModuleHandleW(NULL),
        .lpszClassName = L"AK_Engine Window Class"
    };

    if(!RegisterClassExW(&OS->MainWindowClass)) {
        //todo: logging
        return -1;
    }

    AK_Atomic_Store_U32_Relaxed(&OS->IsRunning, true);
    AK_Auto_Reset_Event_Signal(&OS->ThreadInitEvent);

    OS->EventStream = OS_Event_Manager_Allocate_Stream(&OS->EventManager);

    for(;;) {
        if(!AK_Atomic_Load_U32_Relaxed(&OS->IsRunning)) {
            PostQuitMessage(0);
        }

        //If there are no messages. Use GetMessageW to block the thread until the
        //next message is available
        MSG Message;
        if(!PeekMessageW(&Message, NULL, 0, 0, PM_REMOVE)) {
            GetMessageW(&Message, NULL, 0, 0);

            OS_Event_Manager_Push_Back_Stream(&OS->EventManager, OS->EventStream);
            OS->EventStream = OS_Event_Manager_Allocate_Stream(&OS->EventManager);
        }
        
        switch(Message.message) {
            case WM_QUIT: {
                return 0;
            } break; 

            default: {
                TranslateMessage(&Message);
                DispatchMessageW(&Message);
            } break;
        }
    }
}

void OS_Message_Box(string Message, string Title) {
    Win32_Message_Box(Message, Title);
}

os_window_id OS_Create_Window(const os_window_create_info& CreateInfo) {
    os* OS = OS_Get();
    if(OS) {
        os_window_id WindowID = OS_Window_Storage__Alloc(&OS->WindowStorage);
        if(WindowID) {
            os_window* Window = OS_Window_Storage__Get(&OS->WindowStorage, WindowID);
            OS_Window_Title__Alloc(&Window->Title, CreateInfo.Title);
            Window->Width        = CreateInfo.Width;
            Window->Height       = CreateInfo.Height;
            Window->Flags        = CreateInfo.Flags;
            Window->TargetFormat = CreateInfo.TargetFormat;
            Window->UsageFlags   = CreateInfo.UsageFlags;
            Window->ID = WindowID;
            AK_Event_Create(&Window->CreationEvent);

            WPARAM WParam = AK_Slot64_Index((ak_slot64)WindowID);
            LPARAM LParam = AK_Slot64_Key((ak_slot64)WindowID);
            BOOL Status = SendNotifyMessageW(OS->BaseWindow, Win32__OS_Get_Message(WIN32_CREATE_WINDOW_MSG), WParam, LParam);
            Assert(Status);
        }

        return WindowID;
    }
    return 0;
}

void OS_Delete_Window(os_window_id WindowID) {
    os* OS = OS_Get();
    if(OS) {
        os_window_storage* WindowStorage = &OS->WindowStorage;

        ak_slot64 Slot = (ak_slot64)WindowID;
        if(AK_Async_Slot_Map64_Is_Allocated(&WindowStorage->SlotMap, Slot)) {
            u32 Index = AK_Slot64_Index(Slot);
            WPARAM WParam = Index;
            LPARAM LParam = AK_Slot64_Key(Slot);
            Assert(Index < AK_Async_Slot_Map64_Capacity(&WindowStorage->SlotMap));
            os_window* Window = WindowStorage->Windows + Index;
            AK_Event_Wait(&Window->CreationEvent);
            AK_Event_Delete(&Window->CreationEvent);
            SendNotifyMessageW(OS->BaseWindow, Win32__OS_Get_Message(WIN32_DELETE_WINDOW_MSG), WParam, LParam);
        }
    }
}

gdi_swapchain OS_Window_Get_Swapchain(os_window_id WindowID) {
    os* OS = OS_Get();
    if(OS) {
        os_window_storage* WindowStorage = &OS->WindowStorage;
        ak_slot64 Slot = (ak_slot64)WindowID;
        if(AK_Async_Slot_Map64_Is_Allocated(&WindowStorage->SlotMap, Slot)) {
            os_window* Window = WindowStorage->Windows + AK_Slot64_Index(Slot);
            AK_Event_Wait(&Window->CreationEvent);
            return Window->Swapchain;
        }
    }

    return 0;
}

const os_event* OS_Next_Event() {
    os* OS = OS_Get();
    if(!OS) return NULL;
    os_event_queue_iter* Iter = OS_Event_Manager_Get_Iter(&OS->EventManager);
    return OS_Event_Queue_Iter_Next(Iter);
}

os* OS_Create(gdi_context* GDIContext, const os_create_info& CreateInfo) {
    Assert(!OS_Get());
    arena* Arena = Arena_Create(Core_Get_Base_Allocator());
    Arena_Track(Arena, String_Lit("OS Arena"));

    os* OS = Arena_Push_Struct(Arena, os);
    OS->Arena = Arena;
    OS->GDIContext = GDIContext;
    OS_Set(OS);

    OS_Window_Storage__Init(&OS->WindowStorage, OS->Arena, CreateInfo.MaxWindowCount);
    OS_Event_Manager_Create(&OS->EventManager, 1);

    AK_Auto_Reset_Event_Create(&OS->ThreadInitEvent, 0);
    if(!AK_Thread_Create(&OS->Thread, Win32__OS_Thread_Callback, nullptr)) {
        OS_Delete();
        return NULL;
    }

    //For for the os thread to initialize so we can start calling api functions
    AK_Auto_Reset_Event_Wait(&OS->ThreadInitEvent);
    if(!AK_Atomic_Load_U32_Relaxed(&OS->IsRunning)) {
        OS_Delete();
        return NULL;
    }

    return OS;
}

void OS_Delete() {
    os* OS = OS_Get();
    if(OS) {
        AK_Atomic_Store_U32_Relaxed(&OS->IsRunning, false);
        AK_Thread_Delete(&OS->Thread);
        Arena_Delete(OS->Arena);
        OS_Set(nullptr);
    }
}

global os* G_OS;
os* OS_Get() {
    return G_OS;
}

void OS_Set(os* OS) {
    G_OS = OS;
}

#include <os/os_event.cpp>
#include <os/win32/win32_shared.cpp>