static os_keycode Win32__VK_To_OS(DWORD KeyCode);
static uint32_t Win32__Get_Modifiers()
{
    uint32_t Result = 0;
    if(GetAsyncKeyState(VK_MENU) & 0x8000)
        Result |= OS_MODIFIER_ALT;
    
    if(GetAsyncKeyState(VK_SHIFT) & 0x8000)
        Result |= OS_MODIFIER_SHIFT;
    
    if(GetAsyncKeyState(VK_CONTROL) & 0x8000)
        Result |= OS_MODIFIER_CONTROL;
    
    return Result;
}

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
        
        case WM_CHAR:
        case WM_SYSCHAR:
        {
            win32_window* OSWindow = (win32_window*)GetWindowLongPtr(Window, GWLP_USERDATA);
            os_event* Event = Win32__Enqueue_Event(OS_EVENT_TYPE_TEXT_INPUT);
            Event->Window = (os_window*)OSWindow;
            Event->TextInputUTF32 = UTF16_Read((const uint16_t*)&WParam);
        } break;
        
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            if((LParam & (1 << 30)) == 0)
            {
                win32_window* OSWindow = (win32_window*)GetWindowLongPtr(Window, GWLP_USERDATA);
                os_event* Event = Win32__Enqueue_Event(OS_EVENT_TYPE_KEY_PRESSED);
                Event->Window = (os_window*)OSWindow;
                Event->KeyPressed.Keycode = Win32__VK_To_OS((DWORD)WParam);
                Event->KeyPressed.Modifiers = Win32__Get_Modifiers();
            }
        } break;
        
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            win32_window* OSWindow = (win32_window*)GetWindowLongPtr(Window, GWLP_USERDATA);
            os_event* Event = Win32__Enqueue_Event(OS_EVENT_TYPE_KEY_RELEASED);
            Event->Window = (os_window*)OSWindow;
            Event->KeyReleased.Keycode = Win32__VK_To_OS((DWORD)WParam);
            Event->KeyReleased.Modifiers = Win32__Get_Modifiers();
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

