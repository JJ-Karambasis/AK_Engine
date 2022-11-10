#ifndef BASE_H
#define BASE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <intrin.h>

typedef int8_t  bool8_t;
typedef int16_t bool16_t;
typedef int32_t bool32_t;

#define global static
#define local static

#if defined(COMPILER_MSVC) || defined(COMPILER_CLANG)
#define thread_local __declspec(thread)
#define shared_export __declspec(dllexport)
#else
#error Not Implemented
#endif

#ifdef OS_WIN32
#define Debug_Break() __debugbreak()
#else
#error Not Implemented
#endif

#define Glue_(a, b) a##b
#define Glue(a, b) Glue_(a, b)

#define LINE_NUMBER __LINE__
#define FUNCTION_NAME __FUNCSIG__
#define FILENAME __FILE__

#ifdef ENABLE_ASSERTS
#define Assert(c) do { if(!(c)) Debug_Break(); } while(0)
#else
#define Assert(c)
#endif

#if defined(COMPILER_MSVC)
#define alignof(x) __alignof(x)
#elif defined(COMPILER_CLANG)
#define alignof(x) __alignof__(x)
#endif

#define Abs(a) (((a) < 0) ? -(a) : (a))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Is_Pow2(x) ((x != 0) && ((x & (x - 1)) == 0))

#define Static_Assert(c) char Glue(__Ignore__Value__, LINE_NUMBER)[(c) ? 1 : -1]
Static_Assert(alignof(int) == 4);

#define SLL_Push_Back(First, Last, Node) (!First ? (First = Last = Node) : (Last->Next = Node, Last = Node))
#define SLL_Push_Front(First, Node) (Node->Next = First, First = Node)
#define SLL_Pop_Front(First) (First = First->Next)

#define Stringify_(a) #a
#define Stringify(a) Stringify_(a)

#define Kilo(x) ((x)*1024LLU)
#define Mega(x) (Kilo(x)*1024LLU)
#define Giga(x) (Mega(x)*1024LLU)
#define Tera(x) (Giga(x)*1024LLU)

#define MAX_U8 0xFF
#define MAX_U16 0xFFFF
#define MAX_U32 0xFFFFFFFF
#define MAX_U64 0xFFFFFFFFFFFFFFFF

#define MAX_S8 127
#define MAX_S16 32767
#define MAX_S32 2147483647
#define MAX_S64 9223372036854775807

#define MIN_S8 (-127 - 1)
#define MIN_S16 (-32767 - 1)
#define MIN_S32 (-2147483647 - 1)
#define MIN_S64 (-9223372036854775807 - 1)

#define MAX_F32 3.402823466e+38f;
#define MIN_F32 -MAX_F32
#define SMALLEST_F32 1.1754943508e-38f
#define EPSILON_F32 5.96046448e-8f
#define PI_F32 3.14159265359f
#define TAU_F32 6.28318530718f
#define EXP_F32 2.71828182846f
#define INFINITY_F32 ((float)0x7F800000)

#define MAX_F64 1.79769313486231e+308
#define MIN_F64 -MAX_F64
#define SMALLEST_F64 4.94065645841247e-324
#define EPSILON_F64 1.11022302462515650e-16
#define PI_F64 3.14159265359
#define TAU_F64 6.28318530718
#define EXP_F64 2.71828182846
#define INFINITY_F64 ((float)0x7FF0000000000000)

#define BIT_1  0x00000001
#define BIT_2  0x00000002
#define BIT_3  0x00000004
#define BIT_4  0x00000008
#define BIT_5  0x00000010
#define BIT_6  0x00000020
#define BIT_7  0x00000040
#define BIT_8  0x00000080
#define BIT_9  0x00000100
#define BIT_10 0x00000200
#define BIT_11 0x00000400
#define BIT_12 0x00000800
#define BIT_13 0x00001000
#define BIT_14 0x00002000
#define BIT_15 0x00004000
#define BIT_16 0x00008000
#define BIT_17 0x00010000
#define BIT_18 0x00020000
#define BIT_19 0x00040000
#define BIT_20 0x00080000
#define BIT_21 0x00100000
#define BIT_22 0x00200000
#define BIT_23 0x00400000
#define BIT_24 0x00800000
#define BIT_25 0x01000000
#define BIT_26 0x02000000
#define BIT_27 0x04000000
#define BIT_28 0x08000000
#define BIT_29 0x10000000
#define BIT_30 0x20000000
#define BIT_31 0x40000000
#define BIT_32 0x80000000

