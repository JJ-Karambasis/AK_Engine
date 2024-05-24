#include "shader_common.h"
#include "ui_shader.h"

struct ps_input {
    vec4 P  : SV_Position;
    vec2 UV : TEXCOORD0;
    vec4 C  : COLOR0;
};

ConstantBuffer<ui_global_data> GlobalData : register(BUFFER(0), SPACE(0));

Texture2D<vec4> Texture : register(TEXTURE(0), SPACE(1));
sampler Sampler         : register(SAMPLER(1), SPACE(1));

ps_input VS_Main(ui_vertex Vertex) {
    ps_input Result = (ps_input)0;

    Result.P  = mul(point4(Vertex.P, 0.0f, 1.0f), GlobalData.Projection);
    Result.UV = Vertex.UV;
    Result.C  = Vertex.C;

    return Result;
}

vec4 PS_Main(ps_input Input) : SV_Target0 {
    vec4 Result = Texture.Sample(Sampler, Input.UV)*Input.C;
    return Result;
}