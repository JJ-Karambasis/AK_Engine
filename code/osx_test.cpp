#include <core/core.h>
#include <os/os.h>

int main() {
    Core_Create();
    OS_Create({});
    OS_Process_Exit(OS_Exec_Process(String_Lit("echo"), String_Lit("hello")));

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
}

#include <core/core.cpp>