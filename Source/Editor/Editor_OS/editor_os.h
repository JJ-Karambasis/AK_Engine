#ifndef EDITOR_OS_H
#define EDITOR_OS_H

#include <Runtime_OS/runtime_os.h>

#include "Public/os_window.h"
#include "Public/os_event.h"
#include "Public/os_library.h"

typedef struct editor_os editor_os;

editor_os* Editor_OS_Init();
void       Editor_OS_Shutdown();

#ifdef OS_WIN32
#include "Win32/win32_editor_os.h"
#endif

#endif