os_window* OS_Create_Window(uint32_t Width, uint32_t Height, str8 WindowName, uint64_t WindowFlags, void* UserData)
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    if(!OS) return NULL;
    win32_editor_os* EditorOS = (win32_editor_os*)OS->OS.EditorOS;
    
    WNDCLASSEXW TempWindowClass;
    if(!GetClassInfoExW(GetModuleHandle(0), WIN32_WINDOW_CLASS, &TempWindowClass))
        Win32_Register_Window_Class(WIN32_WINDOW_CLASS, Win32_Window_Proc);
    
    DWORD ExStyle = 0;
    DWORD Style = WS_VISIBLE|WS_OVERLAPPEDWINDOW;
    
    if(WindowFlags & OS_WINDOW_FLAG_MAXIMIZE) Style |= WS_MAXIMIZE;
    HWND Handle = Win32_Create_Window(Width, Height, WindowName, WIN32_WINDOW_CLASS, Style, ExStyle);
    
    win32_window* Window = EditorOS->FreeWindows;
    if(!Window) Window = Arena_Push_Struct(OS->OS.Arena, win32_window);
    else SLL_Pop_Front(EditorOS->FreeWindows);
    Zero_Struct(Window, win32_window);
    
    Window->Handle = Handle;
    Window->Window.UserData = UserData;
    Window->Window.Dim = V2i(Width, Height);
    SetWindowLongPtr(Window->Handle, GWLP_USERDATA, (LONG_PTR)Window);
    
    return &Window->Window;
}

void OS_Delete_Window(os_window* Window)
{
    runtime_os* OS = OS_Get();
    if(!OS) return;
    
    win32_window* OSWindow = (win32_window*)Window;
    Win32_Destroy_Window(OSWindow->Handle);
    
    win32_editor_os* EditorOS = (win32_editor_os*)OS->EditorOS;
    SLL_Push_Front(EditorOS->FreeWindows, OSWindow);
}