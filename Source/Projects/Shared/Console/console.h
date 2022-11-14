#ifndef CONSOLE_H
#define CONSOLE_H

typedef enum console_value_type
{
    CONSOLE_VALUE_TYPE_INTEGER,
    CONSOLE_VALUE_TYPE_FLOAT,
    CONSOLE_VALUE_TYPE_STRING
} console_value_type;

enum
{
    CONSOLE_VALIDATION_BIT_FLAG_NONE = 0,
    CONSOLE_VALIDATION_BIT_FLAG_STRING = (1 << 0),
    CONSOLE_VALIDATION_BIT_FLAG_INTEGER = (1 << 1),
    CONSOLE_VALIDATION_BIT_FLAG_FLOAT = (1 << 2),
    CONSOLE_VALIDATION_BIT_FLAG_NUMERIC = (1 << 3),
    CONSOLE_VALIDATION_BIT_FLAG_CASE_INSENSITIVE = (1 << 4),
    CONSOLE_VALIDATION_BIT_FLAG_PATH = (1 << 5)
};

typedef struct console console;
typedef struct console_arg console_arg;

console*           Console_Create(allocator* Allocator);
void               Console_Begin_Arg(console* Console, str8 Argument);
void               Console_Arg_Add_Required_Value(console* Console, str8 Value);
void               Console_Arg_Set_Validation(console* Console, uint64_t ValidationBitFlag);
void               Console_Arg_Set_Array_Restriction(console* Console, uint32_t LengthRestriction);
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