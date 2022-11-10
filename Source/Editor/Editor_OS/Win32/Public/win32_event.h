#ifndef WIN32_EVENT_H
#define WIN32_EVENT_H

typedef struct win32_event
{
    os_event     Event;
    struct win32_event* Next;
} win32_event;

typedef struct win32_event_list
{
    win32_event* First;
    win32_event* Last;
} win32_event_list;

typedef struct win32_event_manager
{
    arena*           Arena;
    win32_event_list EventQueue;
    win32_event_list PendingEvents;
    win32_event_list FreeEvents;
} win32_event_manager;

#endif