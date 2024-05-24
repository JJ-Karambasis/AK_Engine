#ifndef MATRIX_H
#define MATRIX_H

struct matrix4 {
    union {
        f32 Data[16] = {
            1, 0, 0, 0, 
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 0
        };
        f32_4x Rows[4];
        struct {
            f32_3x x; f32 __unused0__;
            f32_3x y; f32 __unused1__;
            f32_3x z; f32 __unused2__;
            f32_3x t; f32 __unused3__;
        };
        struct {
            f32 m00, m01, m02, m03;
            f32 m10, m11, m12, m13;
            f32 m20, m21, m22, m23;
            f32 m30, m31, m32, m33;
        };
    };
};

matrix4 Transpose(const matrix4& M);
matrix4 Ortho_Projection2D(f32 L, f32 R, f32 T, f32 B);
matrix4 Ortho_Projection2D(f32 Width, f32 Height);
#endif