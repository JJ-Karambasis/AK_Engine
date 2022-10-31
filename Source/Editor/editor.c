#include "editor.h"

int main(int ArgumentCount, char** Arguments)
{
    if(!Core_Init())
    {
        //TODO(JJ): Diagnostic and error logging
        return 1;
    }
    
    os_window* MainWindow = OS_Create_Window(0, 0, Str8_Lit("AK Engine"), OS_WINDOW_FLAG_MAXIMIZE);
    bool32_t IsLooping = true;
    while(IsLooping)
    {
        OS_Poll_Events();
        
        os_event* Event = OS_Get_Next_Event();
        while(Event)
        {
            switch(Event->Type)
            {
                case OS_EVENT_TYPE_WINDOW_CLOSED:
                {
                    if(Event->Window == MainWindow)
                    {
                        IsLooping = false;
                    }
                    OS_Delete_Window(Event->Window);
                } break;
            }
            
            Event = OS_Get_Next_Event();
        }
    }
    
    Core_Shutdown();
    return 0;
}

#include <Core/core.c>