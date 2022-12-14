const char gl_rect_shader[] = "#define v2 vec2\n"
"#define v3 vec3\n"
"#define v4 vec4\n"
"#define uint32_t uint\n"
"#define gpu_texture2D sampler2D\n"
"#define Floor_F32(v) floor(v)\n"
"#define Sample_Texture(sampler, uv) texture(sampler, v2(uv.x, 1.0f-uv.y))\n"
"#ifdef SHADER_CODE\n"
"#define Cast(type, value) type(value)\n"
"#else\n"
"#define Cast(type, value) (type)(value)\n"
"#endif\n"
"uint32_t Color_V4_To_U32(v4 Color)\n"
"{\n"
"    uint32_t B = Cast(uint32_t, Floor_F32(Color.z >= 1.0f ? 255 : Color.z * 255.0f));\n"
"    uint32_t G = Cast(uint32_t, Floor_F32(Color.y >= 1.0f ? 255 : Color.y * 255.0f));\n"
"    uint32_t R = Cast(uint32_t, Floor_F32(Color.x >= 1.0f ? 255 : Color.x * 255.0f));\n"
"    uint32_t A = Cast(uint32_t, Floor_F32(Color.w >= 1.0f ? 255 : Color.w * 255.0f));\n"
"    return R | (G << 8) | (B << 16) | (A << 24);\n"
"}\n"
"v4 Color_U32_To_V4(uint32_t Color)\n"
"{\n"
"    v4 Result;\n"
"    Result.x = Cast(float, ((Color >> 0) & 0xFF)) / 255.0f;\n"
"    Result.y = Cast(float, ((Color >> 8) & 0xFF)) / 255.0f;\n"
"    Result.z = Cast(float, ((Color >> 16) & 0xFF)) / 255.0f;\n"
"    Result.w = Cast(float, ((Color >> 24) & 0xFF)) / 255.0f;\n"
"    return Result;\n"
"}\n"
"\n"
"struct vs_input\n"
"{\n"
"    v2       Min;\n"
"    v2       Max;\n"
"    v2       InvHalfResolution;\n"
"    uint32_t VertexID;\n"
"};\n"
"\n"
"struct vs_output\n"
"{\n"
"    v4 Clip;\n"
"#ifdef TEXTURED\n"
"    v2 UV;\n"
"#endif\n"
"};\n"
"\n"
"vs_output Vertex_Shader(vs_input Input)\n"
"{\n"
"    vs_output Output;\n"
"    \n"
"    uint32_t VertexIndices[] = {0, 1, 2, 2, 3, 0};\n"
"    uint32_t VertexIndex = VertexIndices[Input.VertexID];\n"
"    \n"
"#ifdef TEXTURED\n"
"    v2 UV;\n"
"    UV.x = (VertexIndex == 0 || VertexIndex == 3) ? 1.0f : 0.0f;\n"
"    UV.y = (VertexIndex == 0 || VertexIndex == 1) ? 0.0f : 1.0f;\n"
"    Output.UV = UV;\n"
"#endif\n"
"    \n"
"    v2 V;\n"
"    V.x = (VertexIndex == 0 || VertexIndex == 3) ? Input.Max.x : Input.Min.x;\n"
"    V.y = (VertexIndex == 0 || VertexIndex == 1) ? Input.Min.y : Input.Max.y;\n"
"    \n"
"    Output.Clip = v4(V.x*Input.InvHalfResolution.x-1.0f, -(V.y*Input.InvHalfResolution.y-1.0f), 0.0f, 1.0f);\n"
"    return Output;\n"
"}\n"
"#ifdef VERTEX_SHADER\n"
"uniform v2 Min;\n"
"uniform v2 Max;\n"
"uniform v2 InvHalfResolution;\n"
"#ifdef TEXTURED\n"
"layout (location = 0) out v2 OutUV;\n"
"#endif\n"
"void main()\n"
"{\n"
"    vs_input Input;\n"
"    Input.Min = Min;\n"
"    Input.Max = Max;\n"
"    Input.InvHalfResolution = InvHalfResolution;\n"
"    Input.VertexID = gl_VertexID;\n"
"    \n"
"    vs_output Output = Vertex_Shader(Input);\n"
"    gl_Position = Output.Clip;\n"
"#ifdef TEXTURED\n"
"    OutUV = Output.UV;\n"
"#endif\n"
"}\n"
"#endif\n"
"#ifdef PIXEL_SHADER\n"
"#ifdef TEXTURED\n"
"layout(location = 0) in v2 UV;\n"
"uniform gpu_texture2D Texture;\n"
"#endif\n"
"uniform uint32_t Color;\n"
"layout(location = 0) out v4 OutColor;\n"
"void main()\n"
"{\n"
"    v4 Color = Color_U32_To_V4(Color);\n"
"    Color.rgb *= Color.a;\n"
"#ifdef TEXTURED\n"
"    Color *= Sample_Texture(Texture, UV);\n"
"#endif\n"
"    OutColor = Color;\n"
"}\n"
"#endif\n";