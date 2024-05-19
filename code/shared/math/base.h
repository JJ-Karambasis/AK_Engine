#ifndef BASE_H
#define BASE_H

struct vec2;
struct vec2i;
struct point2;
struct point2i;
struct dim2;
struct dim2i;
struct rect2;
struct rect2i;

struct vec2 {
    union {
        f32_2x Data;
        struct {
            f32 x, y;
        };
    };
};

struct vec2i {
    union {
        s32_2x Data;
        struct {
            s32 x, y;
        };
    };

    vec2i() = default;
    vec2i(s32 x, s32 y);
    vec2i(s32_2x Data);
};

bool operator!=(const vec2i& A, const vec2i& B);
bool operator==(const vec2i& A, const vec2i& B);

struct point2 {
    union {
        f32_2x Data;
        struct {
            f32 x, y;
        };
    };
};

struct point2i {
    union {
        s32_2x Data;
        struct {
            s32 x, y;
        };
    };

    point2i() = default;
    point2i(s32 x, s32 y);
    point2i(s32_2x Data);
};

bool operator!=(const point2i& A, const point2i& B);
bool operator==(const point2i& A, const point2i& B);
point2i operator+(const point2i& A, const point2i& B);
point2i operator+=(point2i& A, const point2i& B);
point2i operator+(const point2i& A, const dim2i& B);
vec2i operator-(const point2i& A, const point2i& B);

struct dim2 {
    union {
        f32_2x Data;
        struct {
            f32 width, height;
        };
    };
};

struct dim2i {
    union {
        s32_2x Data;
        struct {
            s32 width, height;
        };
    };

    dim2i() = default;
    dim2i(s32 w, s32 h);
    dim2i(const vec2i& Extent);
    dim2i(s32_2x Data);
};

bool operator!=(const dim2i& A, const dim2i& B);
bool operator==(const dim2i& A, const dim2i& B);

struct rect2 {
    point2 P1;
    point2 P2;
};

struct rect2i {
    point2i P1;
    point2i P2;

    rect2i() = default;
    rect2i(const point2i& Min, const point2i& Max);
};

s32   Rect2i_Get_Height(const rect2i& Rect);
dim2i Rect2i_Get_Dim(const rect2i& Rect);

struct vec4 {
    union {
        f32_4x Data;
        struct {
            f32 x, y, z, w;
        };
    };
};

struct point4 {
    union {
        f32_4x Data;
        struct {
            f32 x, y, z, w;
        };
    };
};

struct color4 {
    union {
        f32_4x Data;
        struct {
            f32 r, g, b, a;
        };
    };
};

#endif