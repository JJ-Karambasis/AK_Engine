#ifndef WIN32_EDITOR_OS_H
#define WIN32_EDITOR_OS_H

#include "Public/win32_window.h"
#include "Public/win32_event.h"

typedef struct win32_editor_os
{
    win32_window*       FreeWindows;
    win32_event_manager EventManager;
} win32_editor_os;

#endif