f32_2x operator+(f32_2x A, f32_2x B) {
    f32_2x Result = {A.Data[0]+B.Data[0], A.Data[1]+B.Data[1]};
    return Result;
}

f32_2x operator/(f32 A, const f32_2x& B) {
    f32_2x Result = {A/B.Data[0], A/B.Data[1]};
    return Result;
}

s32_2x::s32_2x(std::initializer_list<s32> List) {
    Memory_Copy(Data, List.begin(), List.size()*sizeof(s32));
}

s32_2x::s32_2x(s32 A, s32 B) {
    Data[0] = A;
    Data[1] = B;
}

bool operator!=(s32_2x A, s32_2x B) {
    return A.Data[0] != B.Data[0] || A.Data[1] != B.Data[1];
}

bool operator==(s32_2x A, s32_2x B) {
    return A.Data[0] == B.Data[0] && A.Data[1] == B.Data[1];
}

s32_2x operator+(s32_2x A, s32_2x B) {
    s32_2x Result = {A.Data[0]+B.Data[0], A.Data[1]+B.Data[1]}; 
    return Result;
}

s32_2x& operator+=(s32_2x& A, s32_2x B) {
    A = A+B;
    return A;
}

s32_2x operator-(s32_2x A, s32_2x B) {
    return {A.Data[0]-B.Data[0], A.Data[1]-B.Data[1]};
}