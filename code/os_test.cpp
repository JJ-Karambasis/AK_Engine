#include <core/core.h>
#include <os.h>

#include <stdio.h>

bool Application_Main() {
    OS_Process_Exit(OS_Exec_Process(String_Lit("echo"), String_Lit("hello")));
    OS_Process_Exit(OS_Exec_Process(String_Lit("echo"), String_Lit("world")));

    span<os_monitor_id> Monitors = OS_Get_Monitors();
    os_monitor_id PrimaryMonitor = OS_Get_Primary_Monitor();
    for(os_monitor_id Monitor : Monitors) {
        const os_monitor_info* MonitorInfo = OS_Get_Monitor_Info(Monitor);
        printf("Monitor %.*s has rectangle: {(%f, %f), (%f, %f)}.", (int)MonitorInfo->Name.Size, MonitorInfo->Name.Str, 
        MonitorInfo->Rect.Min.x, MonitorInfo->Rect.Min.y, MonitorInfo->Rect.Max.x, MonitorInfo->Rect.Max.y);

        if(Monitor == PrimaryMonitor) {
            printf(" Monitor is primary!\n");
        } else {
            printf("\n");
        }
    }

    svec2 LastMousePosition;
    for(;;) {
        for(u32 i = 0; i < OS_KEYBOARD_KEY_COUNT; i++) {
            if(OS_Keyboard_Get_Key_State(i)) {
                printf("OS Key down: %d\n", i);
            }
        }

        for(u32 i = 0; i < OS_MOUSE_KEY_COUNT; i++) {
            if(OS_Mouse_Get_Key_State(i)) {
                printf("OS Mouse down: %d\n", i);
            }
        }

        svec2 MousePosition = OS_Mouse_Get_Position();
        if(MousePosition.x != LastMousePosition.x || MousePosition.y != LastMousePosition.y) {
            printf("Mouse position: (%d, %d)\n", MousePosition.x, MousePosition.y);
            LastMousePosition = MousePosition;
        }

        svec2 MouseDelta = OS_Mouse_Get_Delta();
        if(MouseDelta.x != 0 || MouseDelta.y != 0) {
            printf("Mouse delta: (%d, %d)\n", MouseDelta.x, MouseDelta.y);
        }

        f32 MouseScroll = OS_Mouse_Get_Scroll();
        if(MouseScroll != 0) {
            printf("Mouse scroll: %f\n", MouseScroll);
        }

        if(OS_Keyboard_Get_Key_State(OS_KEYBOARD_KEY_ESCAPE)) {
            return true;
        }
    }
}