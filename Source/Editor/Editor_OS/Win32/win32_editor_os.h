#ifndef WIN32_EDITOR_OS_H
#define WIN32_EDITOR_OS_H

#include <unknwn.h>

#define Com_Base_Decl(type) \
STDMETHOD(QueryInterface)(type*, REFIID, void**); \
STDMETHOD_(ULONG, AddRef)(type*); \
STDMETHOD_(ULONG, Release)(type*)

#include "Public/win32_dwrite_headers.h"
#include "Public/win32_window.h"
#include "Public/win32_event.h"
#include "Public/win32_file_enumerator.h"
#include "Public/win32_font_loader.h"

typedef struct win32_editor_os
{
    win32_window*       FreeWindows;
    win32_event_manager EventManager;
    HMODULE             DWriteLibrary;
} win32_editor_os;

#endif