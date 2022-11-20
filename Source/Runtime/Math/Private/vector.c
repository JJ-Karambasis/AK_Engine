v2 V2(float x, float y)
{
    v2 Result;
    Result.x = x;
    Result.y = y;
    return Result;
}

v2i V2i(int64_t x, int64_t y)
{
    v2i Result;
    Result.x = x;
    Result.y = y;
    return Result;
}

v3 V3(float x, float y, float z)
{
    v3 Result;
    Result.x = x;
    Result.y = y;
    Result.z = z;
    return Result;
}

v3i V3i(int64_t x, int64_t y, int64_t z)
{
    v3i Result;
    Result.x = x;
    Result.y = y;
    Result.z = z;
    return Result;
}

v4 V4(float x, float y, float z, float w)
{
    v4 Result;
    Result.x = x;
    Result.y = y;
    Result.z = z;
    Result.w = w;
    return Result;
}

v4i V4i(int64_t x, int64_t y, int64_t z, int64_t w)
{
    v4i Result;
    Result.x = x;
    Result.y = y;
    Result.z = z;
    Result.w = w;
    return Result;
}

v2 V2_Add_V2(v2 Left, v2 Right)
{
    v2 Result;
    Result.x = Left.x+Right.x;
    Result.y = Left.y+Right.y;
    return Result;
}

v2 V2_Mul_S(v2 V, float S)
{
    V.x *= S;
    V.y *= S;
    return V;
}

v2 S_Div_V2(float S, v2 V)
{
    Assert(!Equal_Zero_Eps_F32(V.x) && !Equal_Zero_Eps_F32(V.y));
    V.x = S/V.x;
    V.y = S/V.y;
    return V;
}

v2 V2_Inv(v2 V)
{
    return S_Div_V2(1.0f, V);
}