#define BIT_33 0x0000000100000000
#define BIT_34 0x0000000200000000
#define BIT_35 0x0000000400000000
#define BIT_36 0x0000000800000000
#define BIT_37 0x0000001000000000
#define BIT_38 0x0000002000000000
#define BIT_39 0x0000004000000000
#define BIT_40 0x0000008000000000
#define BIT_41 0x0000010000000000
#define BIT_42 0x0000020000000000
#define BIT_43 0x0000040000000000
#define BIT_44 0x0000080000000000
#define BIT_45 0x0000100000000000
#define BIT_46 0x0000200000000000
#define BIT_47 0x0000400000000000
#define BIT_48 0x0000800000000000
#define BIT_49 0x0001000000000000
#define BIT_50 0x0002000000000000
#define BIT_51 0x0004000000000000
#define BIT_52 0x0008000000000000
#define BIT_53 0x0010000000000000
#define BIT_54 0x0020000000000000
#define BIT_55 0x0040000000000000
#define BIT_56 0x0080000000000000
#define BIT_57 0x0100000000000000
#define BIT_58 0x0200000000000000
#define BIT_59 0x0400000000000000
#define BIT_60 0x0800000000000000
#define BIT_61 0x1000000000000000
#define BIT_62 0x2000000000000000
#define BIT_63 0x4000000000000000
#define BIT_64 0x8000000000000000

#define BITMASK_1  0x00000001
#define BITMASK_2  0x00000003
#define BITMASK_3  0x00000007
#define BITMASK_4  0x0000000f
#define BITMASK_5  0x0000001f
#define BITMASK_6  0x0000003f
#define BITMASK_7  0x0000007f
#define BITMASK_8  0x000000ff
#define BITMASK_9  0x000001ff
#define BITMASK_10 0x000003ff
#define BITMASK_11 0x000007ff
#define BITMASK_12 0x00000fff
#define BITMASK_13 0x00001fff
#define BITMASK_14 0x00003fff
#define BITMASK_15 0x00007fff
#define BITMASK_16 0x0000ffff
#define BITMASK_17 0x0001ffff
#define BITMASK_18 0x0003ffff
#define BITMASK_19 0x0007ffff
#define BITMASK_20 0x000fffff
#define BITMASK_21 0x001fffff
#define BITMASK_22 0x003fffff
#define BITMASK_23 0x007fffff
#define BITMASK_24 0x00ffffff
#define BITMASK_25 0x01ffffff
#define BITMASK_26 0x03ffffff
#define BITMASK_27 0x07ffffff
#define BITMASK_28 0x0fffffff
#define BITMASK_29 0x1fffffff
#define BITMASK_30 0x3fffffff
#define BITMASK_31 0x7fffffff

#define BITMASK_32 0x00000000ffffffff
#define BITMASK_33 0x00000001ffffffff
#define BITMASK_34 0x00000003ffffffff
#define BITMASK_35 0x00000007ffffffff
#define BITMASK_36 0x0000000fffffffff
#define BITMASK_37 0x0000001fffffffff
#define BITMASK_38 0x0000003fffffffff
#define BITMASK_39 0x0000007fffffffff
#define BITMASK_40 0x000000ffffffffff
#define BITMASK_41 0x000001ffffffffff
#define BITMASK_42 0x000003ffffffffff
#define BITMASK_43 0x000007ffffffffff
#define BITMASK_44 0x00000fffffffffff
#define BITMASK_45 0x00001fffffffffff
#define BITMASK_46 0x00003fffffffffff
#define BITMASK_47 0x00007fffffffffff
#define BITMASK_48 0x0000ffffffffffff
#define BITMASK_49 0x0001ffffffffffff
#define BITMASK_50 0x0003ffffffffffff
#define BITMASK_51 0x0007ffffffffffff
#define BITMASK_52 0x000fffffffffffff
#define BITMASK_53 0x001fffffffffffff
#define BITMASK_54 0x003fffffffffffff
#define BITMASK_55 0x007fffffffffffff
#define BITMASK_56 0x00ffffffffffffff
#define BITMASK_57 0x01ffffffffffffff
#define BITMASK_58 0x03ffffffffffffff
#define BITMASK_59 0x07ffffffffffffff
#define BITMASK_60 0x0fffffffffffffff
#define BITMASK_61 0x1fffffffffffffff
#define BITMASK_62 0x3fffffffffffffff
#define BITMASK_63 0x7fffffffffffffff

#ifdef OS_WIN32
#define OS_FILE_DELIMTER "\\"
#else
#define OS_FILE_DELIMTER "/"
#endif

