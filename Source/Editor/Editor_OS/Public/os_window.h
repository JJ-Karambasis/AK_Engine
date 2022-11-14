#ifndef OS_WINDOW_H
#define OS_WINDOW_H

typedef struct os_window 
{
    void* UserData;
    v2i   Dim;
} os_window;

enum
{
    OS_WINDOW_FLAG_NONE = 0,
    OS_WINDOW_FLAG_MAXIMIZE = 1
};

os_window* OS_Create_Window(uint32_t Width, uint32_t Height, str8 WindowName, uint64_t WindowFlags, void* UserData);
void       OS_Delete_Window(os_window* Window);

#endif