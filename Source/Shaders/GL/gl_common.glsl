#define v2 vec2
#define v3 vec3
#define v4 vec4

#ifdef VERTEX_SHADER
#define Vertex_Shader_Main() void main()
#endif

#ifdef PIXEL_SHADER
#define Pixel_Shader_Main() void main()
#endif

#define Shader_Get_Vertex_Index() gl_VertexID