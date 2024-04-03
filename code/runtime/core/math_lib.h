#ifndef MATH_LIB_H
#define MATH_LIB_H

#define PI 3.14159265359f
#define INV_PI 0.31830988618f
#define To_Radians(degrees) ((degrees) * (PI/180.0f)) 

union uvec2 {
    u32 Data[2] = {};
    struct { u32 x, y; };
    struct { u32 w, h; };
    uvec2() = default;
};

bool operator!=(uvec2 A, uvec2 B);

union vec3 {
    f32 Data[3] = {};
    struct { f32 x, y, z; };
    struct { f32 w, h, d; };
    struct { f32 r, g, b; };
    vec3() = default;
    vec3(f32 _x, f32 _y, f32 _z);
};

f32  Vec3_Dot(vec3 A, vec3 B);
vec3 Vec3_Cross(vec3 A, vec3 B);
vec3 operator+(vec3 A, vec3 B);
vec3 operator*(vec3 A, f32 B);

union vec4 {
    f32 Data[4] = {};
    struct { f32 x, y, z, w; };
    struct { f32 r, g, b, a; };
    struct { vec3 xyz; f32 Unused__0; };
    vec4() = default;
    vec4(f32 _x, f32 _y, f32 _z, f32 _w);
};

union quat {
    f32 Data[4] = {0, 0, 0, 1};
    struct { f32 x, y, z, w; };
    struct { vec3 v; f32 s; };

    quat() = default;
    quat(f32 _x, f32 _y, f32 _z, f32 _w);
    quat(vec3 _v, f32 _s);
};

quat Quat_RotX(f32 Angle);
quat Quat_RotY(f32 Angle);
quat Quat_RotZ(f32 Angle);
f32  Quat_Dot(quat A, quat B);
f32  Quat_Sq_Mag(quat Q);
quat Quat_Normalize(quat Q);
quat operator*(quat A, f32 B);
quat operator*(quat A, quat B);

union matrix3 {
    f32  Data[9] = {
        1, 0, 0, 
        0, 1, 0, 
        0, 0, 1
    };
    vec3 Rows[3];
    struct { vec3 x, y, z; };
    struct {
        f32 m00, m01, m02;
        f32 m10, m11, m12;
        f32 m20, m21, m22;
    };

    matrix3(quat Q);
};

union matrix4_affine {
    f32 Data[12] = {
        1, 0, 0, 
        0, 1, 0, 
        0, 0, 1, 
        0, 0, 0
    };
    vec3 Rows[4];
    struct {
        matrix3 M;
        vec3 Unused__0;
    };
    struct {
        vec3 x, y, z, t;
    };
    struct {
        f32 m00, m01, m02;
        f32 m10, m11, m12;
        f32 m20, m21, m22;
        f32 m30, m31, m32;
    };

    matrix4_affine() = default;
    matrix4_affine(std::initializer_list<f32> List);
};

void Matrix4_Affine_Translation(matrix4_affine* Result, vec3 Translation);
void Matrix4_Affine_Transpose(matrix4_affine* Result, const matrix4_affine& M);
void Matrix4_Affine_Inverse_No_Scale(matrix4_affine* Result, vec3 P, const matrix3& M);

union matrix4 {
    f32 Data[16] = {
        1, 0, 0, 0, 
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    vec4 Rows[4];
    struct {
        vec3 x; f32 Unused__0;
        vec3 y; f32 Unused__1;
        vec3 z; f32 Unused__2;
        vec3 t; f32 Unused__3;
    };
    struct {
        f32 m00, m01, m02, m03;
        f32 m10, m11, m12, m13;
        f32 m20, m21, m22, m23;
        f32 m30, m31, m32, m33;
    };
};

void Matrix4_Zero(matrix4* M);
void Matrix4_Transpose(matrix4* Result, const matrix4& M);
void Matrix4_Perspective(matrix4* M, f32 AspectRatio, f32 FieldOfView, f32 NearPlane, f32 FarPlane);
matrix4 operator*(const matrix4_affine& A, const matrix4& B);

#endif