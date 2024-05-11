#ifndef VEC4_H
#define VEC4_H

struct vec4 {
    union {
        f32_4x Data = {};
        struct {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };

        f32_2x_sub<4, 0, 1>	xy;
        f32_2x_sub<4, 0, 2>	xz;
        f32_2x_sub<4, 0, 3>	xw;
        f32_2x_sub<4, 1, 0>	yx;
        f32_2x_sub<4, 1, 2>	yz;
        f32_2x_sub<4, 1, 3>	yw;
        f32_2x_sub<4, 2, 0>	zx;
        f32_2x_sub<4, 2, 1>	zy;
        f32_2x_sub<4, 2, 3>	zw;
        f32_2x_sub<4, 3, 0>	wx;
        f32_2x_sub<4, 3, 1>	wy;
        f32_2x_sub<4, 3, 2>	wz;

        f32_3x_sub<4, 0, 1, 2> xyz;
        f32_3x_sub<4, 0, 2, 1> xzy;
        f32_3x_sub<4, 0, 1, 3> xyw;
        f32_3x_sub<4, 0, 3, 1> xwy;
        f32_3x_sub<4, 0, 2, 3> xzw;
        f32_3x_sub<4, 0, 3, 2> xwz;
        f32_3x_sub<4, 1, 0, 2> yxz;
        f32_3x_sub<4, 1, 2, 0> yzx;
        f32_3x_sub<4, 1, 0, 3> yxw;
        f32_3x_sub<4, 1, 3, 0> ywx;
        f32_3x_sub<4, 1, 2, 3> yzw;
        f32_3x_sub<4, 1, 3, 2> ywz;
        f32_3x_sub<4, 2, 0, 1> zxy;
        f32_3x_sub<4, 2, 1, 0> zyx;
        f32_3x_sub<4, 2, 0, 3> zxw;
        f32_3x_sub<4, 2, 3, 0> zwx;
        f32_3x_sub<4, 2, 1, 3> zyw;
        f32_3x_sub<4, 2, 3, 1> zwy;
        f32_3x_sub<4, 3, 0, 1> wxy;
        f32_3x_sub<4, 3, 1, 0> wyx;
        f32_3x_sub<4, 3, 0, 2> wxz;
        f32_3x_sub<4, 3, 2, 0> wzx;
        f32_3x_sub<4, 3, 1, 2> wyz;
        f32_3x_sub<4, 3, 2, 1> wzy;

        f32_4x_sub<4, 0, 1, 2, 3> xyzw;
        f32_4x_sub<4, 0, 1, 3, 2> xywz;
        f32_4x_sub<4, 0, 2, 1, 3> xzyw;
        f32_4x_sub<4, 0, 2, 3, 1> xzwy;
        f32_4x_sub<4, 0, 3, 1, 2> xwyz;
        f32_4x_sub<4, 0, 3, 2, 1> xwzy;
        f32_4x_sub<4, 1, 0, 2, 3> yxzw;
        f32_4x_sub<4, 1, 0, 3, 2> yxwz;
        f32_4x_sub<4, 1, 2, 0, 3> yzxw;
        f32_4x_sub<4, 1, 2, 3, 0> yzwx;
        f32_4x_sub<4, 1, 3, 0, 2> ywxz;
        f32_4x_sub<4, 1, 3, 2, 0> ywzx;
        f32_4x_sub<4, 2, 0, 1, 3> zxyw;
        f32_4x_sub<4, 2, 0, 3, 1> zxwy;
        f32_4x_sub<4, 2, 1, 0, 3> zyxw;
        f32_4x_sub<4, 2, 1, 3, 0> zywx;
        f32_4x_sub<4, 2, 3, 0, 1> zwxy;
        f32_4x_sub<4, 2, 3, 1, 0> zwyx;
        f32_4x_sub<4, 3, 0, 1, 2> wxyz;
        f32_4x_sub<4, 3, 0, 2, 1> wxzy;
        f32_4x_sub<4, 3, 1, 0, 2> wyxz;
        f32_4x_sub<4, 3, 1, 2, 0> wyzx;
        f32_4x_sub<4, 3, 2, 0, 1> wzxy;
        f32_4x_sub<4, 3, 2, 1, 0> wzyx;
    };

    vec4()   = default;
    f32&       operator[](u32 Index);
    const f32& operator[](u32 Index) const;
};

struct point4 : vec4 {

};

struct color4 {
    union {
        f32_4x Data = {};
        struct {
            f32 r;
            f32 g;
            f32 b;
            f32 a;
        };
        color3 rgb;
    };

    color4()   = default;
    f32&       operator[](u32 Index);
    const f32& operator[](u32 Index) const;
};

#endif