#include "shader_common.h"
#include "ui_box_shader.h"

struct ps_input {
    vec4 Vertex : SV_Position;
    vec2 UV     : TEXCOORD0;
    vec4 Color  : COLOR0;
};

Texture2D<vec4> Texture : register(TEXTURE(0), SPACE(0));
sampler Sampler         : register(SAMPLER(1), SPACE(0));
ConstantBuffer<ui_box_shader_info> ShaderInfo : register(BUFFER(0), SPACE(1));

ps_input VS_Main(ui_box_shader_box Box, u32 VertexID : SV_VertexID, u32 InstanceID : SV_InstanceID) {
    static const vec2 Vertices[] = {
        {-1, +1},
        {-1, -1},
        {+1, +1},
        {+1, -1}
    };

    static const vec2 UVs[] = {
        {-1,-1},
        {-1, 1},
        { 1,-1},
        { 1, 1}
    };

    vec2 DstHalfSize = (Box.DstP1-Box.DstP0) * 0.5f;
    vec2 DstCenter = (Box.DstP1+Box.DstP0) * 0.5f;
    
    //We want to flip to be top down not bottom up
    DstCenter.y = -DstCenter.y;
    vec2 DstVertex = Vertices[VertexID]*DstHalfSize + DstCenter;

    vec2 SrcHalfSize = (Box.SrcP1-Box.SrcP0) * 0.5f;
    vec2 SrcCenter = (Box.SrcP1+Box.SrcP0) * 0.5f;
    
    //We want to flip to be top down not bottom up
    vec2 SrcVertex = UVs[VertexID]*SrcHalfSize + SrcCenter;

    ps_input Result = (ps_input)0;
    Result.Vertex = vec4((2.0f * DstVertex.x * ShaderInfo.InvRes.x) - 1.0f, 
                         (2.0f * DstVertex.y * ShaderInfo.InvRes.y) + 1.0f,
                         0, 1);

    Result.UV = SrcVertex*ShaderInfo.InvTexRes;

    Result.Color = Box.Color;
    return Result;
}

vec4 PS_Main(ps_input Input) : SV_Target0 {
    vec4 Result = Texture.Sample(Sampler, Input.UV)*Input.Color;
    return Result;
}