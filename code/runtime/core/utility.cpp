u32 Ceil_Pow2(u32 V) {
    V--;
    V |= V >> 1;
    V |= V >> 2;
    V |= V >> 4;
    V |= V >> 8;
    V |= V >> 16;
    V++;
    return V;
}

u64 Ceil_Pow2(u64 V) {
    V--;
    V |= V >> 1;
    V |= V >> 2;
    V |= V >> 4;
    V |= V >> 8;
    V |= V >> 16;
    V |= V >> 32;
    V++;
    return V;
}

char To_Lower(char C) {
    return (char)tolower(C);
}

char To_Upper(char C) {
    return (char)toupper(C);
}

wchar_t To_Lower(wchar_t C) {
    return (wchar_t)tolower(C);
}

wchar_t To_Upper(wchar_t C) {
    return (wchar_t)toupper(C);
}

bool Is_Whitespace(char C) {
    return isspace(C) != 0;
}

u32 Safe_U32(u64 x) {
    Assert(x <= 0xFFFFFFFF);
    return (u32)x;
}

f32 Safe_Ratio(u32 w, u32 h) {
    Assert(h != 0);
    return (f32)w/(f32)h;
}

f32 SNorm(s16 Value) {
    return Clamp(-1.0f, (f32)Value / (f32)((1 << 15) - 1), 1.0f);
}

s16 SNorm_S16(f32 Value) {
    s16 Result = (s16)(Clamp(-1.0f, Value, 1.0f) * (f32)(1 << 15));
    return Result;
}

u32 Ceil_U32(f64 V) {
    return (u32)ceil(V);
}

s32 Ceil_S32(f64 V) {
    return (s32)ceil(V);
}

u32 Floor_U32(f64 V) {
    return (u32)floor(V);
}

s32 Floor_S32(f64 V) {
    return (s32)floor(V);
}

u32 Ceil_U32(f32 V) {
    return (u32)ceilf(V);
}

s32 Ceil_S32(f32 V) {
    return (s32)ceilf(V);
}

u32 Floor_U32(f32 V) {
    return (u32)floorf(V);
}

s32 Floor_S32(f32 V) {
    return (s32)floorf(V);
}

bool Equal_Zero_Approx(f32 Value, f32 Epsilon) {
    return Abs(Value) <= Epsilon;
}

bool Equal_Zero_Eps(f32 Value) {
    return Equal_Zero_Approx(Value, FLT_EPSILON);
}

bool Equal_Zero_Eps_Sq(f32 SqValue) {
    return Equal_Zero_Approx(SqValue, Sq(FLT_EPSILON));
}

f32 Sqrt(f32 Value) {
    return sqrtf(Value);
}

f32 Pow(f32 X, f32 Y) {
    return powf(X, Y);
}

f32 ACos(f32 Angle) {
    return acosf(Angle);
}

f32 Cos(f32 Angle) {
    return cosf(Angle);
}

f32 Sin(f32 Angle) {
    return sinf(Angle);
}

f32 Tan(f32 Angle) {
    return tanf(Angle);
}

f32 Exp(f32 Value) {
    return expf(Value);
}

bool Is_Nan(f32 Value) {
    return isnan(Value);
}

bool Is_Finite(f32 Value) {
    return isfinite(Value);
}

s32 Random_Between(s32 Min, s32 Max) {
    s32 n = Max - Min + 1;
    s32 remainder = RAND_MAX % n;
    s32 x;
    do{
        x = rand();
    } while (x >= RAND_MAX - remainder);
    return Min + x % n;
}