vec2i::vec2i(s32 _x, s32 _y) : x(_x), y(_y) { }
vec2i::vec2i(s32_2x _Data) : Data(_Data) { }

bool operator!=(const vec2i& A, const vec2i& B) {
    return A.Data != B.Data;
}

bool operator==(const vec2i& A, const vec2i& B) {
    return A.Data == B.Data;
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

point2i operator+=(point2i& A, const point2i& B) {
    A.Data += B.Data;
    return A;
}

point2i operator+(const point2i& A, const dim2i& B) {
    return A.Data+B.Data;
}

vec2i operator-(const point2i& A, const point2i& B) {
    return A.Data-B.Data;
}

dim2i::dim2i(s32 w, s32 h) : width(w), height(h) { }

dim2i::dim2i(const vec2i& Extent) : width(Extent.x), height(Extent.y) { }

dim2i::dim2i(s32_2x _Data) : Data(_Data) { }

rect2i::rect2i(const point2i& _Min, const point2i& _Max) : P1(_Min), P2(_Max) { }

s32 Rect2i_Get_Height(const rect2i& Rect) {
    return Rect.P2.y - Rect.P1.y;
}

dim2i Rect2i_Get_Dim(const rect2i& Rect) {
    return Rect.P2-Rect.P1;
}