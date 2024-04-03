bool operator!=(uvec2 A, uvec2 B) {
    return A.x != B.x || A.y != B.y;
}

vec3::vec3(f32 _x, f32 _y, f32 _z) {
    x = _x;
    y = _y;
    z = _z;
}

f32 Vec3_Dot(vec3 A, vec3 B) {
    return A.x*B.x + A.y*B.y + A.z*B.z;
}

vec3 Vec3_Cross(vec3 A, vec3 B) {
    return vec3(A.y*B.z - A.z*B.y, A.z*B.x - A.x*B.z, A.x*B.y - A.y*B.x);
}

vec3 operator+(vec3 A, vec3 B) {
    return vec3(A.x+B.x, A.y+B.y, A.z+B.z);
}

vec3 operator*(vec3 A, f32 B) {
    return vec3(A.x*B, A.y*B, A.z*B);
}

vec4::vec4(f32 _x, f32 _y, f32 _z, f32 _w) {
    x = _x;
    y = _y;
    z = _z;
    w = _w;
}

quat::quat(f32 _x, f32 _y, f32 _z, f32 _w) {
    x = _x;
    y = _y;
    z = _z;
    w = _w;
}

quat::quat(vec3 _v, f32 _s) {
    v = _v;
    s = _s;
}

quat Quat_RotX(f32 Angle) {
    Angle *= 0.5f;
    return quat(Sin(Angle), 0.0f, 0.0f, Cos(Angle));
}

quat Quat_RotY(f32 Angle) {
    Angle *= 0.5f;
    return quat(0.0f, Sin(Angle), 0.0f, Cos(Angle));
}

quat Quat_RotZ(f32 Angle) {
    Angle *= 0.5f;
    return quat(0.0f, 0.0f, Sin(Angle), Cos(Angle));
}

f32 Quat_Dot(quat A, quat B) {
    return A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;
}

f32 Quat_Sq_Mag(quat Q) {
    return Quat_Dot(Q, Q);
}

quat Quat_Normalize(quat Q) {
    f32 SqLength = Quat_Sq_Mag(Q);
    if(Equal_Zero_Eps_Sq(SqLength)) {
        return quat();
    }

    f32 InvLength = 1.0f/Sqrt(SqLength);
    return Q * InvLength;
}

quat operator*(quat A, f32 B) {
    return quat(A.x*B, A.y*B, A.z*B, A.w*B);
}

quat operator*(quat A, quat B) {
    return quat(Vec3_Cross(A.v, B.v) + A.v*B.s + B.v*A.s, A.s*B.s - Vec3_Dot(A.v, B.v));
}

matrix3::matrix3(quat Q) {
    f32 qxqy = Q.x*Q.y;
    f32 qwqz = Q.w*Q.z;
    f32 qxqz = Q.x*Q.z;
    f32 qwqy = Q.w*Q.y;
    f32 qyqz = Q.y*Q.z;
    f32 qwqx = Q.w*Q.x;
    
    f32 qxqx = Q.x*Q.x;
    f32 qyqy = Q.y*Q.y;
    f32 qzqz = Q.z*Q.z;

    Rows[0] = vec3(1 - 2*(qyqy+qzqz), 2*(qxqy+qwqz),     2*(qxqz-qwqy));
    Rows[1] = vec3(2*(qxqy-qwqz),     1 - 2*(qxqx+qzqz), 2*(qyqz+qwqx)); 
    Rows[2] = vec3(2*(qxqz+qwqy),     2*(qyqz-qwqx),     1 - 2*(qxqx+qyqy));
}

matrix4_affine::matrix4_affine(std::initializer_list<f32> List) {
    Assert(List.size() <= 12);
    Memory_Copy(Data, List.begin(), List.size()*sizeof(f32));
}

void Matrix4_Affine_Translation(matrix4_affine* Result, vec3 Translation) {
    *Result = matrix4_affine();
    Result->t = Translation;
}

void Matrix4_Affine_Transpose(matrix4_affine* Result, const matrix4_affine& M) {
    Result->Data[0]  = M.m00;
    Result->Data[1]  = M.m10;
    Result->Data[2]  = M.m20;
    Result->Data[3]  = M.m30;

    Result->Data[4]  = M.m01;
    Result->Data[5]  = M.m11;
    Result->Data[6]  = M.m21;
    Result->Data[7]  = M.m31;
    
    Result->Data[8]  = M.m02;
    Result->Data[9]  = M.m12;
    Result->Data[10] = M.m22;
    Result->Data[11] = M.m32;
}

