#ifndef INPUT_H
#define INPUT_H

struct input {
    bool WasDown       = false;
    bool IsDown        = false;
    bool IsRepeating   = false;
    f64  RepeatingTime = 0.0;

    inline bool Is_Pressed() const { return IsDown && !WasDown; }
    inline bool Is_Released() const { return !IsDown && WasDown; }
    inline bool Is_Down() const { return IsDown; }

    bool Is_Down_Rate_Repeated(double dt, double WaitTime, double RepeatTime);

    void New_Frame();
};

inline bool Input_Is_Pressed(input* Input) { return Input->IsDown && !Input->WasDown; }
inline bool Input_Is_Released(input* Input) { return !Input->IsDown && Input->WasDown; }
inline bool Input_Is_Down(input* Input) { return Input->IsDown; }

bool Input_Is_Down_Rate_Repeated(input* Input, f64 dt, f64 WaitTime, f64 RepeatTime);
void Input_New_Frame(input* Input);

#endif