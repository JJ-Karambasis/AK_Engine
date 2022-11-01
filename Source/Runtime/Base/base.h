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

#define Abs(a) (((a) < 0) ? -(a) : (a))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Min(a, b) (((a) < (b)) ? (a) : (b))

#define Static_Assert(c) char Glue(__Ignore__Value__, LINE_NUMBER)[(c) ? 1 : -1]

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