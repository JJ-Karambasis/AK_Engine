#define WIN32_WINDOW_CLASS L"AK_Engine_Win32_Window_Class"

LRESULT Win32_Window_Proc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);
editor_os* Editor_OS_Init()
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    
    WNDCLASSEXW WindowClass;
    Zero_Struct(&WindowClass, WNDCLASSEX);
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.style = CS_OWNDC;
    WindowClass.lpfnWndProc = Win32_Window_Proc;
    WindowClass.hInstance = GetModuleHandle(0);
    WindowClass.lpszClassName = WIN32_WINDOW_CLASS;
    WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    if(!RegisterClassExW(&WindowClass)) return NULL;
    
    win32_editor_os* EditorOS = Arena_Push_Struct(OS->Arena, win32_editor_os);
    Zero_Struct(EditorOS, win32_editor_os);
    EditorOS->EventManager.Arena = Arena_Create(Get_Base_Allocator(OS->Arena), Kilo(128));
    return (editor_os*)EditorOS;
}

void Editor_OS_Shutdown()
{
    PostQuitMessage(0);
    OS_Poll_Events();
}

#include "Private/win32_window.c"
#include "Private/win32_event.c"