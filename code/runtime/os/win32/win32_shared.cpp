void Win32_Message_Box(string Message, string Title) {
    Assert(Message.Size < WIN32_MESSAGE_BOX_CAPACITY);
    Assert(Title.Size < WIN32_MESSAGE_BOX_CAPACITY);

    wchar_t MessageW[WIN32_MESSAGE_BOX_CAPACITY+1] = {};
    wchar_t TitleW[WIN32_MESSAGE_BOX_CAPACITY+1] = {0};

    MultiByteToWideChar(CP_UTF8, 0, Message.Str, (int)Message.Size, MessageW, WIN32_MESSAGE_BOX_CAPACITY+1);
    MultiByteToWideChar(CP_UTF8, 0, Title.Str, (int)Title.Size, TitleW, WIN32_MESSAGE_BOX_CAPACITY+1);

    MessageBoxW(NULL, MessageW, TitleW, MB_OK);
}

HWND Win32_Create_Window(WNDCLASSEXW* WindowClass, LONG Width, LONG Height, string Title, bool IsVisible, void* UserData) {
    DWORD ExStyle = 0;
    DWORD Style = WS_OVERLAPPEDWINDOW;
    if(IsVisible) {
        Style |= WS_VISIBLE;
    }

    RECT Rect = {0};
    Rect.right = Width;
    Rect.bottom = Height;

    AdjustWindowRectEx(&Rect, Style, FALSE, ExStyle);
    
    scratch Scratch = Scratch_Get();
    wstring TitleW(&Scratch, Title);
    HWND Result = CreateWindowExW(ExStyle, WindowClass->lpszClassName, TitleW.Str, Style, 
                                  CW_USEDEFAULT, CW_USEDEFAULT, 
                                  Rect.right-Rect.left, Rect.bottom-Rect.top,
                                  NULL, NULL, WindowClass->hInstance, UserData);
    return Result;
}