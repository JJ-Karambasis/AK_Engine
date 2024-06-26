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

s32 Safe_S32(u32 X) {
    Assert(X <= INT32_MAX);
    return (s32)X;
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

f32 Floor_F32(f32 V) {
    return floorf(V);
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

u32 F32_To_U32(f32 V) {
    union {
        f32 F;
        u32 U;
    } u;
    u.F = V;
    return u.U;
}

f32 U32_To_F32(u32 V) {
    union {
        f32 F;
        u32 U;
    } u;

    u.U = V;
    return u.F;
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

s32 Align(s32 Alignment, s32 Value) {
    s32 Remainder = Value % Alignment;
    return Remainder ? Value + (Alignment-Remainder) : Value;
}

u32 Align(u32 Alignment, u32 Value) {
    u32 Remainder = Value % Alignment;
    return Remainder ? Value + (Alignment-Remainder) : Value;
}

u64 Align(u64 Alignment, u64 Value) {
    u64 Remainder = Value % Alignment;
    return Remainder ? Value + (Alignment-Remainder) : Value;
}

bool In_Range(u32 Value, u32 Min, u32 Max) {
    return Value >= Min && Value <= Max;
}

bool In_Range(u64 Value, u64 Min, u64 Max) {
    return Value >= Min && Value <= Max;
}

s64 Pack_S64(s32_2x Data) {
    return Pack_S64(Data[0], Data[1]);
}

s64 Pack_S64(s32 A, s32 B) {
    s64 PackedX = ((s64)A) & 0xFFFFFFFFLL;
    s64 PackedY = ((s64)B) & 0xFFFFFFFFLL;
    return (PackedX) | (PackedY << 32);
}

s32_2x Unpack_S64(s64 A) {
    return s32_2x((s32)A, (s32)(A >> 32));
}

scoped_mutex::scoped_mutex(ak_mutex* _Mutex) : Mutex(_Mutex) { 
    AK_Mutex_Lock(Mutex);
}

scoped_mutex::~scoped_mutex() {
    if(Mutex) {
        AK_Mutex_Unlock(Mutex);
        Mutex = NULL;
    }
}