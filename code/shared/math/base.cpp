vec2::vec2(const vec2i& Vec) : x(Vec.x), y(Vec.y) { }
vec2::vec2(const point2& P) : Data(P.Data) { }

vec2i::vec2i(s32 _x, s32 _y) : x(_x), y(_y) { }
vec2i::vec2i(s32_2x _Data) : Data(_Data) { }

bool operator!=(const vec2i& A, const vec2i& B) {
    return A.Data != B.Data;
}

bool operator==(const vec2i& A, const vec2i& B) {
    return A.Data == B.Data;
}

vec2i operator+(const vec2i& A, const vec2i& B) {
    return A.Data + B.Data;
}

point2::point2(const point2i& P) : x(P.x), y(P.y) {}

point2 operator+(const point2& A, const vec2& B) {
    point2 Result;
    Result.Data = A.Data+B.Data;
    return Result;
}

point2 operator+(const point2& A, const dim2& B) {
    point2 Result;
    Result.Data = A.Data+B.Data;
    return Result;
}

point2& operator+=(point2& A, const vec2& B) {
    A = A+B;
    return A;
}

point2i::point2i(s32 _x, s32 _y) : x(_x), y(_y) { }
point2i::point2i(s32_2x _Data) : Data(_Data) { }

bool operator!=(const point2i& A, const point2i& B) {
    return A.Data != B.Data;
}

bool operator==(const point2i& A, const point2i& B) {
    return A.Data == B.Data;
}

point2i operator+(const point2i& A, const point2i& B) {
    return A.Data+B.Data;
}

point2i& operator+=(point2i& A, const point2i& B) {
    A.Data += B.Data;
    return A;
}

point2i operator+(const point2i& A, const dim2i& B) {
    return A.Data+B.Data;
}

vec2i operator-(const point2i& A, const point2i& B) {
    return A.Data-B.Data;
}

dim2::dim2(const dim2i& Dim) : width((f32)Dim.width), height((f32)Dim.height) { }
dim2::dim2(f32_2x _Data) : Data(_Data) { }

dim2 operator/(f32 A, const dim2& B) {
    return A/B.Data;
}

dim2i::dim2i(s32 w, s32 h) : width(w), height(h) { }

dim2i::dim2i(const vec2i& Extent) : width(Extent.x), height(Extent.y) { }

dim2i::dim2i(s32_2x _Data) : Data(_Data) { }

bool operator!=(const dim2i& A, const dim2i& B) {
    return A.Data != B.Data;
}

bool operator==(const dim2i& A, const dim2i& B) {
    return A.Data == B.Data;
}

rect2::rect2(const point2& Min, const point2& Max) : P1(Min), P2(Max) { }
rect2::rect2(const rect2i& Rect) : P1(Rect.P1), P2(Rect.P2) { }

rect2& operator+=(rect2& A, const vec2& B) {
    A.P1 += B;
    A.P2 += B;
    return A;
}

rect2i::rect2i(const point2i& _Min, const point2i& _Max) : P1(_Min), P2(_Max) { }

s32 Rect2i_Get_Height(const rect2i& Rect) {
    return Rect.P2.y - Rect.P1.y;
}

dim2i Rect2i_Get_Dim(const rect2i& Rect) {
    return Rect.P2-Rect.P1;
}

color4::color4(f32 _r, f32 _g, f32 _b, f32 _a) : r(_r), g(_g), b(_b), a(_a) {}

color4 Color4_White(f32 Alpha) {
    return color4(1.0f, 1.0f, 1.0f, Alpha);
}

color4 Color4_Black(f32 Alpha) {
    return color4(0.0f, 0.0f, 0.0f, Alpha);
}

color4 Color4_Red(f32 Alpha) {
    return color4(1.0f, 0.0f, 0.0f, Alpha);
}

color4 Color4_Green(f32 Alpha) {
    return color4(0.0f, 1.0f, 0.0f, Alpha);
}

color4 Color4_Blue(f32 Alpha) {
    return color4(0.0f, 0.0f, 1.0f, Alpha);
}

color4 Color4_Yellow(f32 Alpha) {
    return color4(1.0f, 1.0f, 0.0f, Alpha);
}

color4 Color4_Magenta(f32 Alpha) {
    return color4(1.0f, 0.0f, 1.0f, Alpha);
}