uint64_t Align_U64(uint64_t Value, uint64_t Alignment);
int64_t  Align_S64(int64_t Value,  int64_t Alignment);
uint64_t Ceil_Pow2_U64(uint64_t V);
int64_t  Ceil_Pow2_S64(int64_t V);
bool8_t Equal_Zero_Approx_F32(float V,  float Eps);
bool8_t Equal_Zero_Approx_F64(double V, double Eps);
bool8_t Equal_Approx_F32(float A, float B, float Eps);
bool8_t Equal_Approx_F64(double A, double B, double Eps);
bool8_t Equal_Zero_Eps_F32(float V);
bool8_t Equal_Zero_Eps_F64(double V);
bool8_t Equal_Eps_F32(float A, float B);
bool8_t Equal_Eps_F64(double A, double B);
float Safe_Ratio_F32(float A, float B);
double Safe_Ratio_F64(double A, double B);
uint8_t Safe_U16_U8(uint16_t V);
uint8_t Safe_U32_U8(uint32_t V);
uint8_t Safe_U64_U8(uint64_t V);
uint8_t Safe_S8_U8(int8_t V);
uint8_t Safe_S16_U8(int16_t V);
uint8_t Safe_S32_U8(int32_t V);
uint8_t Safe_S64_U8(int64_t V);
uint8_t Safe_F32_U8(float V);
uint8_t Safe_F64_U8(double V);
uint16_t Safe_U32_U16(uint32_t V);
uint16_t Safe_U64_U16(uint64_t V);
uint16_t Safe_S8_U16(int8_t V);
uint16_t Safe_S16_U16(int16_t V);
uint16_t Safe_S32_U16(int32_t V);
uint16_t Safe_S64_U16(int64_t V);
uint16_t Safe_F32_U16(float V);
uint16_t Safe_F64_U16(double V);
uint32_t Safe_U64_U32(uint64_t V);
uint32_t Safe_S8_U32(int8_t V);
uint32_t Safe_S16_U32(int16_t V);
uint32_t Safe_S32_U32(int32_t V);
uint32_t Safe_S64_U32(int64_t V);
uint32_t Safe_F32_U32(float V);
uint32_t Safe_F64_U32(double V);
uint64_t Safe_S8_U64(int8_t V);
uint64_t Safe_S16_U64(int16_t V);
uint64_t Safe_S32_U64(int32_t V);
uint64_t Safe_S64_U64(int64_t V);
uint64_t Safe_F32_U64(float V);
uint64_t Safe_F64_U64(double V);
int8_t Safe_U8_S8(uint8_t V);
int8_t Safe_U16_S8(uint16_t V);
int8_t Safe_U32_S8(uint32_t V);
int8_t Safe_U64_S8(uint64_t V);
int8_t Safe_S16_S8(int16_t V);
int8_t Safe_S32_S8(int32_t V);
int8_t Safe_S64_S8(int64_t V);
int16_t Safe_U16_S16(uint16_t V);
int16_t Safe_U32_S16(uint32_t V);
int16_t Safe_U64_S16(uint64_t V);
int16_t Safe_S32_S16(int32_t V);
int16_t Safe_S64_S16(int64_t V);
int32_t Safe_U32_S32(uint32_t V);
int32_t Safe_U64_S32(uint64_t V);
int32_t Safe_S64_S32(int64_t V);
int64_t Safe_U64_S64(uint64_t V);
float  Sqrt_F32(float V);
double Sqrt_F64(double V);
float  Sin_F32(float V);
double Sin_F64(double B);
float  Cos_F32(float V);
double Cos_F64(double B);
float  Tan_F32(float V);
double Tan_F64(double B);
float  ASin_F32(float V);
double ASin_F64(double B);
float  ACos_F32(float V);
double ACos_F64(double B);
float  ATan_F32(float V);
double ATan_F64(double B);
float  ATan2_F32(float Y, float X);
double ATan2_F64(double Y, double X);
float  Pow_F32(float V, float Exp); 
double Pow_F64(double V, double Exp);
float  Floor_F32(float V);
double Floor_F64(double V);
float  Ceil_F32(float V);
double Ceil_F64(double V);
float  Round_F32(float V);
double Round_F64(double V);
float  Lerp_F32(float a, float t, float b);
double Lerp_F64(double a, double t, double b);
char     To_UpperC(char C);
uint8_t  To_Upper8(uint8_t C);
uint16_t To_Upper16(uint16_t C);
uint32_t To_Upper32(uint32_t C);
char     To_LowerC(char C);
uint8_t  To_Lower8(uint8_t C);
uint16_t To_Lower16(uint16_t C);
uint32_t To_Lower32(uint32_t C);
int32_t Atomic_Increment32(volatile int32_t* Addend);
int64_t Atomic_Increment64(volatile int64_t* Addend);
int32_t Atomic_Decrement32(volatile int32_t* Addend);
int64_t Atomic_Decrement64(volatile int64_t* Addend);
int32_t Atomic_Add32(volatile int32_t* Addend, int32_t Value);
int64_t Atomic_Add64(volatile int64_t* Addend, int64_t Value);
int32_t Atomic_Exchange32(volatile int32_t* Dst, int32_t Value);
int64_t Atomic_Exchange64(volatile int64_t* Dst, int64_t Value);
void*   Atomic_ExchangePtr(void* volatile* Dst, void* Value);
int32_t Atomic_Compare_Exchange32(volatile int32_t* Dst, int32_t Exchange, int32_t Comperand);
int64_t Atomic_Compare_Exchange64(volatile int64_t* Dst, int64_t Exchange, int64_t Comperand);
void*   Atomic_Compare_ExchangePtr(void* volatile* Dst, void* Exchange, void* Comperand);

#endif