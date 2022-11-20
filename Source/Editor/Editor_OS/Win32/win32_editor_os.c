LRESULT Win32_Window_Proc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);
editor_os* Editor_OS_Init()
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    
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

bool8_t OS_File_Exists(str8 File)
{
    str16 FileW = UTF8_To_UTF16(Get_Base_Allocator(Core_Get_Thread_Context()->Scratch), File);
    DWORD FileAttributes = GetFileAttributesW(FileW.Str);
    return FileAttributes != INVALID_FILE_ATTRIBUTES && !(FileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

bool8_t    OS_Directory_Exists(str8 Directory)
{
    str16 DirectoryW = UTF8_To_UTF16(Get_Base_Allocator(Core_Get_Thread_Context()->Scratch), Directory);
    DWORD FileAttributes = GetFileAttributesW(DirectoryW.Str);
    return FileAttributes != INVALID_FILE_ATTRIBUTES && (FileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

#include "Private/win32_window.c"
#include "Private/win32_event.c"
#include "Private/win32_library.c"
#include "Private/win32_file_enumerator.c"