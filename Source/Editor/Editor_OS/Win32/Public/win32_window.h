#ifndef WIN32_WINDOW_H
#define WIN32_WINDOW_H

typedef struct win32_window
{
    os_window            Window;
    HWND                 Handle;
    struct win32_window* Next;
} win32_window;

#endif