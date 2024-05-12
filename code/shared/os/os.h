#ifndef OS_H
#define OS_H

struct os_create_info {
    u32 MaxWindowCount  = 128;
    u32 MaxProcessCount = 128;
};

struct os;
os*    OS_Create(const os_create_info& CreateInfo);
void   OS_Delete();
os*    OS_Get();
void   OS_Set(os* OS);
string OS_Get_Executable_Path();
bool   OS_Directory_Exists(string Directory);
bool   OS_File_Exists(string File);

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

#endif