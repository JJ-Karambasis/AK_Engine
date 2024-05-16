vec2i::vec2i(s32 _x, s32 _y) : x(_x), y(_y) { }
vec2i::vec2i(s32_2x _Data) : Data(_Data) { }

bool operator!=(vec2i A, vec2i B) {
    return A.Data != B.Data;
}

bool operator==(vec2i A, vec2i B) {
    return A.Data == B.Data;
}

point2i::point2i(s32 _x, s32 _y) : x(_x), y(_y) { }

bool operator!=(point2i A, point2i B) {
    return A.Data != B.Data;
}

bool operator==(point2i A, point2i B) {
    return A.Data == B.Data;
}

point2i operator+(point2i A, point2i B) {
    return A.Data+B.Data;
}

point2i operator+=(point2i& A, point2i B) {
    A.Data += B.Data;
    return A;
}

vec2i operator-(point2i A, point2i B) {
    return A.Data-B.Data;
}

dim2i::dim2i(s32 w, s32 h) : width(w), height(h) { }

dim2i::dim2i(const vec2i& Extent) : width(Extent.x), height(Extent.y) { }

rect2i::rect2i(point2i _Min, point2i _Max) : P1(_Min), P2(_Max) { }

dim2i Rect2i_Get_Dim(const rect2i& Rect) {
    return Rect.P2-Rect.P1;
}