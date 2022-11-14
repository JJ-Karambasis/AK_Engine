#include "shader_builder.h"
#include <Console/console.h>

int main(int ArgumentCount, char** Arguments)
{
    core* Core = Core_Init();
    
    console* Console = Console_Create(OS_Get_Allocator());
    Console_Begin_Arg(Console, Str8_Lit("--api"));
    {
        Console_Arg_Add_Required_Value(Console, Str8_Lit("GL"));
        Console_Arg_Add_Required_Value(Console, Str8_Lit("Direct3D"));
        Console_Arg_Add_Required_Value(Console, Str8_Lit("Vulkan"));
        Console_Arg_Set_Validation(Console, CONSOLE_VALIDATION_BIT_FLAG_STRING|CONSOLE_VALIDATION_BIT_FLAG_CASE_INSENSITIVE);
        Console_Arg_Set_Array_Restriction(Console, 1);
    }
    Console_End_Arg(Console);
    
    Console_Begin_Arg(Console, Str8_Lit("--shaderPath"));
    {
        Console_Arg_Set_Validation(Console, CONSOLE_VALIDATION_BIT_FLAG_PATH);
        Console_Arg_Set_Array_Restriction(Console, 1);
    }
    Console_End_Arg(Console);
    
    if(!Console_Parse(Console, Arguments, ArgumentCount))
        Console_Log_Error(Console);
    
    console_arg* Arg = Console_Get_Arg(Console, Str8_Lit("--api"));
    str8 Api = Console_Get_Arg_Value_Str(Console, Arg, 0);
    
    Arg= Console_Get_Arg(Console, Str8_Lit("--shaderPath"));
    str8 ShaderPath = Console_Get_Arg_Value_Str(Console, Arg, 0);
    
    Console_Delete(Console);
}

#include <Core/core.c>
#include <Console/console.c>