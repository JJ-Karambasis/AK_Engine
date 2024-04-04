#ifndef EDITOR_INPUT_H
#define EDITOR_INPUT_H

struct editor_input_manager {
    input KeyboardInput[OS_KEYBOARD_KEY_COUNT];
    input MouseInput[OS_MOUSE_KEY_COUNT];
    vec2  MouseDelta;
    f32   MouseScroll;
    f64   dt;

    bool Is_Down(os_keyboard_key Key);
    bool Is_Pressed(os_keyboard_key Key);
    bool Is_Released(os_keyboard_key Key);

    bool Is_Down(os_mouse_key Key);
    bool Is_Pressed(os_mouse_key Key);
    bool Is_Released(os_mouse_key Key);

    f32 Get_DeltaX();
    f32 Get_DeltaY();
};

void Editor_Input_Manager_New_Frame(editor_input_manager* InputManager, f64 dt);

#endif