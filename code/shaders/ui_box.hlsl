#include "shader_common.h"
#include "ui_box_shader.h"

struct ps_input {
    vec4 Vertex : SV_Position;
    vec2 UV     : TEXCOORD0;
    vec4 Color  : COLOR0;
};

ConstantBuffer<ui_box_shader_global>  GlobalData : register(BUFFER(0), SPACE(UI_BIND_GROUP_GLOBAL_INDEX));
ConstantBuffer<ui_box_shader_dynamic> DynamicData : register(BUFFER(0), SPACE(UI_BIND_GROUP_DYNAMIC_INDEX));

Texture2D<vec4> Texture : register(TEXTURE(UI_TEXTURE_BIND_GROUP_TEXTURE_BINDING), SPACE(UI_BIND_GROUP_TEXTURE_INDEX));
sampler Sampler : register(s1, SPACE(UI_BIND_GROUP_TEXTURE_INDEX));

ps_input VS_Main(u32 VertexID : SV_VertexID) {
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

    vec2 DstHalfSize = (DynamicData.DstP1-DynamicData.DstP0) * 0.5f;
    vec2 DstCenter = (DynamicData.DstP1+DynamicData.DstP0) * 0.5f;
    
    //We want to flip to be top down not bottom up
    DstCenter.y = -DstCenter.y;
    vec2 DstVertex = Vertices[VertexID]*DstHalfSize + DstCenter;

    vec2 SrcHalfSize = (DynamicData.SrcP1-DynamicData.SrcP0) * 0.5f;
    vec2 SrcCenter = (DynamicData.SrcP1+DynamicData.SrcP0) * 0.5f;
    
    //We want to flip to be top down not bottom up
    vec2 SrcVertex = UVs[VertexID]*SrcHalfSize + SrcCenter;

    ps_input Result = (ps_input)0;
    Result.Vertex = vec4((2.0f * DstVertex.x * GlobalData.InvRes.x) - 1.0f, 
                         (2.0f * DstVertex.y * GlobalData.InvRes.y) + 1.0f,
                         0, 1);

    Result.UV = SrcVertex*GlobalData.InvTexRes;

    Result.Color = DynamicData.Color;
    return Result;
}

vec4 PS_Main(ps_input Input) : SV_Target0 {
    vec4 Result = Texture.Sample(Sampler, Input.UV)*Input.Color;
    return Result;
}