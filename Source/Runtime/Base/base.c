inline uint64_t Align_U64(uint64_t Value, uint64_t Alignment)
{
    Alignment = Alignment ? Alignment : 1;
    uint64_t Mod = Value & (Alignment-1);
    return Mod ? Value + (Alignment-Mod) : Value;
}

inline int64_t Align_S64(int64_t Value, int64_t Alignment)
{
    Alignment = Alignment ? Alignment : 1;
    int64_t Mod = Value & (Alignment-1);
    return Mod ? Value + (Alignment-Mod) : Value;
}

uint32_t Ceil_Pow2_U32(uint32_t V)
{
    --V;
    V |= V >> 1;
    V |= V >> 2;
    V |= V >> 4;
    V |= V >> 8;
    V |= V >> 16;
    ++V;
    V += ( V == 0 );    
    return V;
}

int32_t Ceil_Pow2_S32(int32_t V)
{
    --V;
    V |= V >> 1;
    V |= V >> 2;
    V |= V >> 4;
    V |= V >> 8;
    V |= V >> 16;
    ++V;
    V += ( V == 0 );    
    return V;
}

uint64_t Ceil_Pow2_U64(uint64_t V)
{
    --V;
    V |= V >> 1;
    V |= V >> 2;
    V |= V >> 4;
    V |= V >> 8;
    V |= V >> 16;
    V |= V >> 32;
    ++V;
    V += ( V == 0 );    
    return V;
}

int64_t  Ceil_Pow2_S64(int64_t V)
{
    --V;
    V |= V >> 1;
    V |= V >> 2;
    V |= V >> 4;
    V |= V >> 8;
    V |= V >> 16;
    V |= V >> 32;
    ++V;
    V += ( V == 0 );    
    return V;
}

inline bool8_t Equal_Zero_Approx_F32(float V,  float Eps)
{
    return Abs(V) < Eps;
}

inline bool8_t Equal_Zero_Approx_F64(double V, double Eps)
{
    return Abs(V) < Eps;
}

inline bool8_t Equal_Approx_F32(float A, float B, float Eps)
{
    return Equal_Zero_Approx_F32(A-B, Eps);
}

inline bool8_t Equal_Approx_F64(double A, double B, double Eps)
{
    return Equal_Zero_Approx_F64(A-B, Eps);
}

inline bool8_t Equal_Zero_Eps_F32(float V)
{
    return Equal_Zero_Approx_F32(V, EPSILON_F32);
}

inline bool8_t Equal_Zero_Eps_F64(double V)
{
    return Equal_Zero_Approx_F64(V, EPSILON_F64);
}

inline bool8_t Equal_Eps_F32(float A, float B)
{
    return Equal_Approx_F32(A, B, EPSILON_F32);
}

inline bool8_t Equal_Eps_F64(double A, double B)
{
    return Equal_Approx_F64(A, B, EPSILON_F64);
}

inline float Safe_Ratio_F32(float A, float B)
{
    Assert(!Equal_Zero_Eps_F32(B));
    return A/B;
}

inline double Safe_Ratio_F64(double A, double B)
{
    Assert(!Equal_Zero_Eps_F64(B));
    return A/B;
}

inline uint8_t Safe_U16_U8(uint16_t V)
{
    Assert(V <= MAX_U8);
    return (uint8_t)V;
}

inline uint8_t Safe_U32_U8(uint32_t V)
{
    Assert(V <= MAX_U8);
    return (uint8_t)V;
}

inline uint8_t Safe_U64_U8(uint64_t V)
{
    Assert(V <= MAX_U8);
    return (uint8_t)V;
}

inline uint8_t Safe_S8_U8(int8_t V)
{
    Assert(V >= 0);
    return (uint8_t)V;
}

inline uint8_t Safe_S16_U8(int16_t V)
{
    Assert(V >= 0 && V <= MAX_U8);
    return (uint8_t)V;
}

inline uint8_t Safe_S32_U8(int32_t V)
{
    Assert(V >= 0 && V <= MAX_U8);
    return (uint8_t)V;
}

inline uint8_t Safe_S64_U8(int64_t V)
{
    Assert(V >= 0 && V <= MAX_U8);
    return (uint8_t)V;
}

inline uint8_t Safe_F32_U8(float V)
{
    Assert(V >= 0 && V <= (float)MAX_U8);
    return (uint8_t)V;
}

