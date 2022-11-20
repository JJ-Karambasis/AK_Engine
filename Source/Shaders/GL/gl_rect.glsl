#include "gl_common.h"
#include "../rect.shader"

#ifdef VERTEX_SHADER

uniform v2 Min;
uniform v2 Max;
uniform v2 InvHalfResolution;

#ifdef TEXTURED
layout (location = 0) out v2 OutUV;
#endif

void main()
{
    vs_input Input;
    Input.Min = Min;
    Input.Max = Max;
    Input.InvHalfResolution = InvHalfResolution;
    Input.VertexID = gl_VertexID;
    
    vs_output Output = Vertex_Shader(Input);
    gl_Position = Output.Clip;
#ifdef TEXTURED
    OutUV = Output.UV;
#endif
}

#endif

#ifdef PIXEL_SHADER

#ifdef TEXTURED
layout(location = 0) in v2 UV;
uniform gpu_texture2D Texture;
#else
uniform uint32_t Color;
#endif

layout(location = 0) out v4 OutColor;

void main()
{
#ifdef TEXTURED
    v4 Color = Sample_Texture(Texture, UV);
#else
    v4 Color = Color_U32_To_V4(Color);
    Color.rgb *= Color.a;
#endif
    OutColor = Color;
}

#endif