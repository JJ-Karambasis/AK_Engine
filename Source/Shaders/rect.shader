#include "common.shader"

struct vs_input
{
    v2       Min;
    v2       Max;
    v2       InvHalfResolution;
    uint32_t VertexID;
};

struct vs_output
{
    v4 Clip;
#ifdef TEXTURED
    v2 UV;
#endif
};

vs_output Vertex_Shader(vs_input Input)
{
    vs_output Output;
    
    uint32_t VertexIndices[] = {0, 1, 2, 2, 3, 0};
    uint32_t VertexIndex = VertexIndices[Input.VertexID];
    
#ifdef TEXTURED
    v2 UV;
    UV.x = (VertexIndex == 0 || VertexIndex == 1) ? 0.0f : 1.0f;
    UV.y = (VertexIndex == 1 || VertexIndex == 2) ? 1.0f : 0.0f;
    Output.UV = UV;
#endif
    
    v2 V;
    V.x = (VertexIndex == 0 || VertexIndex == 1) ? Input.Min.x : Input.Max.x;
    V.y = (VertexIndex == 0 || VertexIndex == 3) ? Input.Min.y : Input.Max.y;
    
    Output.Clip = v4(V.x*Input.InvHalfResolution.x-1.0f, V.y*Input.InvHalfResolution.y-1.0f, 0.0f, 1.0f);
    return Output;
}