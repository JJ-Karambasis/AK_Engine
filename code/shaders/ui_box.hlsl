#include "shader_common.h"
#include "ui_box_shader.h"

struct ps_input {
    vec4 Vertex : SV_Position;
    vec2 UV     : TEXCOORD0;
    vec4 Color  : COLOR0;
};

ConstantBuffer<ui_box_shader_global>  GlobalData : register(BUFFER(0), SPACE(UI_BIND_GROUP_GLOBAL_INDEX));
ConstantBuffer<ui_box_shader_dynamic> DynamicData : register(BUFFER(0), SPACE(UI_BIND_GROUP_DYNAMIC_INDEX));

ps_input VS_Main(u32 VertexID : SV_VertexID) {
    static vec2 Vertices[] = {
        {-1, +1},
        {-1, -1},
        {+1, +1},
        {+1, -1}
    };

    static vec2 UVs[] = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1}
    };

    vec2 DstHalfSize = (DynamicData.P2-DynamicData.P1) * 0.5f;
    vec2 DstCenter = (DynamicData.P2+DynamicData.P1) * 0.5f;
    
    //We want to flip to be top down not bottom up
    DstCenter.y = -DstCenter.y;

    vec2 DstVertex = Vertices[VertexID]*DstHalfSize + DstCenter;

    ps_input Result = (ps_input)0;
    Result.Vertex = vec4((2.0f * DstVertex.x * GlobalData.InvRes.x) - 1.0f, 
                         (2.0f * DstVertex.y * GlobalData.InvRes.y) + 1.0f,
                         0, 1);
    Result.UV = UVs[VertexID];

    Result.Color = DynamicData.Color;
    return Result;
}

vec4 PS_Main(ps_input Input) : SV_Target0 {
    return Input.Color;
}