static os_keycode Win32__VK_To_OS(DWORD KeyCode)
{
    os_keycode Result = OS_KEYCODE_UNKNOWN;
    switch(KeyCode)
    {
        case 'A':
        Result = OS_KEYCODE_A;
        break;
        
        case 'B':
        Result = OS_KEYCODE_B;
        break;
        
        case 'C':
        Result = OS_KEYCODE_C;
        break;
        
        case 'D':
        Result = OS_KEYCODE_D;
        break;
        
        case 'E':
        Result = OS_KEYCODE_E;
        break;
        
        case 'F':
        Result = OS_KEYCODE_F;
        break;
        
        case 'G':
        Result = OS_KEYCODE_G;
        break;
        
        case 'H':
        Result = OS_KEYCODE_H;
        break;
        
        case 'I':
        Result = OS_KEYCODE_I;
        break;
        
        case 'J':
        Result = OS_KEYCODE_J;
        break;
        
        case 'K':
        Result = OS_KEYCODE_K;
        break;
        
        case 'L':
        Result = OS_KEYCODE_L;
        break;
        
        case 'M':
        Result = OS_KEYCODE_M;
        break;
        
        case 'N':
        Result = OS_KEYCODE_N;
        break;
        
        case 'O':
        Result = OS_KEYCODE_O;
        break;
        
        case 'P':
        Result = OS_KEYCODE_P;
        break;
        
        case 'Q':
        Result = OS_KEYCODE_Q;
        break;
        
        case 'R':
        Result = OS_KEYCODE_R;
        break;
        
        case 'S':
        Result = OS_KEYCODE_S;
        break;
        
        case 'T':
        Result = OS_KEYCODE_T;
        break;
        
        case 'U':
        Result = OS_KEYCODE_U;
        break;
        
        case 'V':
        Result = OS_KEYCODE_V;
        break;
        
        case 'W':
        Result = OS_KEYCODE_W;
        break;
        
        case 'X':
        Result = OS_KEYCODE_X;
        break;
        
        case 'Y':
        Result = OS_KEYCODE_Y;
        break;
        
        case 'Z':
        Result = OS_KEYCODE_Z;
        break;
        
        case '0':
        Result = OS_KEYCODE_0;
        break;
        
        case '1':
        Result = OS_KEYCODE_1;
        break;
        
        case '2':
        Result = OS_KEYCODE_2;
        break;
        
        case '3':
        Result = OS_KEYCODE_3;
        break;
        
        case '4':
        Result = OS_KEYCODE_4;
        break;
        
        case '5':
        Result = OS_KEYCODE_5;
        break;
        
        case '6':
        Result = OS_KEYCODE_6;
        break;
        
        case '7':
        Result = OS_KEYCODE_7;
        break;
        
        case '8':
        Result = OS_KEYCODE_8;
        break;
        
        case '9':
        Result = OS_KEYCODE_9;
        break;
        
        case VK_SPACE:
        Result = OS_KEYCODE_SPACE;
        break;
        
        case VK_OEM_3:
        Result = OS_KEYCODE_TICK;
        break;
        
        case VK_OEM_MINUS:
        Result = OS_KEYCODE_MINUS;
        break;
        
        case VK_OEM_PLUS:
        Result = OS_KEYCODE_EQUAL;
        break;
        
        case VK_OEM_4:
        Result = OS_KEYCODE_LEFTBRACKET;
        break;
        
        case VK_OEM_6:
        Result = OS_KEYCODE_RIGHTBRACKET;
        break;
        
        case VK_OEM_1:
        Result = OS_KEYCODE_SEMICOLON;
        break;
        
        case VK_OEM_7:
        Result = OS_KEYCODE_QUOTE;
        break;
        
        case VK_OEM_COMMA:
        Result = OS_KEYCODE_COMMA;
        break;
        
        case VK_OEM_PERIOD:
        Result = OS_KEYCODE_PERIOD;
        break;
        
        case VK_OEM_2:
        Result = OS_KEYCODE_FORWARDSLASH;
        break;
        
        case VK_OEM_5:
        Result = OS_KEYCODE_BACKSLASH;
        break;
        
        case VK_TAB:
        Result = OS_KEYCODE_TAB;
        break;
        
        case VK_ESCAPE:
        Result = OS_KEYCODE_ESCAPE;
        break;
        
        case VK_PAUSE:
        Result = OS_KEYCODE_PAUSE;
        break;
        
        case VK_UP:
        Result = OS_KEYCODE_UP;
        break;
        
        case VK_DOWN:
        Result = OS_KEYCODE_DOWN;
        break;
        
        case VK_LEFT:
        Result = OS_KEYCODE_LEFT;
        break;
        
        case VK_RIGHT:
        Result = OS_KEYCODE_RIGHT;
        break;
        
        case VK_BACK:
        Result = OS_KEYCODE_BACKSPACE;
        break;
        
        case VK_RETURN:
        Result = OS_KEYCODE_RETURN;
        break;
        
        case VK_DELETE:
        Result = OS_KEYCODE_DELETE;
        break;
        
        case VK_INSERT:
        Result = OS_KEYCODE_INSERT;
        break;
        
        case VK_HOME:
        Result = OS_KEYCODE_HOME;
        break;
        
        case VK_END:
        Result = OS_KEYCODE_END;
        break;
        
        case VK_PRIOR:
        Result = OS_KEYCODE_PAGEUP;
        break;
        
        case VK_NEXT:
        Result = OS_KEYCODE_PAGEDOWN;
        break;
        
        case VK_CAPITAL:
        Result = OS_KEYCODE_CAPSLOCK;
        break;
        
        case VK_NUMLOCK:
        Result = OS_KEYCODE_NUMLOCK;
        break;
        
        case VK_SCROLL:
        Result = OS_KEYCODE_SCROLLLOCK;
        break;
        
        case VK_SHIFT:
        Result = OS_KEYCODE_SHIFT;
        break;
        
        case VK_CONTROL:
        Result = OS_KEYCODE_CONTROL;
        break;
        
        case VK_MENU:
        Result = OS_KEYCODE_ALT;
        break;
        
        case VK_LSHIFT:
        Result = OS_KEYCODE_LSHIFT;
        break;
        
        case VK_LCONTROL:
        Result = OS_KEYCODE_LCONTROL;
        break;
        
        case VK_LMENU:
        Result = OS_KEYCODE_LALT;
        break;
        
        case VK_LWIN:
        Result = OS_KEYCODE_LSUPER;
        break;
        
        case VK_RSHIFT:
        Result = OS_KEYCODE_RSHIFT;
        break;
        
        case VK_RCONTROL:
        Result = OS_KEYCODE_RCONTROL;
        break;
        
        case VK_RMENU:
        Result = OS_KEYCODE_RALT;
        break;
        
        case VK_RWIN:
        Result = OS_KEYCODE_RSUPER;
        break;
        
        case VK_SNAPSHOT:
        Result = OS_KEYCODE_PRINTSCREEN;
        break;
        
        case VK_APPS:
        Result = OS_KEYCODE_MENU;
        break;
        
        //NOTE(EVERYONE): We skip OS_KEYCODE_COMMAND
        
        case VK_F1:
        Result = OS_KEYCODE_F1;
        break;
        
        case VK_F2:
        Result = OS_KEYCODE_F2;
        break;
        
        case VK_F3:
        Result = OS_KEYCODE_F3;
        break;
        
        case VK_F4:
        Result = OS_KEYCODE_F4;
        break;
        
        case VK_F5:
        Result = OS_KEYCODE_F5;
        break;
        
        case VK_F6:
        Result = OS_KEYCODE_F6;
        break;
        
        case VK_F7:
        Result = OS_KEYCODE_F7;
        break;
        
        case VK_F8:
        Result = OS_KEYCODE_F8;
        break;
        
        case VK_F9:
        Result = OS_KEYCODE_F9;
        break;
        
        case VK_F10:
        Result = OS_KEYCODE_F10;
        break;
        
        case VK_F11:
        Result = OS_KEYCODE_F11;
        break;
        
        case VK_F12:
        Result = OS_KEYCODE_F12;
        break;
        
        case VK_F13:
        Result = OS_KEYCODE_F13;
        break;
        
        case VK_F14:
        Result = OS_KEYCODE_F14;
        break;
        
        case VK_F15:
        Result = OS_KEYCODE_F15;
        break;
        
        case VK_F16:
        Result = OS_KEYCODE_F16;
        break;
        
        case VK_F17:
        Result = OS_KEYCODE_F17;
        break;
        
        case VK_F18:
        Result = OS_KEYCODE_F18;
        break;
        
        case VK_F19:
        Result = OS_KEYCODE_F19;
        break;
        
        case VK_F20:
        Result = OS_KEYCODE_F20;
        break;
        
        case VK_F21:
        Result = OS_KEYCODE_F21;
        break;
        
        case VK_F22:
        Result = OS_KEYCODE_F22;
        break;
        
        case VK_F23:
        Result = OS_KEYCODE_F23;
        break;
        
        case VK_F24:
        Result = OS_KEYCODE_F24;
        break;
        
        case VK_NUMPAD0:
        Result = OS_KEYCODE_NUMPAD0;
        break;
        
        case VK_NUMPAD1:
        Result = OS_KEYCODE_NUMPAD1;
        break;
        
        case VK_NUMPAD2:
        Result = OS_KEYCODE_NUMPAD2;
        break;
        
        case VK_NUMPAD3:
        Result = OS_KEYCODE_NUMPAD3;
        break;
        
        case VK_NUMPAD4:
        Result = OS_KEYCODE_NUMPAD4;
        break;
        
        case VK_NUMPAD5:
        Result = OS_KEYCODE_NUMPAD5;
        break;
        
        case VK_NUMPAD6:
        Result = OS_KEYCODE_NUMPAD6;
        break;
        
        case VK_NUMPAD7:
        Result = OS_KEYCODE_NUMPAD7;
        break;
        
        case VK_NUMPAD8:
        Result = OS_KEYCODE_NUMPAD8;
        break;
        
        case VK_NUMPAD9:
        Result = OS_KEYCODE_NUMPAD9;
        break;
        
        case VK_MULTIPLY:
        Result = OS_KEYCODE_NUMPADSTAR;
        break;
        
        case VK_ADD:
        Result = OS_KEYCODE_NUMPADPLUS;
        break;
        
        case VK_SUBTRACT:
        Result = OS_KEYCODE_NUMPADMINUS;
        break;
        
        case VK_DECIMAL:
        Result = OS_KEYCODE_NUMPADDOT;
        break;
        
        case VK_DIVIDE:
        Result = OS_KEYCODE_NUMPADSLASH;
        break;
    }
    
    return Result;
}