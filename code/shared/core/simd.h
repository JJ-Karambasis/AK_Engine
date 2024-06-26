#ifndef SIMD_H
#define SIMD_H

#if defined(_M_X64) || defined(_M_IX86) || defined(__x86_64__) || defined(__SSE__)
	#define SIMD_SSE 1
#endif

#if defined(__AVX__)
	#define SIMD_AVX 1
#endif

#if defined(__ARM_NEON)
	#define SIMD_NEON 1
#endif

#if defined(SIMD_SSE)
#include <emmintrin.h>
#endif

#if defined(SIMD_AVX)
#include <immintrin.h>
#endif

#if defined(SIMD_NEON)
#include <arm_neon.h>
#endif

struct f32_2x {
    union {
        f32 Data[2];
    };

    inline f32& operator[](u32 Index) {
        Assert(Index < 2);
        return Data[Index];
    }
};

f32_2x operator+(f32_2x A, f32_2x B);
f32_2x operator/(f32 A, const f32_2x& B);
f32_2x operator/(const f32_2x& A, const f32_2x& B);

struct f32_3x {
    union {
        f32 Data[3];
    };
};

struct f32_4x {
    union {
#if defined(SIMD_SSE)
        __m128 V;
#elif defined(SIMD_NEON)
        float32x4_t V;
#else 
#error Not Implemented
#endif
        f32 Data[4];
    };
};

static_assert(sizeof(f32_2x) == 8);
static_assert(sizeof(f32_3x) == 12);
static_assert(sizeof(f32_4x) == 16);

struct s32_2x {
    union {
        s32 Data[2];
    };

    s32_2x() = default;
    s32_2x(std::initializer_list<s32> List);
    s32_2x(s32 A, s32 B);

    inline s32& operator[](u32 i) {
        Assert(i < 2);
        return Data[i];
    }
};


bool operator!=(s32_2x A, s32_2x B);
bool operator==(s32_2x A, s32_2x B);
s32_2x operator+(s32_2x A, s32_2x B);
s32_2x& operator+=(s32_2x& A, s32_2x B);
s32_2x operator-(s32_2x A, s32_2x B);

struct s32_3x {
    union {
        s32 Data[3];
    };
};

struct s32_4x {
    union {
        s32 Data[4];
    };
};

static_assert(sizeof(s32_2x) == 8);
static_assert(sizeof(s32_3x) == 12);
static_assert(sizeof(s32_4x) == 16);

struct u32_2x {
    union {
        u32 Data[2];
    };
};

struct u32_3x {
    union {
        u32 Data[3];
    };
};

struct u32_4x {
    union {
        u32 Data[4];
    };
};

static_assert(sizeof(u32_2x) == 8);
static_assert(sizeof(u32_3x) == 12);
static_assert(sizeof(u32_4x) == 16);

#endif