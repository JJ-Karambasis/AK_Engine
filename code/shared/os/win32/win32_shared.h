#ifndef WIN32_SHARED_H
#define WIN32_SHARED_H

#define WIN32_MESSAGE_BOX_CAPACITY 1024

internal void   Win32_Message_Box(string Message, string Title);
internal HWND   Win32_Create_Window(WNDCLASSEXW* WindowClass, DWORD Style, LONG XOffset, LONG YOffset, LONG Width, LONG Height, string Title, void* UserData);
internal bool   Win32_File_Exists(const wchar_t* Path);
internal bool   Win32_Directory_Exists(const wchar_t* Path);
internal string Win32_Get_Exe_File_Path(allocator* Allocator);

#endif