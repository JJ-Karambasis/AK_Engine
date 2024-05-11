#ifndef VEC3_H
#define VEC3_H

struct base_vec3 {
    union {
        f32_3x Data = {};
        struct {
            f32 x;
            f32 y;
            f32 z;
        };
        struct {
            f32 width;
            f32 height;
            f32 depth;
        };
        struct {
            f32 r;
            f32 g;
            f32 b;
        };

        f32_2x_sub<3, 0, 1>	xy;
        f32_2x_sub<3, 0, 2>	xz;
        f32_2x_sub<3, 1, 0>	yx;
        f32_2x_sub<3, 1, 2>	yz;
        f32_2x_sub<3, 2, 0>	zx;
        f32_2x_sub<3, 2, 1>	zy;

        f32_3x_sub<3, 0, 1, 2> xyz;
        f32_3x_sub<3, 0, 2, 1> xzy;
        f32_3x_sub<3, 1, 0, 2> yxz;
        f32_3x_sub<3, 1, 2, 0> yzx;
        f32_3x_sub<3, 2, 0, 1> zxy;
        f32_3x_sub<3, 2, 1, 0> zyx; 
    };

    base_vec3() = default;
    f32&        operator[](u32 Index);
    const f32&  operator[](u32 Index) const;
};

struct vec3 : base_vec3 {
    vec3() = default;
};

struct bivec3 : base_vec3 {
    bivec3() = default;
};

struct point3 : base_vec3 {
    point3() = default;
};

typedef vec3 dim3;

struct color3 : base_vec3 {
    color3() = default;
};

struct rect3 {
    point3 Min;
    point3 Max;
    rect3() = default;
};

vec3 Vec3(f32 x, f32 y, f32 z);
vec3 Vec3(u32 x, u32 y, u32 z);
vec3 Vec3(s32 x, s32 y, s32 z);
vec3 Vec3(span<f32> Span);
vec3 Vec3(span<f64> Span);

vec2  operator+(f32 A, const vec2& B);
vec2  operator+(const vec2& A, f32 B);
vec2  operator+(const vec2& A, const vec2& B);
vec2& operator+=(vec2& A, f32 B);
vec2& operator+=(vec2& A, const vec2& B);

vec2  operator-(f32 A, const vec2& B);
vec2  operator-(const vec2& A, f32 B);
vec2  operator-(const vec2& A, const vec2& B);
vec2& operator-=(vec2& A, f32 B);
vec2& operator-=(vec2& A, const vec2& B);

vec2  operator*(f32 A, const vec2& B);
vec2  operator*(const vec2& A, f32 B);
vec2  operator*(const vec2& A, const vec2& B);
vec2& operator*=(vec2& A, f32 B);
vec2& operator*=(vec2& A, const vec2& B);

vec2  operator/(f32 A, const vec2& B);
vec2  operator/(const vec2& A, f32 B);
vec2  operator/(const vec2& A, const vec2& B);
vec2& operator/=(vec2& A, f32 B);
vec2& operator/=(vec2& A, const vec2& B);

bool operator!=(const vec2& A, const vec2& B);
bool operator==(const vec2& A, const vec2& B);

f32    Dot(const vec2& A, const vec2& B);
bivec3 Wedge(const vec2& A, const vec2& B);
f32    Sq_Mag(const vec2& A);
f32    Mag(const vec2& A);
vec2   Norm(const vec2& A);

point3 operator+(const point3& P, const vec3& Delta);
vec3   operator-(const point3& A, const point3& B);

rect3 Rect3(const point3& Min, const point3& Max);
rect3 Rect3(const point3& Min, const dim3& Dim);
rect3 Rect3(const dim3& Dim);
rect3 Rect3_Translate(const rect3& Rect, const vec3& Delta);
dim3  Rect3_Dim(const rect3& Rect);
bool  Rect3_Is_Empty(const rect3& Rect);
f32   Rect3_Area(const rect3& Rect);

#endif