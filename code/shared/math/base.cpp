vec2i::vec2i(s32 _x, s32 _y) : x(_x), y(_y) { }

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

rect2i::rect2i(point2i _Min, point2i _Max) : P1(_Min), P2(_Max) { }