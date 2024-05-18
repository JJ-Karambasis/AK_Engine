#ifndef OS_H
#define OS_H


#define OS_MAX_PROCESS_COUNT 128
#define OS_MAX_WINDOW_COUNT 128

bool Application_Main();

struct os;
os*    OS_Get();
void   OS_Set(os* OS);
string OS_Get_Executable_Path();
bool   OS_Directory_Exists(string Directory);
bool   OS_File_Exists(string File);
void   OS_Message_Box(string Message, string Title);

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
    rect2i Rect;
};

typedef u64 os_monitor_id;
span<os_monitor_id>    OS_Get_Monitors();
os_monitor_id          OS_Get_Primary_Monitor();
const os_monitor_info* OS_Get_Monitor_Info(os_monitor_id ID);             

//Window api
enum {
    OS_WINDOW_FLAG_NONE,
    OS_WINDOW_FLAG_MAIN_BIT = (1 << 0),
    OS_WINDOW_FLAG_MAXIMIZE_BIT = (1 << 1)
};
typedef u32 os_window_flags;

struct os_open_window_info {
    os_window_flags Flags;
    string          Title;
    os_monitor_id   Monitor;
    point2i         Pos; //Position relative to monitor if monitor id is valid
    dim2i           Size; 
    void*           UserData;
};

typedef u64 os_window_id;
os_window_id OS_Open_Window(const os_open_window_info& OpenInfo);
void         OS_Close_Window(os_window_id WindowID);
bool         OS_Window_Is_Open(os_window_id WindowID);
void         OS_Set_Main_Window(os_window_id WindowID);
os_window_id OS_Get_Main_Window();
void         OS_Window_Set_Data(os_window_id WindowID, void* UserData);
void*        OS_Window_Get_Data(os_window_id WindowID);
void         OS_Window_Set_Title(os_window_id WindowID, string Title);
dim2i        OS_Window_Get_Size(os_window_id WindowID);
point2i      OS_Window_Get_Pos(os_window_id WindowID);

//Event api
typedef u32 os_keyboard_key;
typedef u32 os_mouse_key;
enum os_event_type {
    OS_EVENT_TYPE_NONE,
    OS_EVENT_TYPE_WINDOW_CLOSED,
    OS_EVENT_TYPE_MOUSE_SCROLL,
    OS_EVENT_TYPE_MOUSE_DELTA,
    OS_EVENT_TYPE_COUNT
};

struct os_event {
    os_event_type Type;
    os_window_id  Window;
};

struct os_mouse_delta_event : os_event {
    vec2i Delta;
};

struct os_mouse_scroll_event : os_event {
    f32 Scroll;
};

const os_event* OS_Next_Event();
bool            OS_Keyboard_Get_Key_State(os_keyboard_key Key);
bool            OS_Mouse_Get_Key_State(os_mouse_key Key);
point2i         OS_Mouse_Get_Position();

//Input keys
enum {
    OS_MOUSE_KEY_LEFT,
    OS_MOUSE_KEY_MIDDLE,
    OS_MOUSE_KEY_RIGHT,
    OS_MOUSE_KEY_COUNT
};

enum {
    OS_KEYBOARD_KEY_A,
    OS_KEYBOARD_KEY_B, 
    OS_KEYBOARD_KEY_C,
    OS_KEYBOARD_KEY_D,
    OS_KEYBOARD_KEY_E,
    OS_KEYBOARD_KEY_F,
    OS_KEYBOARD_KEY_G,
    OS_KEYBOARD_KEY_H,
    OS_KEYBOARD_KEY_I,
    OS_KEYBOARD_KEY_J,
    OS_KEYBOARD_KEY_K,
    OS_KEYBOARD_KEY_L,
    OS_KEYBOARD_KEY_M,
    OS_KEYBOARD_KEY_N,
    OS_KEYBOARD_KEY_O,
    OS_KEYBOARD_KEY_P,
    OS_KEYBOARD_KEY_Q,
    OS_KEYBOARD_KEY_R,
    OS_KEYBOARD_KEY_S,
    OS_KEYBOARD_KEY_T,
    OS_KEYBOARD_KEY_U,
    OS_KEYBOARD_KEY_V, 
    OS_KEYBOARD_KEY_W, 
    OS_KEYBOARD_KEY_X, 
    OS_KEYBOARD_KEY_Y, 
    OS_KEYBOARD_KEY_Z, 
    OS_KEYBOARD_KEY_ZERO, 
    OS_KEYBOARD_KEY_ONE,
    OS_KEYBOARD_KEY_TWO,
    OS_KEYBOARD_KEY_THREE,
    OS_KEYBOARD_KEY_FOUR,
    OS_KEYBOARD_KEY_FIVE,
    OS_KEYBOARD_KEY_SIX,
    OS_KEYBOARD_KEY_SEVEN,
    OS_KEYBOARD_KEY_EIGHT,
    OS_KEYBOARD_KEY_NINE,
    OS_KEYBOARD_KEY_SPACE,
    OS_KEYBOARD_KEY_TAB,
    OS_KEYBOARD_KEY_ESCAPE,
    OS_KEYBOARD_KEY_UP,
    OS_KEYBOARD_KEY_DOWN,
    OS_KEYBOARD_KEY_LEFT, 
    OS_KEYBOARD_KEY_RIGHT,
    OS_KEYBOARD_KEY_BACKSPACE,
    OS_KEYBOARD_KEY_RETURN,
    OS_KEYBOARD_KEY_DELETE,
    OS_KEYBOARD_KEY_INSERT, 
    OS_KEYBOARD_KEY_SHIFT,
    OS_KEYBOARD_KEY_CONTROL,
    OS_KEYBOARD_KEY_ALT,
    OS_KEYBOARD_KEY_F1,
    OS_KEYBOARD_KEY_F2,
    OS_KEYBOARD_KEY_F3,
    OS_KEYBOARD_KEY_F4,
    OS_KEYBOARD_KEY_F5,
    OS_KEYBOARD_KEY_F6, 
    OS_KEYBOARD_KEY_F7, 
    OS_KEYBOARD_KEY_F8, 
    OS_KEYBOARD_KEY_F9, 
    OS_KEYBOARD_KEY_F10,
    OS_KEYBOARD_KEY_F11, 
    OS_KEYBOARD_KEY_F12,
    OS_KEYBOARD_KEY_COMMAND,
    OS_KEYBOARD_KEY_COUNT
};

#endif