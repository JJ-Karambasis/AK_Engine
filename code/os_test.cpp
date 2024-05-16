#include <core/core.h>
#include <math/math.h>
#include <os.h>

#include <stdio.h>

bool Application_Main() {
    OS_Process_Exit(OS_Exec_Process(String_Lit("echo"), String_Lit("hello")));
    OS_Process_Exit(OS_Exec_Process(String_Lit("echo"), String_Lit("world")));

    OS_Create_Window({
        .Flags = OS_WINDOW_FLAG_MAIN_BIT, 
        .Title = String_Lit("Test"),
        .Monitor = OS_Get_Primary_Monitor(), 
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

    point2i LastMousePosition = {};
    int FrameCount = 0;
    for(;;) {
        bool Down = false;
        for(u32 i = 0; i < OS_KEYBOARD_KEY_COUNT; i++) {
            if(OS_Keyboard_Get_Key_State(i)) {
                printf("OS Key down: %d %d\n", i, FrameCount);
                Down = true;
            }
        }

        for(u32 i = 0; i < OS_MOUSE_KEY_COUNT; i++) {
            if(OS_Mouse_Get_Key_State(i)) {
                printf("OS Mouse down: %d %d\n", i, FrameCount);
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
            printf("Mouse position: (%d, %d)\n", MousePosition.x, MousePosition.y);
            LastMousePosition = MousePosition;
        }

        // vec2i MouseDelta = OS_Mouse_Get_Delta();
        // if(MouseDelta != vec2i()) {
        //     //printf("Mouse delta: (%d, %d)\n", MouseDelta.x, MouseDelta.y);
        // }

        f32 MouseScroll = OS_Mouse_Get_Scroll();
        if(MouseScroll != 0) {
            printf("Mouse scroll: %f\n", MouseScroll);
        }

        if(OS_Keyboard_Get_Key_State(OS_KEYBOARD_KEY_ESCAPE)) {
            return true;
        }
    }

    return true;
}
