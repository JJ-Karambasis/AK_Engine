#ifndef OS_EVENT_H
#define OS_EVENT_H

typedef enum event_type
{
    OS_EVENT_TYPE_NONE,
    OS_EVENT_TYPE_WINDOW_CLOSED,
    OS_EVENT_TYPE_WINDOW_RESIZED,
    OS_EVENT_TYPE_TEXT_INPUT
} event_type;

typedef struct os_event
{
    event_type Type;
    os_window* Window;
    
    union
    {
        uint32_t TextInputUTF32;
    };
} os_event;

void            OS_Poll_Events();
const os_event* OS_Get_Next_Event();

#endif