inline uint8_t Safe_F64_U8(double V)
{
    Assert(V >= 0 && V <= (double)MAX_U8);
    return (uint8_t)V;
}

inline uint16_t Safe_U32_U16(uint32_t V)
{
    Assert(V <= MAX_U16);
    return (uint16_t)V;
}

inline uint16_t Safe_U64_U16(uint64_t V)
{
    Assert(V <= MAX_U16);
    return (uint16_t)V;
}

inline uint16_t Safe_S8_U16(int8_t V)
{
    Assert(V >= 0);
    return (uint16_t)V;
}

inline uint16_t Safe_S16_U16(int16_t V)
{
    Assert(V >= 0);
    return (uint16_t)V;
}

inline uint16_t Safe_S32_U16(int32_t V)
{
    Assert(V >= 0 && V < MAX_U16);
    return (uint16_t)V;
}

inline uint16_t Safe_S64_U16(int64_t V)
{
    Assert(V >= 0 && V < MAX_U16);
    return (uint16_t)V;
}

inline uint16_t Safe_F32_U16(float V)
{
    Assert(V >= 0 && V <= (float)MAX_U16);
    return (uint8_t)V;
}

inline uint16_t Safe_F64_U16(double V)
{
    Assert(V >= 0 && V <= (double)MAX_U16);
    return (uint8_t)V;
}

inline uint32_t Safe_U64_U32(uint64_t V)
{
    Assert(V <= MAX_U32);
    return (uint32_t)V;
}

inline uint32_t Safe_S8_U32(int8_t V)
{
    Assert(V >= 0);
    return (uint32_t)V;
}

inline uint32_t Safe_S16_U32(int16_t V)
{
    Assert(V >= 0);
    return (uint32_t)V;
}

inline uint32_t Safe_S32_U32(int32_t V)
{
    Assert(V >= 0);
    return (uint32_t)V;
}

inline uint32_t Safe_S64_U32(int64_t V)
{
    Assert(V >= 0 && V <= MAX_U32);
    return (uint32_t)V;
}

inline uint32_t Safe_F32_U32(float V)
{
    Assert(V >= V && V <= (float)MAX_U32);
    return (uint32_t)V;
}

inline uint32_t Safe_F64_U32(double V)
{
    Assert(V >= V && V <= (double)MAX_U32);
    return (uint32_t)V;
}

inline uint64_t Safe_S8_U64(int8_t V)
{
    Assert(V >= 0);
    return (uint64_t)V;
}

inline uint64_t Safe_S16_U64(int16_t V)
{
    Assert(V >= 0);
    return (uint64_t)V;
}

inline uint64_t Safe_S32_U64(int32_t V)
{
    Assert(V >= 0);
    return (uint64_t)V;
}

inline uint64_t Safe_S64_U64(int64_t V)
{
    Assert(V >= 0);
    return (uint64_t)V;
}

inline uint64_t Safe_F32_U64(float V)
{
    Assert(V >= 0);
    return (uint64_t)V;
}

inline uint64_t Safe_F64_U64(double V)
{
    Assert(V >= 0);
    return (uint64_t)V;
}

inline int8_t Safe_U8_S8(uint8_t V)
{
    Assert(V <= MAX_S8);
    return (int8_t)V;
}

inline int8_t Safe_U16_S8(uint16_t V)
{
    Assert(V <= MAX_S8);
    return (int8_t)V;
}

inline int8_t Safe_U32_S8(uint32_t V)
{
    Assert(V <= MAX_S8);
    return (int8_t)V;
}

inline int8_t Safe_U64_S8(uint64_t V)
{
    Assert(V <= MAX_S8);
    return (int8_t)V;
}

inline int8_t Safe_S16_S8(int16_t V)
{
    Assert(V >= MIN_S8 && V <= MAX_S8);
    return (int8_t)V;
}

inline int8_t Safe_S32_S8(int32_t V)
{
    Assert(V >= MIN_S8 && V <= MAX_S8);
    return (int8_t)V;
}

inline int8_t Safe_S64_S8(int64_t V)
{
    Assert(V >= MIN_S8 && V <= MAX_S8);
    return (int8_t)V;
}

inline int16_t Safe_U16_S16(uint16_t V)
{
    Assert(V <= MAX_S16);
    return (int16_t)V;
}

inline int16_t Safe_U32_S16(uint32_t V)
{
    Assert(V <= MAX_S16);
    return (int16_t)V;
}

