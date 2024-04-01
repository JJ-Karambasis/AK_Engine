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

union vec3 {
    f32 Data[3] = {};
    struct { f32 x, y, z; };
    struct { f32 w, h, d; };
    struct { f32 r, g, b; };
    vec3() = default;
    vec3(f32 _x, f32 _y, f32 _z);
};

#endif