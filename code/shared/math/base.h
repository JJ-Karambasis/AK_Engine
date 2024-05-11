#ifndef BASE_H
#define BASE_H

template <u32 Count, u32 XIndex, u32 YIndex>
struct f32_2x_sub {
    f32 Data[Count];
};

template <u32 Count, u32 XIndex, u32 YIndex, u32 ZIndex>
struct f32_3x_sub {
    f32 Data[Count];
};

template <u32 Count, u32 XIndex, u32 YIndex, u32 ZIndex, u32 WIndex>
struct f32_4x_sub {
    f32 Data[Count];
};

template <u32 Count, u32 XIndex, u32 YIndex>
struct s32_2x_sub {
    s32 Data[Count];
};

template <u32 Count, u32 XIndex, u32 YIndex, u32 ZIndex>
struct s32_3x_sub {
    s32 Data[Count];
};

template <u32 Count, u32 XIndex, u32 YIndex, u32 ZIndex, u32 WIndex>
struct s32_4x_sub {
    s32 Data[Count];
};

template <u32 Count, u32 XIndex, u32 YIndex>
struct u32_2x_sub {
    u32 Data[Count];
};

template <u32 Count, u32 XIndex, u32 YIndex, u32 ZIndex>
struct u32_3x_sub {
    u32 Data[Count];
};

template <u32 Count, u32 XIndex, u32 YIndex, u32 ZIndex, u32 WIndex>
struct u32_4x_sub {
    u32 Data[Count];
};

#endif