inline int16_t Safe_U64_S16(uint64_t V)
{
    Assert(V <= MAX_S16);
    return (int16_t)V;
}

inline int16_t Safe_S32_S16(int32_t V)
{
    Assert(V >= MIN_S16 && V <= MAX_S16);
    return (int16_t)V;
}

inline int16_t Safe_S64_S16(int64_t V)
{
    Assert(V >= MIN_S16 && V <= MAX_S16);
    return (int16_t)V;
}

inline int32_t Safe_U32_S32(uint32_t V)
{
    Assert(V <= MAX_S32);
    return (int32_t)V;
}

inline int32_t Safe_U64_S32(uint64_t V)
{
    Assert(V <= MAX_S32);
    return (int32_t)V;
}

inline int32_t Safe_S64_S32(int64_t V)
{
    Assert(V >= MIN_S32 && V <= MAX_S32);
    return (int32_t)V;
}

inline int64_t Safe_U64_S64(uint64_t V)
{
    Assert(V <= MAX_S64);
    return (uint64_t)V;
}

inline float Sqrt_F32(float V)
{
    return sqrtf(V);
}

inline double Sqrt_F64(double V)
{
    return sqrt(V);
}

inline float  Sin_F32(float V)
{
    return sinf(V);
}

inline double Sin_F64(double V)
{
    return sin(V);
}

inline float  Cos_F32(float V)
{
    return cosf(V);
}

inline double Cos_F64(double V)
{
    return cos(V);
}

inline float Tan_F32(float V)
{
    return tanf(V);
}

inline double Tan_F64(double V)
{
    return tan(V);
}

inline float  ASin_F32(float V)
{
    return asinf(V);
}

inline double ASin_F64(double V)
{
    return asin(V);
}

inline float  ACos_F32(float V)
{
    return acosf(V);
}

inline double ACos_F64(double V)
{
    return acos(V);
}

inline float ATan_F32(float V)
{
    return atanf(V);
}

inline double ATan_F64(double V)
{
    return atan(V);
}

inline float ATan2_F32(float Y, float X)
{
    return atan2f(Y, X);
}

inline double ATan2_F64(double Y, double X)
{
    return atan2(Y, X);
}

inline float Pow_F32(float V, float Exp)
{
    return powf(V, Exp);
}

inline double Pow_F64(double V, double Exp)
{
    return pow(V, Exp);
}

inline float Floor_F32(float V)
{
    return floorf(V);
}

inline double Floor_F64(double V)
{
    return floor(V);
}

inline float Ceil_F32(float V)
{
    return ceilf(V);
}

inline double Ceil_F64(double V)
{
    return ceil(V);
}

inline float  Round_F32(float V)
{
    return roundf(V);
}

inline double Round_F64(double V)
{
    return round(V);
}

float Lerp_F32(float a, float t, float b)
{
    return a + t*(b-a);
}

double Lerp_F64(double a, double t, double b)
{
    return a + t*(b-a);
}

bool8_t Is_WhitespaceC(char C)
{
    return (C == ' ' || C == '\n' || C == '\r' || C == '\t' || C == '\f' || C == '\v');
}

bool8_t Is_Whitespace8(uint8_t C)
{
    return (C == ' ' || C == '\n' || C == '\r' || C == '\t' || C == '\f' || C == '\v');
}

bool8_t Is_Whitespace16(uint16_t C)
{
    return (C == ' ' || C == '\n' || C == '\r' || C == '\t' || C == '\f' || C == '\v');
}

bool8_t Is_Whitespace32(uint32_t C)
{
    return (C == ' ' || C == '\n' || C == '\r' || C == '\t' || C == '\f' || C == '\v');
}

inline char To_UpperC(char C)
{
    if (('a' <= C) && (C <= 'z'))
        C -= 'a' - 'A';
    return C;
}

inline uint8_t To_Upper8(uint8_t C)
{
    if (('a' <= C) && (C <= 'z'))
        C -= 'a' - 'A';
    return C;
}

inline uint16_t To_Upper16(uint16_t C)
{
    if (('a' <= C) && (C <= 'z'))
        C -= 'a' - 'A';
    return C;
}

inline uint32_t To_Upper32(uint32_t C)
{
    if (('a' <= C) && (C <= 'z'))
        C -= 'a' - 'A';
    return C;
}

