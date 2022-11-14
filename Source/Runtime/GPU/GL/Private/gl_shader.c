static GLenum G_ShaderTypes[GL_SHADER_COUNT] = 
{
    GL_COMPUTE_SHADER,
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER
};

static strc G_ShaderTypesStr[GL_SHADER_COUNT] = 
{
    StrC_Expand("Compute"), 
    StrC_Expand("Vertex"), 
    StrC_Expand("Fragment")
};

gl_shader_builder* GL_Shader_Builder_Create(arena* Arena)
{
    arena* NewArena = Arena_Create(Get_Base_Allocator(Arena), Kilo(256));
    gl_shader_builder* Builder = Arena_Push_Struct(NewArena, gl_shader_builder);
    Builder->Arena = NewArena;
    return Builder;
}

void GL_Shader_Builder_Add_Shader(gl_shader_builder* Builder, strc ShaderCode, gl_shader_type Type)
{
    gl_shader_builder_list*  List = Builder->Shaders + Type;
    gl_shader_builder_entry* Entry = Arena_Push_Struct(Builder->Arena, gl_shader_builder_entry);
    Entry->ShaderCode = StrC_Copy(Get_Base_Allocator(Builder->Arena), ShaderCode);
    SLL_Push_Back(List->First, List->Last, Entry);
    List->Count++;
}

GLuint GL_Shader_Builder_Compile_Program(gl_shader_builder* Builder)
{
    uint32_t ShaderCount = 0;
    GLuint   Shaders[GL_SHADER_COUNT];
    
    bool32_t HasThrownErrors = false;
    
    for(uint32_t ShaderIndex = 0; ShaderIndex < GL_SHADER_COUNT; ShaderIndex++)
    {
        gl_shader_builder_list* List = Builder->Shaders + ShaderIndex;
        if(List->Count > 0)
        {
            const char** ShaderCode = Arena_Push_Array(Builder->Arena, const char*, List->Count);
            
            const char** At = ShaderCode;
            for(gl_shader_builder_entry* Entry = List->First; Entry; Entry = Entry->Next)
            {
                *At = (const char*)Entry->ShaderCode.Str;
                At++;
            }
            
            GLuint Shader = glCreateShader(G_ShaderTypes[ShaderIndex]);
            glShaderSource(Shader, List->Count, ShaderCode, NULL);
            glCompileShader(Shader);
            
            GLint HasCompiled;
            glGetShaderiv(Shader, GL_COMPILE_STATUS, &HasCompiled);
            if(HasCompiled != GL_TRUE)
            {
                HasThrownErrors = true;
                
                GLint ErrorLength;
                glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &ErrorLength);
                
                char* ErrorBuffer = Arena_Push_Array(Builder->Arena, char, ErrorLength);
                glGetShaderInfoLog(Shader, ErrorLength, &ErrorLength, ErrorBuffer);
                
                strc Str = StrC(ErrorBuffer, ErrorLength);
                StrC_List_Push_Format(&Builder->ErrorList, Get_Base_Allocator(Builder->Arena), StrC_Lit("%.*s Shader error: %.*s"), G_ShaderTypesStr[ShaderIndex].Length, G_ShaderTypesStr[ShaderIndex].Str, ErrorLength, ErrorBuffer);
            }
            
            Shaders[ShaderCount++] = Shader;
        }
    }
    
    GLuint Program = 0;
    if(!HasThrownErrors)
    {
        Program = glCreateProgram();
        for(uint32_t ShaderIndex = 0; ShaderIndex < ShaderCount; ShaderIndex++)
            glAttachShader(Program, Shaders[ShaderIndex]);
        
        glLinkProgram(Program);
        
        GLint HasLinked;
        glGetProgramiv(Program, GL_LINK_STATUS, &HasLinked);
        if(HasLinked != GL_TRUE)
        {
            HasThrownErrors = true;
            
            GLint ErrorLength;
            glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &ErrorLength);
            
            char* ErrorBuffer = Arena_Push_Array(Builder->Arena, char, ErrorLength);
            glGetProgramInfoLog(Program, ErrorLength, &ErrorLength, ErrorBuffer);
            
            StrC_List_Push_Format(&Builder->ErrorList, Get_Base_Allocator(Builder->Arena), StrC_Lit("Program link error: %.*s"), 
                                  ErrorLength, ErrorBuffer);
        }
        
        for(uint32_t ShaderIndex = 0; ShaderIndex < ShaderCount; ShaderIndex++)
            glDetachShader(Program, Shaders[ShaderIndex]);
    }
    
    for(uint32_t ShaderIndex = 0; ShaderIndex < ShaderCount; ShaderIndex++)
        glDeleteShader(Shaders[ShaderIndex]);
    
    if(HasThrownErrors && Program)
    {
        glDeleteProgram(Program);
        return 0;
    }
    
    return Program;
}

strc GL_Shader_Builder_Get_Error_String(gl_shader_builder* Builder, arena* Arena)
{
    strc Result = StrC_List_Join(Get_Base_Allocator(Arena), &Builder->ErrorList);
    return Result;
}