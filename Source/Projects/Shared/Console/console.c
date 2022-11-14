typedef struct console_value
{
    console_value_type Type;
    union
    {
        str8    ValueStr;
        int64_t ValueInt;
        double  ValueFloat;
    };
} console_value;

typedef struct console_value_list
{
    uint64_t       Count;
    uint64_t       Capacity;
    console_value* Values;
} console_value_list;

typedef struct console_arg
{
    str8                Key;
    str8_list           RequiredValues;
    uint64_t            MaxLength;
    uint64_t            ValidationBitFlag;
    console_value_list  Values;
    struct console_arg* Next;
} console_arg;

typedef struct console
{
    arena*       Arena;
    console_arg* SetArgument;
    console_arg* FirstArg;
    console_arg* LastArg;
    str8_list    ErrorList;
} console;

console_arg* Console__Find_Argument(console* Console, str8 Key)
{
    for(console_arg* Arg = Console->FirstArg; Arg; Arg = Arg->Next)
        if(Str8_Equal(Arg->Key, Key)) return Arg;
    return NULL;
}

void Console__Add_Value(console* Console, console_arg* Arg, console_value Value)
{
    console_value_list* List = &Arg->Values;
    if(List->Count == List->Capacity)
    {
        uint64_t NewCapacity = 32;
        if(List->Capacity) NewCapacity = List->Capacity*2;
        
        console_value* Values = Arena_Push_Array(Console->Arena, console_value, NewCapacity);
        if(List->Capacity) Memory_Copy(Values, List->Values, List->Capacity*sizeof(console_value));
        
        List->Values = Values;
        List->Capacity = NewCapacity;
    }
    List->Values[List->Count++] = Value;
}

bool8_t Console__Parse_Argument(console* Console, console_arg* Arg, str8 Value)
{
}

console* Console_Create(allocator* Allocator)
{
    arena* Arena = Arena_Create(Allocator, Mega(1));
    console* Console = Arena_Push_Struct(Arena, console);
    Console->Arena = Arena;
    return Console;
}

void Console_Begin_Argument(console* Console, str8 Key)
{
    Assert(!Console->SetArgument);
    console_arg* TargetArgument = NULL;
    
    for(console_arg* Arg = Console->FirstArg; Arg; Arg = Arg->Next)
    {
        if(Str8_Equal(Arg->Key, Key))
            TargetArgument = Arg;
    }
    
    if(!TargetArgument)
    {
        TargetArgument = Arena_Push_Struct(Console->Arena, console_arg);
        SLL_Push_Back(Console->FirstArg, Console->LastArg, TargetArgument);
        TargetArgument->Key = Str8_Copy(Get_Base_Allocator(Console->Arena), Key);
    }
    
    Console->SetArgument = TargetArgument;
}

void Console_Add_Required_Value(console* Console, str8 Value)
{
    Assert(Console->SetArgument);
    console_arg* TargetArgument = Console->SetArgument;
    Str8_List_Push(&TargetArgument->RequiredValues, Get_Base_Allocator(Console->Arena), Str8_Copy(Get_Base_Allocator(Console->Arena), Value));
}

void Console_Set_Validation(console* Console, uint64_t ValidationBitFlag)
{
    Assert(Console->SetArgument);
    console_arg* TargetArgument = Console->SetArgument;
    
    bool8_t IsString = ValidationBitFlag & CONSOLE_VALIDATION_BIT_FLAG_STRING;
    bool8_t IsInteger = ValidationBitFlag & CONSOLE_VALIDATION_BIT_FLAG_INTEGER;
    bool8_t IsFloat = ValidationBitFlag & CONSOLE_VALIDATION_BIT_FLAG_FLOAT;
    bool8_t IsNumeric = ValidationBitFlag & CONSOLE_VALIDATION_BIT_FLAG_NUMERIC;
    bool8_t IsCaseInsensitive = ValidationBitFlag & CONSOLE_VALIDATION_BIT_FLAG_CASE_INSENSITIVE;
    bool8_t IsPath = ValidationBitFlag & CONSOLE_VALIDATION_BIT_FLAG_PATH;
    
    Assert(IsString && !(IsInteger || IsFloat || IsNumeric));
    Assert(IsInteger && !(IsString || IsFloat || IsNumeric || IsCaseInsensitive || IsPath));
    Assert(IsFloat && !(IsString || IsInteger || IsNumeric || IsCaseInsensitive || IsPath));
    Assert(IsNumeric && !(IsString || IsInteger || IsFloat || IsCaseInsensitive || IsPath));
    Assert(IsCaseInsensitive && !(IsInteger || IsFloat || IsNumeric || IsPath));
    Assert(IsPath && !(IsInteger || IsFloat || IsNumeric || IsCaseInsensitive));
    
    TargetArgument->ValidationBitFlag = ValidationBitFlag;
}

void Console_Add_Array_Restriction(console* Console, uint32_t LengthRestriction)
{
    Assert(Console->SetArgument);
    console_arg* TargetArgument = Console->SetArgument;
    TargetArgument->MaxLength = LengthRestriction;
}

void Console_End_Argument(console* Console)
{
    Assert(Console->SetArgument);
    Console->SetArgument = NULL;
}

bool8_t Console_Parse(console* Console, const char** Arguments, int ArgumentCount)
{
    console_arg* TargetArgument = NULL;
    for(int ArgIndex = 0; ArgIndex < ArgumentCount; ArgIndex++)
    {
        str8 ArgumentStr = Str8_Null_Term((uint8_t*)Arguments[ArgIndex]);
        console_arg* Argument = Console__Find_Argument(Console, ArgumentStr);
        if(!Argument && !TargetArgument)
        {
            Str8_List_Push_Format(&Console->ErrorList, Get_Base_Allocator(Console->Arena), Str8_Lit("Unknown argument %.*s"), ArgumentStr.Length, ArgumentStr.Str);
            return false;
        }
        else if(!Argument && TargetArgument)
        {
            //TODO(JJ): This is an arguments value
            str8 ArgumentValue = ArgumentStr;
            if(!Console__Parse_Argument(Console, Argument, ArgumentValue))
                return false;
        }
        else
        {
            TargetArgument = Argument;
        }
    }
    return true;
}

void             Console_Log_Error(console* Console);
console_arg*       Console_Get_Arg(console* Console, str8 Arg);
uint32_t           Console_Get_Arg_Array_Length(console* Console, console_arg* Arg);
console_value_type Console_Get_Arg_Value_Type(console* Console, console_arg* Arg, uint32_t ArgumentIndex);
str8               Console_Get_Arg_Value_Str(console* Console, console_arg* Arg, uint32_t ArgumentIndex);
int64_t            Console_Get_Arg_Value_Integer(console* Console, console_arg* Arg, uint32_t ArgumentIndex);
double             Console_Get_Arg_Value_Float(console* Console, console_arg* Arg, uint32_t ArgumentIndex);
void               Console_Delete(console* Console);

void Console_Delete(console* Console)
{
    Arena_Delete(Console->Arena);
}