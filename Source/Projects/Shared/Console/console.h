#ifndef CONSOLE_H
#define CONSOLE_H

typedef enum console_value_type
{
    CONSOLE_VALUE_TYPE_INTEGER,
    CONSOLE_VALUE_TYPE_FLOAT,
    CONSOLE_VALUE_TYPE_STRING
} console_value_type;

typedef enum console_validation_type
{
    CONSOLE_VALIDATION_TYPE_NONE,
    CONSOLE_VALIDATION_TYPE_INTEGER,
    CONSOLE_VALIDATION_TYPE_FLOAT,
    CONSOLE_VALIDATION_TYPE_NUMERIC,
    CONSOLE_VALIDATION_TYPE_CASE_INSENSITIVE,
    CONSOLE_VALIDATION_TYPE_DIRECTORY,
    CONSOLE_VALIDATION_TYPE_FILE,
    CONSOLE_VALIDATION_TYPE_COUNT
} console_validation_type;

typedef struct console console;
typedef struct console_arg console_arg;

console*           Console_Create(allocator* Allocator);
void               Console_Begin_Arg(console* Console, str8 Argument);
void               Console_Arg_Add_Required_Value(console* Console, str8 Value);
void               Console_Arg_Set_Validation(console* Console, console_validation_type Validation);
void               Console_Arg_Set_Array_Min_Restriction(console* Console, uint32_t Min);
void               Console_Arg_Set_Array_Max_Restriction(console* Console, uint32_t Max);
void               Console_Arg_Set_Array_Restriction(console* Console, uint32_t Value);
void               Console_End_Arg(console* Console);
bool8_t            Console_Parse(console* Console, const char** Args, int ArgCount);
void               Console_Log_Error(console* Console);
console_arg*       Console_Get_Arg(console* Console, str8 Arg);
uint32_t           Console_Get_Arg_Array_Length(console* Console, console_arg* Arg);
console_value_type Console_Get_Arg_Value_Type(console* Console, console_arg* Arg, uint32_t ArgumentIndex);
str8               Console_Get_Arg_Value_Str(console* Console, console_arg* Arg, uint32_t ArgumentIndex);
int64_t            Console_Get_Arg_Value_Integer(console* Console, console_arg* Arg, uint32_t ArgumentIndex);
double             Console_Get_Arg_Value_Float(console* Console, console_arg* Arg, uint32_t ArgumentIndex);
void               Console_Delete(console* Console);

#endif