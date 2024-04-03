#include "shader_common.h"
#include "shader.h"

ConstantBuffer<view_data> ViewData : register(b0, SPACE(SHADER_VIEW_DATA_BIND_GROUP_INDEX));
ConstantBuffer<draw_data> DrawData : register(b0, SPACE(SHADER_DRAW_DATA_BIND_GROUP_INDEX));

struct vtx_input {
    vec3 P : POSITION0;
};

struct vtx_output {
    vec4 P : SV_Position;
};

vtx_output VS_Main(vtx_input Vtx) {
    vtx_output Result = (vtx_output)0;
    vec3 WorldP = mul(vec4(Vtx.P, 1.0f), DrawData.Model);
    Result.P = mul(vec4(WorldP, 1.0f), ViewData.ViewProjection);
    return Result;
}

vec4 PS_Main(vtx_output Pxl) : SV_Target0 {
    return vec4(1.0f, 0.0f, 0.0f, 1.0f);
}