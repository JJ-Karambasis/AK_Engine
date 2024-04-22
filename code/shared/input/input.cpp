bool Input_Is_Down_Rate_Repeated(input* Input, f64 dt, f64 WaitTime, f64 RepeatTime) {
    if(Input_Is_Pressed(Input)) {
        return true;
    }

    if(Input->IsDown) {
        if(Input->IsRepeating) {
            Input->RepeatingTime += dt;
            if(Input->RepeatingTime >= RepeatTime) {
                Input->RepeatingTime -= RepeatTime;
                return true;
            }
        } else {
            Input->RepeatingTime += dt;
            if(Input->RepeatingTime >= WaitTime) {
                Input->IsRepeating = true;
                Input->RepeatingTime = 0.0;
                return true;
            }
        }
    }

    return false;
}

void Input_New_Frame(input* Input) {
    Input->WasDown = Input->IsDown;
    if(!Input->IsDown) {
        Input->RepeatingTime = 0.0f;
        Input->IsRepeating = false;
    }
    Input->IsDown = false;
}