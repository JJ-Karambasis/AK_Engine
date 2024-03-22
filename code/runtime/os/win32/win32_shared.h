#ifndef WIN32_SHARED_H
#define WIN32_SHARED_H

#define WIN32_MESSAGE_BOX_CAPACITY 1024

void Win32_Message_Box(string Message, string Title);
HWND Win32_Create_Window(WNDCLASSEXW* WindowClass, LONG Width, LONG Height, string Title, bool IsVisible, void* UserData);

#endif