matrix4 Transpose(const matrix4& M) {
    matrix4 Result;

    Result.m00 = M.m00;
    Result.m10 = M.m01;
    Result.m20 = M.m02;
    Result.m30 = M.m03;

    Result.m01 = M.m10;
    Result.m11 = M.m11;
    Result.m21 = M.m12;
    Result.m31 = M.m13;

    Result.m02 = M.m20;
    Result.m12 = M.m21;
    Result.m22 = M.m22;
    Result.m32 = M.m23;

    Result.m03 = M.m30;
    Result.m13 = M.m31;
    Result.m23 = M.m32;
    Result.m33 = M.m33;

    return Result;
}

matrix4 Ortho_Projection2D(f32 L, f32 R, f32 T, f32 B) {
    return {
        2.0f/(R-L),  0.0f,        0.0f, 0.0f, 
        0.0f,        2.0f/(T-B), 0.0f, 0.0f, 
        0.0f,        0.0f,        0.5f, 0.0f, 
        (R+L)/(L-R), (T+B)/(B-T), 0.5f, 1.0f
    };
}

matrix4 Ortho_Projection2D(f32 Width, f32 Height) {
    f32 L = 0.0f;
    f32 R = L+Width;
    f32 T = 0.0f;
    f32 B = T+Height;
    return Ortho_Projection2D(L, R, T, B);
}