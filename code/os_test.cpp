#include <core/core.h>
#include <math/math.h>
#include <os.h>

#include <stdio.h>

bool Application_Main() {
    OS_Process_Exit(OS_Exec_Process(String_Lit("echo"), String_Lit("hello")));
    OS_Process_Exit(OS_Exec_Process(String_Lit("echo"), String_Lit("world")));

    OS_Open_Window({
        .Flags = OS_WINDOW_FLAG_MAIN_BIT, 
        .Title = String_Lit("Test"),
        .Monitor = OS_Get_Primary_Monitor(), 
        .Pos = point2i(200, 200),
        .Size = dim2i(1920, 1080)
    });

    span<os_monitor_id> Monitors = OS_Get_Monitors();
    os_monitor_id PrimaryMonitor = OS_Get_Primary_Monitor();
    for(os_monitor_id Monitor : Monitors) {
        const os_monitor_info* MonitorInfo = OS_Get_Monitor_Info(Monitor);
        printf("Monitor %.*s has rectangle: {(%d, %d), (%d, %d)}.", (int)MonitorInfo->Name.Size, MonitorInfo->Name.Str, 
        MonitorInfo->Rect.P1.x, MonitorInfo->Rect.P1.y, MonitorInfo->Rect.P2.x, MonitorInfo->Rect.P2.y);

        if(Monitor == PrimaryMonitor) {
            printf(" Monitor is primary!\n");
        } else {
            printf("\n");
        }
    }

    string Executable = OS_Get_Executable_Path();
    printf("%.*s\n", (int)Executable.Size, Executable.Str);

    point2i LastMousePosition = {};
    int FrameCount = 0;
    while(OS_Window_Is_Open(OS_Get_Main_Window())) {
        while(const os_event* Event = OS_Next_Event()) {
            switch(Event->Type) {
                case OS_EVENT_TYPE_WINDOW_CLOSED: {
                    OS_Close_Window(Event->Window);
                } break;

                case OS_EVENT_TYPE_MOUSE_DELTA: {
                    const os_mouse_delta_event* DeltaEvent = (const os_mouse_delta_event*)Event;
                    //printf("Mouse delta %d %d\n", DeltaEvent->Delta.x, DeltaEvent->Delta.y);
                } break;

                case OS_EVENT_TYPE_MOUSE_SCROLL: {
                    const os_mouse_scroll_event* ScrollEvent = (const os_mouse_scroll_event*)Event;
                    //printf("Mouse scroll %f\n", ScrollEvent->Scroll);
                } break;
            }
        }

        bool Down = false;
        for(u32 i = 0; i < OS_KEYBOARD_KEY_COUNT; i++) {
            if(OS_Keyboard_Get_Key_State(i)) {
                //printf("OS Key down: %d %d\n", i, FrameCount);
                Down = true;
            }
        }

        for(u32 i = 0; i < OS_MOUSE_KEY_COUNT; i++) {
            if(OS_Mouse_Get_Key_State(i)) {
                //printf("OS Mouse down: %d %d\n", i, FrameCount);
                Down = true;
            }
        }

        if(Down == false) {
            FrameCount = 0;
        } else {
            FrameCount++;
        }

        point2i MousePosition = OS_Mouse_Get_Position();
        if(MousePosition != LastMousePosition) {
            //printf("Mouse position: (%d, %d)\n", MousePosition.x, MousePosition.y);
            LastMousePosition = MousePosition;
        }

        point2i WindowPos = OS_Window_Get_Pos(OS_Get_Main_Window());
        rect2i WindowRect = rect2i(WindowPos, WindowPos+OS_Window_Get_Size(OS_Get_Main_Window()));
        printf("Window rect: [(%d, %d)]\n", WindowPos.x, WindowPos.y);

        if(OS_Keyboard_Get_Key_State(OS_KEYBOARD_KEY_ESCAPE)) {
            return true;
        }
    }

    return true;
}
