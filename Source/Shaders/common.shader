#ifdef SHADER_CODE
#define Cast(type, value) type(value)
#else
#define Cast(type, value) (type)(value)
#endif

uint32_t Color_V4_To_U32(v4 Color)
{
    uint32_t B = Cast(uint32_t, Floor_F32(Color.z >= 1.0f ? 255 : Color.z * 255.0f));
    uint32_t G = Cast(uint32_t, Floor_F32(Color.y >= 1.0f ? 255 : Color.y * 255.0f));
    uint32_t R = Cast(uint32_t, Floor_F32(Color.x >= 1.0f ? 255 : Color.x * 255.0f));
    uint32_t A = Cast(uint32_t, Floor_F32(Color.w >= 1.0f ? 255 : Color.w * 255.0f));
    return R | (G << 8) | (B << 16) | (A << 24);
}

v4 Color_U32_To_V4(uint32_t Color)
{
    v4 Result;
    Result.x = Cast(float, ((Color >> 0) & 0xFF)) / 255.0f;
    Result.y = Cast(float, ((Color >> 8) & 0xFF)) / 255.0f;
    Result.z = Cast(float, ((Color >> 16) & 0xFF)) / 255.0f;
    Result.w = Cast(float, ((Color >> 24) & 0xFF)) / 255.0f;
    return Result;
}