#include "shader_builder.h"
#include <Console/console.h>

void Get_Recursive_File(str8_list* Files, allocator* Allocator, str8 Directory)
{
    os_file_enumerator* FileEnumerator = OS_File_Enumerator_Create(Allocator);
    os_file_enumerator_entry* File = OS_File_Enumerator_Begin(FileEnumerator, Directory);
    while(File)
    {
        if(!Str8_Equal(File->Filename, Str8_Lit(".")) &&
           !Str8_Equal(File->Filename, Str8_Lit("..")))
        {
            str8 Path = File->Path;
            if(OS_Directory_Exists(Path))
            {
                Get_Recursive_File(Files, Allocator, Path);
            }
            else if(OS_File_Exists(Path))
            {
                Str8_List_Push(Files, Allocator, Str8_Copy(Allocator, Path));
            }
        }
        File = OS_File_Enumerator_Next(FileEnumerator);
    }
    
    OS_File_Enumerator_End(FileEnumerator);
    OS_File_Enumerator_Delete(FileEnumerator);
}

str8 Find_Include_Path(str8 Str)
{
    const str8 IncludePattern = Str8_Lit("#include");
    
    uint64_t i = Str8_Find_First(Str, IncludePattern);
    if(i != STR_INVALID_FIND)
    {
        str8 Source = Str8_Skip(Str, i+IncludePattern.Length);
        i = 0;
        for(; i < Source.Length; i++)
        {
            if(!Is_Whitespace8(Source.Str[i]))
            {
                if(Source.Str[i] == '\"')
                {
                    uint64_t StartPathIndex = i+1;
                    uint64_t EndPathIndex = StartPathIndex;
                    bool32_t FoundQuote = false;
                    for(; EndPathIndex < Source.Length; EndPathIndex++)
                    {
                        if(Source.Str[EndPathIndex] == '\"')
                        {
                            return Str8_Substr(Source, StartPathIndex, EndPathIndex);
                        }
                    }
                    
                    if(!FoundQuote)
                    {
                        //TODO(JJ): Some error
                        EndPathIndex = STR_INVALID_FIND;
                    }
                }
                
                return Str8_Empty();
            }
        }
    }
    return Str8_Empty();
}

str8 Inject_Includes(allocator* Allocator, str8 ShaderPath, str8 Shader)
{
    const uint8_t* At = ShaderPath.Str;
    const uint8_t* End = At + ShaderPath.Length;
    
    str8_list ShaderLines = Str8_Split(Allocator, Shader, '\n');
    for(str8_node* Node = ShaderLines.First; Node; Node = Node->Next)
    {
        str8 Path = Find_Include_Path(Node->Str);
        if(Path.Length)
        {
            buffer Buffer;
            str8 FullPath = Str8_Concat(Allocator, ShaderPath, Path);
            if(OS_Read_Entire_File_Null_Term(&Buffer, Allocator, FullPath))
            {
                uint64_t SlashIndex = Str8_Find_Last_Char(FullPath, OS_FILE_DELIMTER_CHAR);
                str8 ShaderFilePath = Str8_Substr(FullPath, 0, SlashIndex+1);
                
                SlashIndex = Str8_Find_Last_Char(FullPath, '/');
                if(SlashIndex != STR_INVALID_FIND)
                    ShaderFilePath = Str8_Substr(FullPath, 0, SlashIndex+1);
                
                str8 InjectedShader = Inject_Includes(Allocator, ShaderFilePath, Str8(Buffer.Ptr, Buffer.Size));
                Str8_List_Update_Node(&ShaderLines, Node, InjectedShader);
            }
        }
    }
    
    str8 Result = Str8_List_Join_Newline(Allocator, &ShaderLines);
    return Result;
}

