#ifndef UTILITY_H
#define UTILITY_H

struct range_u32 {
    u32 Min;
    u32 Max;
};

struct range_u64 {
    u64 Min;
    u64 Max;
};

u32 Ceil_Pow2(u32 V);
u64 Ceil_Pow2(u64 V);
char To_Lower(char C);
char To_Upper(char C);
wchar_t To_Lower(wchar_t C);
wchar_t To_Upper(wchar_t C);
bool Is_Whitespace(char C);
u32 Safe_U32(u64 x);
f32 Safe_Ratio(u32 w, u32 h);
f32 SNorm(s16 Value);
s16 SNorm_S16(f32 Value);
u32 Ceil_U32(f64 V);
s32 Ceil_S32(f64 V);
u32 Floor_U32(f64 V);
s32 Floor_S32(f64 V);
f32 Floor_F32(f32 V);
u32 Ceil_U32(f32 V);
s32 Ceil_S32(f32 V);
u32 Floor_U32(f32 V);
s32 Floor_S32(f32 V);
u32 F32_To_U32(f32 V);
f32 U32_To_F32(u32 V);
bool Equal_Zero_Approx(f32 Value, f32 Epsilon);
bool Equal_Zero_Eps(f32 Value);
bool Equal_Zero_Eps_Sq(f32 SqValue);
f32 Sqrt(f32 Value);
f32 Pow(f32 X, f32 Y);
f32 ACos(f32 Angle);
f32 Cos(f32 Angle);
f32 Sin(f32 Angle);
f32 Tan(f32 Angle);
f32 Exp(f32 Value);
bool Is_Nan(f32 Value);
bool Is_Finite(f32 Value);
s32 Random_Between(s32 Min, s32 Max);
u32 Align_U32(u32 Alignment, u32 Value);
bool In_Range(u32 Value, u32 Min, u32 Max);
bool In_Range(u64 Value, u64 Min, u64 Max);

struct scoped_mutex {
    ak_mutex* Mutex;
    scoped_mutex(ak_mutex* _Mutex);
    ~scoped_mutex();
};

template <typename type> 
struct range_iter {
    type Current;
    
    inline range_iter(const type& _Current) : Current(_Current) { }
    inline type operator*() const { return Current; }
    inline type const* operator->() { return &Current; }
    inline range_iter& operator++() { Current++; return *this; }
    inline bool operator==(const range_iter& Other) const { return Current == Other.Current; }
    inline bool operator!=(const range_iter& Other) const { return Current != Other.Current; }
};

template <typename type> 
struct range_proxy {
    range_iter<type> Begin;
    range_iter<type> End;
    
    inline range_proxy(const type& _Begin, const type& _End) : Begin(_Begin), End(_End) { }
    inline range_iter<type> begin() const { return Begin; }
    inline range_iter<type> end() const { return End; }
};

template <typename type> 
inline auto Indices(const type& Type) -> range_proxy<decltype(Type.Count)> { return {0, Type.Count}; }

template <typename type, uptr N>
inline auto Indices(const type (&Arr)[N]) -> range_proxy<decltype(N)> { return {0, N}; }

#endif