win32_event* Win32__Allocate_Event()
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    win32_editor_os* EditorOS = (win32_editor_os*)OS->OS.EditorOS;
    win32_event_manager* EventManager = &EditorOS->EventManager;
    
    win32_event* Result = EventManager->FreeEvents.First;
    if(!Result) Result = Arena_Push_Struct(EventManager->Arena, win32_event);
    else SLL_Pop_Front(EventManager->FreeEvents.First);
    Zero_Struct(Result, win32_event);
    
    return Result;
}

os_event* Win32__Enqueue_Event_Typeless()
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    win32_editor_os* EditorOS = (win32_editor_os*)OS->OS.EditorOS;
    win32_event_manager* EventManager = &EditorOS->EventManager;
    
    win32_event* Result = Win32__Allocate_Event();
    SLL_Push_Back(EventManager->EventQueue.First, EventManager->EventQueue.Last, Result);
    return &Result->Event;
}

os_event* Win32__Enqueue_Event(event_type Type)
{
    os_event* Result = Win32__Enqueue_Event_Typeless();
    Result->Type = Type;
    return Result;
}

LRESULT Win32_Window_Proc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_CLOSE:
        {
            win32_window* OSWindow = (win32_window*)GetWindowLongPtr(Window, GWLP_USERDATA);
            if(OSWindow)
            {
                os_event* Event = Win32__Enqueue_Event(OS_EVENT_TYPE_WINDOW_CLOSED);
                Event->Window = (os_window*)OSWindow;
            }
        } break;
        
        case WM_SIZE:
        {
            win32_window* OSWindow = (win32_window*)GetWindowLongPtr(Window, GWLP_USERDATA);
            if(OSWindow)
            {
                os_event* Event = Win32__Enqueue_Event(OS_EVENT_TYPE_WINDOW_RESIZED);
                Event->Window = (os_window*)OSWindow;
                Event->Window->Dim = V2i(WIN32_GET_X_LPARAM(LParam), WIN32_GET_Y_LPARAM(LParam));
            }
        } break;
        
        default:
        {
            Result = DefWindowProcW(Window, Message, WParam, LParam);
        } break;
    }
    return Result;
}

void OS_Poll_Events()
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    if(!OS) return;
    
    win32_editor_os* EditorOS = (win32_editor_os*)OS->OS.EditorOS;
    win32_event_manager* EventManager = &EditorOS->EventManager;
    
    MSG Message;
    while(PeekMessageW(&Message, NULL, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                //TODO(JJ): Add some logging
            } break;
            
            default:
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            } break;
        }
    }
    
    //NOTE(EVERYONE): At the end of each poll event we take all pending events (events pulled from OS_Get_Next_Event) and put
    //them into the free list since they are safe to release
    while(EventManager->PendingEvents.First)
    {
        win32_event* EventToTransfer = EventManager->PendingEvents.First;
        SLL_Pop_Front(EventManager->PendingEvents.First);
        SLL_Push_Front(EventManager->FreeEvents.First, EventToTransfer);
    }
}

const os_event* OS_Get_Next_Event()
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    if(!OS) return NULL;
    
    win32_editor_os* EditorOS = (win32_editor_os*)OS->OS.EditorOS;
    win32_event_manager* EventManager = &EditorOS->EventManager;
    
    if(EventManager->EventQueue.First)
    {
        win32_event* EventToTransfer = EventManager->EventQueue.First;
        SLL_Pop_Front(EventManager->EventQueue.First);
        SLL_Push_Front(EventManager->PendingEvents.First, EventToTransfer);
        return &EventToTransfer->Event;
    }
    
    return NULL;
}