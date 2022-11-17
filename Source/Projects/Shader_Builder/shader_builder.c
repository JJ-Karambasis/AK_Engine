#include "shader_builder.h"
#include <Console/console.h>

bool8_t Run(console* Console, char** Arguments, int ArgumentCount)
{
    if(!Console_Parse(Console, Arguments, ArgumentCount))
    {
        Console_Log_Error(Console);
        return false;
    }
    
    console_arg* Arg = Console_Get_Arg(Console, Str8_Lit("--api"));
    str8 Api = Console_Get_Arg_Value_Str(Console, Arg, 0);
    
    Arg= Console_Get_Arg(Console, Str8_Lit("--shaderPath"));
    str8 ShaderPath = Console_Get_Arg_Value_Str(Console, Arg, 0);
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
    
    int Result = Run(Console, Arguments, ArgumentCount) ? 0 : 1;
    
    Console_Delete(Console);
    Core_Shutdown();
    
    return Result;
}

#include <Core/core.c>
#include <Console/console.c>