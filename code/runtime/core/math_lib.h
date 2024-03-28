#ifndef MATH_LIB_H
#define MATH_LIB_H

union uvec2 {
    u32 Data[2] = {};
    struct { u32 x, y; };
    struct { u32 w, h; };
    uvec2() = default;
};

inline bool operator!=(uvec2 A, uvec2 B) {
    return A.x != B.x || A.y != B.y;
}

#endif