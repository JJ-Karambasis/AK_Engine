#ifndef OS_H
#define OS_H

bool Application_Main();

struct os;
os*    OS_Get();
void   OS_Set(os* OS);
// string OS_Get_Executable_Path();
// bool   OS_Directory_Exists(string Directory);
// bool   OS_File_Exists(string File);
// void   OS_Message_Box(string Message, string Title);

//Process api
enum os_process_status {
    OS_PROCESS_STATUS_NONE,
    OS_PROCESS_STATUS_ACTIVE,
    OS_PROCESS_STATUS_EXIT
};

typedef u64 os_process_id;
os_process_id     OS_Exec_Process(string App, string Parameters);
void              OS_Wait_For_Process(os_process_id ID);
os_process_status OS_Process_Status(os_process_id ID);
int               OS_Process_Exit(os_process_id ID);

//Monitor api
struct os_monitor_info {
    string Name;
    rect2  Rect;
};

typedef u64 os_monitor_id;
span<os_monitor_id>    OS_Get_Monitors();
os_monitor_id          OS_Get_Primary_Monitor();
const os_monitor_info* OS_Get_Monitor_Info(os_monitor_id ID);             

//Event api
//#include "os_event.h"

#endif