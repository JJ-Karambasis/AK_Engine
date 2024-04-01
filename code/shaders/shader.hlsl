#include "shader_common.h"

struct vtx_input {
    vec3 P : POSITION0;
};

struct vtx_output {
    vec4 P : SV_Position;
};

vtx_output VS_Main(vtx_input Vtx) {
    vtx_output Result = (vtx_output)0;
    Result.P = vec4(Vtx.P, 1.0f);
    return Result;
}

vec4 PS_Main(vtx_output Pxl) : SV_Target0 {
    return vec4(1.0f, 0.0f, 0.0f, 1.0f);
}