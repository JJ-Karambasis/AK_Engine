bool operator!=(s32_2x A, s32_2x B) {
    return A.Data[0] != B.Data[0] || A.Data[1] != B.Data[1];
}

bool operator==(s32_2x A, s32_2x B) {
    return A.Data[0] == B.Data[0] && A.Data[1] == B.Data[1];
}

s32_2x operator+(s32_2x A, s32_2x B) {
    return {A.Data[0]+B.Data[0], A.Data[1]+B.Data[1]};
}

s32_2x& operator+=(s32_2x& A, s32_2x B) {
    A = A+B;
    return A;
}

s32_2x operator-(s32_2x A, s32_2x B) {
    return {A.Data[0]-B.Data[0], A.Data[1]-B.Data[1]};
}