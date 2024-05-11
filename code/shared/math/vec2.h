#ifndef VEC2_H
#define VEC2_H

struct base_vec2 {
    union {
        f32_2x Data = {};
        struct {
            f32 x;
            f32 y;
        };
        struct {
            f32 width;
            f32 height;
        };
        f32_2x_sub<2, 0, 1> xy;
        f32_2x_sub<2, 1, 0> yx;
    };

    base_vec2() = default;
    f32&        operator[](u32 Index);
    const f32&  operator[](u32 Index) const;
};

struct vec2 : base_vec2 {
    vec2() = default;
};

struct point2 : base_vec2 {};

typedef vec2 dim2;

struct rect2 {
    point2 Min;
    point2 Max;
    rect2() = default;
};

vec2 Vec2(f32 x, f32 y);
vec2 Vec2(u32 x, u32 y);
vec2 Vec2(s32 x, s32 y);
vec2 Vec2(span<f32> Span);
vec2 Vec2(span<f64> Span);

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

f32  Dot(const vec2& A, const vec2& B);
f32  Wedge(const vec2& A, const vec2& B);
f32  Sq_Mag(const vec2& A);
f32  Mag(const vec2& A);
vec2 Norm(const vec2& A);

point2 operator+(const point2& P, const vec2& Delta);
vec2   operator-(const point2& A, const point2& B);

rect2 Rect2(const point2& Min, const point2& Max);
rect2 Rect2(const point2& Min, const dim2& Dim);
rect2 Rect2(const dim2& Dim);
rect2 Rect2_Translate(const rect2& Rect, const vec2& Delta);
dim2  Rect2_Dim(const rect2& Rect);
bool  Rect2_Is_Empty(const rect2& Rect);
f32   Rect2_Area(const rect2& Rect);

struct base_vec2s {
    union {
        s32_2x Data = {};
        struct {
            s32 x;
            s32 y;
        };
        struct {
            s32 width;
            s32 height;
        };
        s32_2x_sub<2, 0, 1> xy;
        s32_2x_sub<2, 1, 0> yx;
    };

    base_vec2s() = default;
    s32&        operator[](u32 Index);
    const s32&  operator[](u32 Index) const;
};

struct base_vec2u {
    union {
        u32_2x Data = {};
        struct {
            u32 x;
            u32 y;
        };
        struct {
            u32 width;
            u32 height;
        };
        u32_2x_sub<2, 0, 1> xy;
        u32_2x_sub<2, 1, 0> yx;
    };

    base_vec2u() = default;
    u32&        operator[](u32 Index);
    const u32&  operator[](u32 Index) const;
};

struct vec2i : base_vec2s {
    vec2i() = default;
};

struct point2i : base_vec2s {
    point2i() = default;
};

struct dim2i : base_vec2u {
    dim2i() = default;
};

struct rect2i {
    point2i Min;
    point2i Max;
};

vec2i Vec2(f32 x, f32 y);
vec2i Vec2(u32 x, u32 y);
vec2i Vec2(s32 x, s32 y);

vec2i  operator+(s32 A, const vec2i& B);
vec2i  operator+(const vec2i& A, s32 B);
vec2i  operator+(const vec2i& A, const vec2i& B);
vec2i& operator+=(vec2i& A, s32 B);
vec2i& operator+=(vec2i& A, const vec2i& B);

vec2i  operator-(s32 A, const vec2i& B);
vec2i  operator-(const vec2i& A, s32 B);
vec2i  operator-(const vec2i& A, const vec2i& B);
vec2i& operator-=(vec2i& A, s32 B);
vec2i& operator-=(vec2i& A, const vec2i& B);

vec2i  operator*(s32 A, const vec2i& B);
vec2i  operator*(const vec2i& A, s32 B);
vec2i  operator*(const vec2i& A, const vec2i& B);
vec2i& operator*=(vec2i& A, s32 B);
vec2i& operator*=(vec2i& A, const vec2i& B);

vec2i  operator/(s32 A, const vec2i& B);
vec2i  operator/(const vec2i& A, s32 B);
vec2i  operator/(const vec2i& A, const vec2i& B);
vec2i& operator/=(vec2i& A, s32 B);
vec2i& operator/=(vec2i& A, const vec2i& B);

bool operator!=(const vec2i& A, const vec2i& B);
bool operator==(const vec2i& A, const vec2i& B);

point2 operator+(const point2& P, const vec2i& Delta);
point2 operator+(const point2& P, const dim2& Delta);
vec2i   operator-(const point2& A, const point2& B);

rect2i Rect2(const point2i& Min, const point2i& Max);
rect2i Rect2(const point2i& Min, const dim2i& Dim);
rect2i Rect2(const dim2i& Dim);
rect2i Rect2_Translate(const rect2i& Rect, const vec2i& Delta);
dim2i  Rect2_Dim(const rect2i& Rect);
bool   Rect2_Is_Empty(const rect2i& Rect);
u32    Rect2_Area(const rect2i& Rect)

#endif