internal void Win32_Message_Box(string Message, string Title) {
    Assert(Message.Size < WIN32_MESSAGE_BOX_CAPACITY);
    Assert(Title.Size < WIN32_MESSAGE_BOX_CAPACITY);

    wchar_t MessageW[WIN32_MESSAGE_BOX_CAPACITY+1] = {};
    wchar_t TitleW[WIN32_MESSAGE_BOX_CAPACITY+1] = {0};

    MultiByteToWideChar(CP_UTF8, 0, Message.Str, (int)Message.Size, MessageW, WIN32_MESSAGE_BOX_CAPACITY+1);
    MultiByteToWideChar(CP_UTF8, 0, Title.Str, (int)Title.Size, TitleW, WIN32_MESSAGE_BOX_CAPACITY+1);

    MessageBoxW(NULL, MessageW, TitleW, MB_OK);
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

internal bool Win32_File_Exists(const wchar_t* Path) {
    DWORD Attrib = GetFileAttributesW(Path);
    return (Attrib != INVALID_FILE_ATTRIBUTES && !(Attrib & FILE_ATTRIBUTE_DIRECTORY));
}

internal bool Win32_Directory_Exists(const wchar_t* Path) {
    DWORD Attrib = GetFileAttributesW(Path);
    return (Attrib != INVALID_FILE_ATTRIBUTES && (Attrib & FILE_ATTRIBUTE_DIRECTORY));
}

internal string Win32_Get_Exe_File_Path(allocator* Allocator) {
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