bool8_t Run(arena* Arena, console* Console, char** Arguments, int ArgumentCount)
{
    if(!Console_Parse(Console, Arguments, ArgumentCount))
    {
        Console_Log_Error(Console);
        return false;
    }
    
    console_arg* Arg = Console_Get_Arg(Console, Str8_Lit("--api"));
    str8 Api = Console_Get_Arg_Value_Str(Console, Arg, 0);
    
    Arg = Console_Get_Arg(Console, Str8_Lit("--shaderPath"));
    str8 ShaderPath = Console_Get_Arg_Value_Str(Console, Arg, 0);
    
    Arg = Console_Get_Arg(Console, Str8_Lit("--outputFilePath"));
    str8 OutFilePath = Console_Get_Arg_Value_Str(Console, Arg, 0);
    if(!Str8_Ends_With_Char(OutFilePath, OS_FILE_DELIMTER_CHAR))
        OutFilePath = Str8_Concat(Get_Base_Allocator(Arena), OutFilePath, Str8_Lit(OS_FILE_DELIMTER));
    
    str8 OutHeaderFileName = Str8_Concat(Get_Base_Allocator(Arena), Api, Str8_Lit("_shader_generated"));
    str8 OutHeaderFileNameUpper = Str8_To_Upper(Get_Base_Allocator(Arena), OutHeaderFileName);
    
    str8 OutHeaderFilePath = Str8_Format(Get_Base_Allocator(Arena), Str8_Lit("%.*s%.*s.h"), OutFilePath.Length, OutFilePath.Str, OutHeaderFileName.Length, OutHeaderFileName.Str);
    str8 OutSourceFilePath = Str8_Format(Get_Base_Allocator(Arena), Str8_Lit("%.*s%.*s.c"), OutFilePath.Length, OutFilePath.Str, OutHeaderFileName.Length, OutHeaderFileName.Str);
    
    str8_list OutputSourceFileList;
    Zero_Struct(&OutputSourceFileList, str8_list); 
    
    if(!Str8_Ends_With_Char(ShaderPath, OS_FILE_DELIMTER_CHAR))
        ShaderPath = Str8_Concat(Get_Base_Allocator(Arena), ShaderPath, Str8_Lit(OS_FILE_DELIMTER));
    
    str8 ApiShaderPath = Str8_Empty();
    str8 ApiShaderExtension = Str8_Empty();
    if(Str8_Equal(Str8_Lit("gl"), Api))
    {
        ApiShaderPath = Str8_Concat(Get_Base_Allocator(Arena), ShaderPath, Str8_Lit(Glue("GL" , OS_FILE_DELIMTER)));
        ApiShaderExtension = Str8_Lit("glsl");
    }
    
    str8_list ShaderFiles;
    Zero_Struct(&ShaderFiles, str8_list);
    Get_Recursive_File(&ShaderFiles, Get_Base_Allocator(Arena), ApiShaderPath);
    
    for(str8_node* Node = ShaderFiles.First; Node; Node = Node->Next)
    {
        str8 File = Node->Str;
        
        uint64_t DotIndex = Str8_Find_Last_Char(File, '.');
        str8 Ext = Str8_Substr(File, DotIndex+1, File.Length);
        if(Str8_Equal(Ext, ApiShaderExtension))
        {
            uint64_t SlashIndex = Str8_Find_Last_Char(File, OS_FILE_DELIMTER_CHAR);
            str8 ShaderName = Str8_Substr(File, SlashIndex+1, DotIndex);
            str8 ShaderFilePath = Str8_Substr(File, 0, SlashIndex+1);
            
            buffer ShaderBuffer;
            OS_Read_Entire_File_Null_Term(&ShaderBuffer, Get_Base_Allocator(Arena), File);
            str8 Shader = Inject_Includes(Get_Base_Allocator(Arena), ShaderFilePath, Str8((const uint8_t*)ShaderBuffer.Ptr, ShaderBuffer.Size));
            
            str8_list ShaderLines = Str8_Split(Get_Base_Allocator(Arena), Shader, '\n');
            for(str8_node* LineNode = ShaderLines.First; LineNode; LineNode = LineNode->Next)
            {
                str8_list StrList;
                Zero_Struct(&StrList, str8_list);
                Str8_List_Push(&StrList, Get_Base_Allocator(Arena), Str8_Lit("\""));
                Str8_List_Push(&StrList, Get_Base_Allocator(Arena), Str8_Replace(Get_Base_Allocator(Arena), LineNode->Str, Str8_Lit("\""), Str8_Lit("\\\"")));
                Str8_List_Push(&StrList, Get_Base_Allocator(Arena), Str8_Lit("\\n\""));
                Str8_List_Update_Node(&ShaderLines, LineNode, Str8_List_Join(Get_Base_Allocator(Arena), &StrList));
            }
            Shader = Str8_List_Join_Newline(Get_Base_Allocator(Arena), &ShaderLines);
            Str8_List_Push_Format(&OutputSourceFileList, Get_Base_Allocator(Arena), Str8_Lit("const char %.*s_shader[] = %.*s;"), 
                                  ShaderName.Length, ShaderName.Str, Shader.Length, Shader.Str);
        }
    }
    
    str8 OutSourceFile = Str8_List_Join_Newline(Get_Base_Allocator(Arena), &OutputSourceFileList);
    OS_Write_Entire_File(OutSourceFilePath, OutSourceFile.Str, Safe_U64_U32(OutSourceFile.Length));
    
    return true;
}

int main(int ArgumentCount, char** Arguments)
{
    core* Core = Core_Init();
    console* Console = Console_Create(OS_Get_Allocator());
    Console_Begin_Arg(Console, Str8_Lit("--api"), true);
    {
        Console_Arg_Add_Required_Value(Console, Str8_Lit("GL"));
        Console_Arg_Add_Required_Value(Console, Str8_Lit("D3D"));
        Console_Arg_Add_Required_Value(Console, Str8_Lit("VK"));
        Console_Arg_Set_Validation(Console, CONSOLE_VALIDATION_TYPE_CASE_INSENSITIVE);
        Console_Arg_Set_Array_Restriction(Console, 1);
    }
    Console_End_Arg(Console);
    
    Console_Begin_Arg(Console, Str8_Lit("--shaderPath"), true);
    {
        Console_Arg_Set_Validation(Console, CONSOLE_VALIDATION_TYPE_DIRECTORY);
        Console_Arg_Set_Array_Restriction(Console, 1);
    }
    Console_End_Arg(Console);
    
    Console_Begin_Arg(Console, Str8_Lit("--outputFilePath"), true);
    {
        Console_Arg_Set_Validation(Console, CONSOLE_VALIDATION_TYPE_DIRECTORY);
        Console_Arg_Set_Array_Restriction(Console, 1);
    }
    Console_End_Arg(Console);
    
    arena* Arena = Arena_Create(OS_Get_Allocator(), Mega(1));
    int Result = Run(Arena, Console, Arguments, ArgumentCount) ? 0 : 1;
    Arena_Delete(Arena);
    
    Console_Delete(Console);
    Core_Shutdown();
    
    return Result;
}

#include <Core/core.c>
#include <Console/console.c>