void Matrix4_Affine_Inverse_No_Scale(matrix4_affine* Result, vec3 P, const matrix3& M) {
    f32 tx = -Vec3_Dot(P, M.x);
    f32 ty = -Vec3_Dot(P, M.y);
    f32 tz = -Vec3_Dot(P, M.z);

    *Result = {
        M.m00, M.m10, M.m20,
        M.m01, M.m11, M.m21,
        M.m02, M.m12, M.m22,
        tx, ty, tz
    };
}

void Matrix4_Zero(matrix4* M) {
    Zero_Struct(M);
}

void Matrix4_Transpose(matrix4* Result, const matrix4& M) {
    Result->Data[0]  = M.m00;
    Result->Data[1]  = M.m10;
    Result->Data[2]  = M.m20;
    Result->Data[3]  = M.m30;

    Result->Data[4]  = M.m01;
    Result->Data[5]  = M.m11;
    Result->Data[6]  = M.m21;
    Result->Data[7]  = M.m31;
    
    Result->Data[8]  = M.m02;
    Result->Data[9]  = M.m12;
    Result->Data[10] = M.m22;
    Result->Data[11] = M.m32;

    Result->Data[12] = M.m03;
    Result->Data[13] = M.m13;
    Result->Data[14] = M.m23;
    Result->Data[15] = M.m33;
}

void Matrix4_Perspective(matrix4* M, f32 AspectRatio, f32 FieldOfView, f32 NearPlane, f32 FarPlane) {
    Matrix4_Zero(M);

    f32 t = 1.0f/Tan(FieldOfView*0.5f);
    f32 n = NearPlane;
    f32 f = FarPlane;

    f32 a = t/AspectRatio;
    f32 b = t;
    f32 c = f/(n-f);
    f32 d = (n*f)/(n-f);
    f32 e = -1;

    M->Data[0]  = a;
    M->Data[5]  = b;
    M->Data[10] = c;
    M->Data[11] = e;
    M->Data[14] = d;
}

inline matrix4 operator*(const matrix4_affine& A, const matrix4& B) {
    matrix4 Result;

    matrix4 BTransposed;
    Matrix4_Transpose(&BTransposed, B);

    Result.Data[0]  = Vec3_Dot(A.Rows[0], BTransposed.Rows[0].xyz);
    Result.Data[1]  = Vec3_Dot(A.Rows[0], BTransposed.Rows[1].xyz);
    Result.Data[2]  = Vec3_Dot(A.Rows[0], BTransposed.Rows[2].xyz);
    Result.Data[3]  = Vec3_Dot(A.Rows[0], BTransposed.Rows[3].xyz);

    Result.Data[4]  = Vec3_Dot(A.Rows[1], BTransposed.Rows[0].xyz);
    Result.Data[5]  = Vec3_Dot(A.Rows[1], BTransposed.Rows[1].xyz);
    Result.Data[6]  = Vec3_Dot(A.Rows[1], BTransposed.Rows[2].xyz);
    Result.Data[7]  = Vec3_Dot(A.Rows[1], BTransposed.Rows[3].xyz);

    Result.Data[8]  = Vec3_Dot(A.Rows[2], BTransposed.Rows[0].xyz);
    Result.Data[9]  = Vec3_Dot(A.Rows[2], BTransposed.Rows[1].xyz);
    Result.Data[10] = Vec3_Dot(A.Rows[2], BTransposed.Rows[2].xyz);
    Result.Data[11] = Vec3_Dot(A.Rows[2], BTransposed.Rows[3].xyz);

    Result.Data[12] = Vec3_Dot(A.Rows[3], BTransposed.Rows[0].xyz) + BTransposed.Rows[0].w;
    Result.Data[13] = Vec3_Dot(A.Rows[3], BTransposed.Rows[1].xyz) + BTransposed.Rows[1].w;
    Result.Data[14] = Vec3_Dot(A.Rows[3], BTransposed.Rows[2].xyz) + BTransposed.Rows[2].w;
    Result.Data[15] = Vec3_Dot(A.Rows[3], BTransposed.Rows[3].xyz) + BTransposed.Rows[3].w;

    return Result;
}