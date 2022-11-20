#ifndef GL_SHADER_H
#define GL_SHADER_H

typedef enum gl_shader_type
{
    GL_SHADER_TYPE_COMPUTE,
    GL_SHADER_TYPE_VERTEX,
    GL_SHADER_TYPE_FRAGMENT,
    GL_SHADER_COUNT
} gl_shader_type;

typedef struct gl_shader_builder_entry
{
    strc                            ShaderCode;
    struct gl_shader_builder_entry* Next;
} gl_shader_builder_entry;

typedef struct gl_shader_builder_list
{
    gl_shader_builder_entry* First;
    gl_shader_builder_entry* Last;
    uint32_t Count;
} gl_shader_builder_list;

typedef struct gl_shader_builder
{
    arena* Arena;
    strc_list Shaders[GL_SHADER_COUNT];
    strc_list ErrorList;
} gl_shader_builder;

gl_shader_builder* GL_Shader_Builder_Create(arena* Arena);
void               GL_Shader_Builder_Add_Shader(gl_shader_builder* Builder, strc ShaderCode, gl_shader_type Type);
GLuint             GL_Shader_Builder_Compile_Program(gl_shader_builder* Builder);
strc               GL_Shader_Builder_Get_Error_String(gl_shader_builder* Builder, arena* Arena);

#endif
