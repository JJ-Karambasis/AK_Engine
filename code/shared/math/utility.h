#ifndef UTILITY_H
#define UTILITY_H

f32 Safe_Ratio(u32 w, u32 h);

bool Equal_Zero_Approx(f32 Value, f32 Epsilon);
bool Equal_Zero_Eps(f32 Value);
bool Equal_Zero_Eps_Sq(f32 SqValue);

f32 Floor(f32 V);
f64 Floor(f64 V);

u32 Floor_U32(f32 V);
u32 Floor_U32(f64 V);

s32 Floor_S32(f32 V);
s32 Floor_S32(f64 V);

f32 Ceil(f32 V);
f64 Ceil(f64 V);

u32 Ceil_U32(f32 V);
u32 Ceil_U32(f64 V);

s32 Ceil_S32(f32 V);
s32 Ceil_S32(f64 V);

f32 Round(f32 V);
f64 Round(f64 V);

u32 Round_U32(f32 V);
u32 Round_U32(f64 V);

s32 Round_S32(f32 V);
s32 Round_S32(f64 V);

f32 Sin(f32 Value);
f64 Sin(f64 Value);

f32 Cos(f32 Value);
f64 Cos(f64 Value);

f32 Tan(f32 Value);
f64 Tan(f64 Value);

f32 ACos(f32 Value);
f64 ACos(f64 Value);

f32 Sqrt(f32 Value);
f64 Sqrt(f64 Value);

f32 Pow(f32 X, f32 Y);
f64 Pow(f64 X, f64 Y);

bool In_Range(u32 Value, u32 Min, u32 Max);
bool In_Range(u64 Value, u64 Min, u64 Max);

#endif