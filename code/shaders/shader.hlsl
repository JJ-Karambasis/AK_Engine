#include "shader_common.h"
#include "shader.h"

ConstantBuffer<view_data> ViewData : register(b0, SPACE(SHADER_VIEW_DATA_BIND_GROUP_INDEX));
ConstantBuffer<draw_data> DrawData : register(b0, SPACE(SHADER_DRAW_DATA_BIND_GROUP_INDEX));

struct vtx_input {
    vec3 P : POSITION0;
#ifdef USE_TEXTURE
    vec2 UV : TEXCOORD0;
#endif
};

struct vtx_output {
    vec4 P : SV_Position;

#ifdef USE_TEXTURE
    vec2 UV : TEXCOORD0;
#endif
};

vtx_output VS_Main(vtx_input Vtx) {
    vtx_output Result = (vtx_output)0;
    vec3 WorldP = mul(vec4(Vtx.P, 1.0f), DrawData.Model);
    Result.P = mul(vec4(WorldP, 1.0f), ViewData.ViewProjection);
#ifdef USE_TEXTURE
    Result.UV = Vtx.UV;
#endif
    return Result;
}

Texture2D<vec4> Texture : register(t0, SPACE(SHADER_TEXTURE_BIND_GROUP_INDEX));
sampler Sampler : register(s1, SPACE(SHADER_TEXTURE_BIND_GROUP_INDEX));

vec4 PS_Main(vtx_output Pxl) : SV_Target0 {
#ifdef USE_TEXTURE
    return Texture.Sample(Sampler, Pxl.UV);
#else
    return DrawData.Color;
#endif
}