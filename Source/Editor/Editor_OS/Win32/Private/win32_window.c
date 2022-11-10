os_window* OS_Create_Window(uint32_t Width, uint32_t Height, str8 WindowName, uint64_t WindowFlags)
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    if(!OS) return NULL;
    win32_editor_os* EditorOS = (win32_editor_os*)OS->OS.EditorOS;
    
    DWORD ExStyle = 0;
    DWORD Style = WS_VISIBLE|WS_OVERLAPPEDWINDOW;
    
    if(WindowFlags & OS_WINDOW_FLAG_MAXIMIZE) Style |= WS_MAXIMIZE;
    RECT ClientRect;
    Zero_Struct(&ClientRect, RECT);
    ClientRect.right  = Width;
    ClientRect.bottom = Height;
    AdjustWindowRectEx(&ClientRect, Style, FALSE, ExStyle);
    
    str16 WindowNameW = UTF8_To_UTF16(Get_Base_Allocator(Core_Get_Thread_Context()->Scratch), WindowName);
    HWND Handle = CreateWindowExW(ExStyle, WIN32_WINDOW_CLASS, WindowNameW.Str, Style, 200, 200, ClientRect.right-ClientRect.left, ClientRect.bottom-ClientRect.top, NULL, NULL, GetModuleHandle(0), NULL);
    if(!Handle) return NULL;
    
    win32_window* Window = EditorOS->FreeWindows;
    if(!Window) Window = Arena_Push_Struct(OS->Arena, win32_window);
    else SLL_Pop_Front(EditorOS->FreeWindows);
    Zero_Struct(Window, win32_window);
    
    Window->Handle = Handle;
    SetWindowLongPtr(Window->Handle, GWLP_USERDATA, (LONG_PTR)Window);
    
    return (os_window*)Window;
}

void OS_Delete_Window(os_window* Window)
{
    runtime_os* OS = OS_Get();
    if(!OS) return;
    
    win32_window* OSWindow = (win32_window*)Window;
    
    DestroyWindow(OSWindow->Handle);
    
    win32_editor_os* EditorOS = (win32_editor_os*)OS->EditorOS;
    SLL_Push_Front(EditorOS->FreeWindows, OSWindow);
}