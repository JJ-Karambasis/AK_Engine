#include "common.shader"

PLATFORM_SHADER_CODE

#ifdef VERTEX_SHADER
Vertex_Shader_Main()
{
    static uint32_t VertexIndices[] = {0, 1, 2, 2, 3, 0};
    uint32_t VertexIndex = VertexIndices[Shader_Get_Vertex_Index()];
    
#ifdef TEXTURED
    v2 UV;
    UV.x = (VertexIndex == 0 || VertexIndex == 1) ? 0.0f : 1.0f;
    UV.y = (VertexIndex == 1 || VertexIndex == 2) ? 1.0f : 0.0f;
    Shader_Set_Vertex_Output(UV, UV);
#endif
    
    rect_constant Rect = Rect_Shader_Get_Rect();
    
    v2 V;
    V.x = (VertexIndex == 0 || VertexIndex == 1) ? Rect.Min.x : Rect.Max.x;
    V.y = (VertexIndex == 0 || VertexIndex == 3) ? Rect.Min.y : Rect.Max.y;
    
    v4 Clip = v4(V.x*Rect.InvHalfResolution.x-1.0f, -V.y*Rect.InvHalfResolution.y+1.0f, 0.0f, 1.0f);
    
    Shader_Finish_Vertex(Clip);
}
#endif

#ifdef PIXEL_SHADER
Pixel_Shader_Main()
{
#ifdef TEXTURED
    v2 UV = Shader_Get_Pixel_Input(UV);
    v4 Color = Shader_Sample_Texture(Rect_Shader_Get_Sampler(), Rect_Shader_Get_Texture(), UV);
#else
    v4 Color = Rect_Shader_Get_Color();
#endif
    
    Shader_Set_Pixel_Output(0, Color);
    Shader_Finish_Pixel();
}
#endif  