inline char To_LowerC(char C)
{
    if (('A' <= C) && (C <= 'Z'))
        C+= 'a' - 'A';
    return C;
}

inline uint8_t To_Lower8(uint8_t C)
{
    if (('A' <= C) && (C <= 'Z'))
        C+= 'a' - 'A';
    return C;
}

inline uint16_t To_Lower16(uint16_t C)
{
    if (('A' <= C) && (C <= 'Z'))
        C+= 'a' - 'A';
    return C;
}

inline uint32_t To_Lower32(uint32_t C)
{
    if (('A' <= C) && (C <= 'Z'))
        C+= 'a' - 'A';
    return C;
}

#if defined(COMPILER_MSVC) || defined(COMPILER_CLANG)
inline int32_t Atomic_Increment32(volatile int32_t* Addend)
{
    int32_t Result = _InterlockedIncrement((volatile long*)Addend);
    return Result;
}

inline int64_t Atomic_Increment64(volatile int64_t* Addend)
{
    int64_t Result = _InterlockedIncrement64(Addend);
    return Result;
}

inline int32_t Atomic_Decrement32(volatile int32_t* Addend)
{
    int32_t Result = _InterlockedDecrement((volatile long*)Addend);
    return Result;
}

inline int64_t Atomic_Decrement64(volatile int64_t* Addend)
{
    int64_t Result = _InterlockedDecrement64(Addend);
    return Result;
}

inline int32_t Atomic_Add32(volatile int32_t* Addend, int32_t Value)
{
    int32_t Result = _InterlockedExchangeAdd((volatile long*)Addend, Value);
    return Result;
}

inline int64_t Atomic_Add64(volatile int64_t* Addend, int64_t Value)
{
    int64_t Result = _InterlockedExchangeAdd64(Addend, Value);
    return Result;
}

inline int32_t Atomic_Exchange32(volatile int32_t* Dst, int32_t Value)
{
    int32_t Result = _InterlockedExchange((volatile long*)Dst, Value);
    return Result;
}

inline int64_t Atomic_Exchange64(volatile int64_t* Dst, int64_t Value)
{
    int64_t Result = _InterlockedExchange64(Dst, Value);
    return Result;
}

inline void* Atomic_ExchangePtr(void* volatile* Dst, void* Value)
{
    void* Result = _InterlockedExchangePointer(Dst, Value);
    return Result;
}

inline int32_t Atomic_Compare_Exchange32(volatile int32_t* Dst, int32_t Exchange, int32_t Comperand)
{
    int32_t Result = _InterlockedCompareExchange((volatile long*)Dst, Exchange, Comperand);
    return Result;
}

inline int64_t Atomic_Compare_Exchange64(volatile int64_t* Dst, int64_t Exchange, int64_t Comperand)
{
    int64_t Result = _InterlockedCompareExchange64(Dst, Exchange, Comperand);
    return Result;
}

inline void* Atomic_Compare_ExchangePtr(void* volatile* Dst, void* Exchange, void* Comperand)
{
    void* Result = _InterlockedCompareExchangePointer(Dst, Exchange, Comperand);
    return Result;
}
#else
#error Not Implemented
#endif

uint32_t Hash_U32(uint32_t Key)
{
    Key = (Key+0x7ed55d16) + (Key<<12);
    Key = (Key^0xc761c23c) ^ (Key>>19);
    Key = (Key+0x165667b1) + (Key<<5);
    Key = (Key+0xd3a2646c) ^ (Key<<9);
    Key = (Key+0xfd7046c5) + (Key<<3);
    Key = (Key^0xb55a4f09) ^ (Key>>16);
    return Key;
}

uint32_t Hash_U64(uint64_t Key)
{
    Key = (~Key) + (Key << 18); // Key = (Key << 18) - Key - 1;
    Key = Key ^ (Key >> 31);
    Key = Key * 21; // Key = (Key + (Key << 2)) + (Key << 4);
    Key = Key ^ (Key >> 11);
    Key = Key + (Key << 6);
    Key = Key ^ (Key >> 22);
    return (uint32_t)Key;
}

uint32_t Hash_Ptr(size_t Key)
{
#ifdef BITNESS_32
    return Hash_U32(Key);
#else
    return Hash_U64(Key);
#endif
}

uint32_t Hash_Combine(uint32_t HashA, uint32_t HashB)
{
    uint32_t Result = HashB + 0x9e3779b9 + (HashA << 6) + (HashA >> 2);
    return Result;
}