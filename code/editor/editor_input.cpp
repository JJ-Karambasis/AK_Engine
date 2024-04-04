bool editor_input_manager::Is_Down(os_keyboard_key Key) {
    Assert(Key < OS_KEYBOARD_KEY_COUNT);
    return KeyboardInput[Key].Is_Down();
}

bool editor_input_manager::Is_Pressed(os_keyboard_key Key) {
    Assert(Key < OS_KEYBOARD_KEY_COUNT);
    return KeyboardInput[Key].Is_Pressed();
}

bool editor_input_manager::Is_Released(os_keyboard_key Key) {
    Assert(Key < OS_KEYBOARD_KEY_COUNT);
    return KeyboardInput[Key].Is_Released();
}

bool editor_input_manager::Is_Down(os_mouse_key Key) {
    Assert(Key < OS_MOUSE_KEY_COUNT);
    return MouseInput[Key].Is_Down();
}

bool editor_input_manager::Is_Pressed(os_mouse_key Key) {
    Assert(Key < OS_MOUSE_KEY_COUNT);
    return MouseInput[Key].Is_Pressed();
}

bool editor_input_manager::Is_Released(os_mouse_key Key) {
    Assert(Key < OS_MOUSE_KEY_COUNT);
    return MouseInput[Key].Is_Released();
}

f32 editor_input_manager::Get_DeltaX() {
    return MouseDelta.x*(f32)dt;
}

f32 editor_input_manager::Get_DeltaY() {
    return MouseDelta.y*(f32)dt;
}

void Editor_Input_Manager_New_Frame(editor_input_manager* InputManager, f64 dt) {
    InputManager->dt = dt;
    for(input& KeyboardInput : InputManager->KeyboardInput) { Input_New_Frame(&KeyboardInput); }
    for(input& MouseInput : InputManager->MouseInput) { Input_New_Frame(&MouseInput); }
    InputManager->MouseDelta  = vec2();
    InputManager->MouseScroll = 0.0f;
}