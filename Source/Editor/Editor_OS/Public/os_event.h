#ifndef OS_EVENT_H
#define OS_EVENT_H

typedef enum event_type
{
    OS_EVENT_TYPE_NONE,
    OS_EVENT_TYPE_WINDOW_CLOSED,
    OS_EVENT_TYPE_WINDOW_RESIZED,
    OS_EVENT_TYPE_TEXT_INPUT,
    OS_EVENT_TYPE_KEY_PRESSED,
    OS_EVENT_TYPE_KEY_RELEASED
} event_type;

typedef enum os_keycode
{
    OS_KEYCODE_UNKNOWN,
    OS_KEYCODE_A,
    OS_KEYCODE_B,
    OS_KEYCODE_C,
    OS_KEYCODE_D,
    OS_KEYCODE_E,
    OS_KEYCODE_F,
    OS_KEYCODE_G,
    OS_KEYCODE_H,
    OS_KEYCODE_I,
    OS_KEYCODE_J,
    OS_KEYCODE_K,
    OS_KEYCODE_L,
    OS_KEYCODE_M,
    OS_KEYCODE_N,
    OS_KEYCODE_O,
    OS_KEYCODE_P,
    OS_KEYCODE_Q,
    OS_KEYCODE_R,
    OS_KEYCODE_S,
    OS_KEYCODE_T,
    OS_KEYCODE_U,
    OS_KEYCODE_V,
    OS_KEYCODE_W,
    OS_KEYCODE_X,
    OS_KEYCODE_Y,
    OS_KEYCODE_Z,
    OS_KEYCODE_0,
    OS_KEYCODE_1,
    OS_KEYCODE_2,
    OS_KEYCODE_3,
    OS_KEYCODE_4,
    OS_KEYCODE_5,
    OS_KEYCODE_6,
    OS_KEYCODE_7,
    OS_KEYCODE_8,
    OS_KEYCODE_9,
    OS_KEYCODE_SPACE,
    OS_KEYCODE_TICK,
    OS_KEYCODE_MINUS,
    OS_KEYCODE_EQUAL,
    OS_KEYCODE_LEFTBRACKET,
    OS_KEYCODE_RIGHTBRACKET,
    OS_KEYCODE_SEMICOLON,
    OS_KEYCODE_QUOTE,
    OS_KEYCODE_COMMA,
    OS_KEYCODE_PERIOD,
    OS_KEYCODE_FORWARDSLASH,
    OS_KEYCODE_BACKSLASH,
    OS_KEYCODE_TAB,
    OS_KEYCODE_ESCAPE,
    OS_KEYCODE_PAUSE,
    OS_KEYCODE_UP,
    OS_KEYCODE_DOWN,
    OS_KEYCODE_LEFT,
    OS_KEYCODE_RIGHT,
    OS_KEYCODE_BACKSPACE,
    OS_KEYCODE_RETURN,
    OS_KEYCODE_DELETE,
    OS_KEYCODE_INSERT,
    OS_KEYCODE_HOME,
    OS_KEYCODE_END,
    OS_KEYCODE_PAGEUP,
    OS_KEYCODE_PAGEDOWN,
    OS_KEYCODE_CAPSLOCK,
    OS_KEYCODE_NUMLOCK,
    OS_KEYCODE_SCROLLLOCK,
    OS_KEYCODE_SHIFT,
    OS_KEYCODE_CONTROL,
    OS_KEYCODE_ALT,
    OS_KEYCODE_LSHIFT,
    OS_KEYCODE_RSHIFT,
    OS_KEYCODE_LCONTROL,
    OS_KEYCODE_RCONTROL,
    OS_KEYCODE_LALT,
    OS_KEYCODE_RALT,
    OS_KEYCODE_LSUPER,
    OS_KEYCODE_RSUPER,
    OS_KEYCODE_PRINTSCREEN,
    OS_KEYCODE_MENU,
    OS_KEYCODE_COMMAND,
    OS_KEYCODE_F1,
    OS_KEYCODE_F2,
    OS_KEYCODE_F3,
    OS_KEYCODE_F4,
    OS_KEYCODE_F5,
    OS_KEYCODE_F6,
    OS_KEYCODE_F7,
    OS_KEYCODE_F8,
    OS_KEYCODE_F9,
    OS_KEYCODE_F10,
    OS_KEYCODE_F11,
    OS_KEYCODE_F12,
    OS_KEYCODE_F13,
    OS_KEYCODE_F14,
    OS_KEYCODE_F15,
    OS_KEYCODE_F16,
    OS_KEYCODE_F17,
    OS_KEYCODE_F18,
    OS_KEYCODE_F19,
    OS_KEYCODE_F20,
    OS_KEYCODE_F21,
    OS_KEYCODE_F22,
    OS_KEYCODE_F23,
    OS_KEYCODE_F24,
    OS_KEYCODE_NUMPAD0,
    OS_KEYCODE_NUMPAD1,
    OS_KEYCODE_NUMPAD2,
    OS_KEYCODE_NUMPAD3,
    OS_KEYCODE_NUMPAD4,
    OS_KEYCODE_NUMPAD5,
    OS_KEYCODE_NUMPAD6,
    OS_KEYCODE_NUMPAD7,
    OS_KEYCODE_NUMPAD8,
    OS_KEYCODE_NUMPAD9,
    OS_KEYCODE_NUMPADSTAR,
    OS_KEYCODE_NUMPADPLUS,
    OS_KEYCODE_NUMPADMINUS,
    OS_KEYCODE_NUMPADDOT,
    OS_KEYCODE_NUMPADSLASH,
    OS_KEYCODE_COUNT
} os_keycode;

#define OS_MODIFIER_ALT (1 << 0)
#define OS_MODIFIER_SHIFT (1 << 1)
#define OS_MODIFIER_CONTROL (1 << 2)

typedef struct os_event
{
    event_type Type;
    os_window* Window;
    
    union
    {
        uint32_t TextInputUTF32;
        
        struct
        {
            os_keycode Keycode;
            uint32_t   Modifiers;
        } KeyPressed, KeyReleased;
    };
} os_event;

void            OS_Poll_Events();
const os_event* OS_Get_Next_Event();

#endif