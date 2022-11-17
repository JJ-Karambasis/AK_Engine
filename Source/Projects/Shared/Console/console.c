typedef struct console_value
{
    console_value_type Type;
    str8               ValueStr;
    union
    {
        int64_t ValueInt;
        double  ValueFloat;
    };
} console_value;

typedef struct console_value_list
{
    uint32_t       Count;
    uint32_t       Capacity;
    console_value* Values;
} console_value_list;

typedef struct console_arg
{
    str8                     Key;
    bool32_t                 IsRequired;
    str8_list                RequiredValues;
    uint32_t                 MinLength;
    uint32_t                 MaxLength;
    console_validation_type  ValidationType;
    console_value_list       Values;
    struct console_arg*      Next;
} console_arg;

typedef struct console
{
    arena*       Arena;
    console_arg* SetArgument;
    console_arg* FirstArg;
    console_arg* LastArg;
    str8_list    ErrorList;
} console;

static str8 G_ValidationTypes[CONSOLE_VALIDATION_TYPE_COUNT] = 
{
    Str8_Expand("None"), 
    Str8_Expand("Integer"), 
    Str8_Expand("Float"), 
    Str8_Expand("Numeric"), 
    Str8_Expand("?"), 
    Str8_Expand("Directory"), 
    Str8_Expand("File")
};

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
        uint32_t NewCapacity = 32;
        if(List->Capacity) NewCapacity = List->Capacity*2;
        
        console_value* Values = Arena_Push_Array(Console->Arena, console_value, NewCapacity);
        if(List->Capacity) Memory_Copy(Values, List->Values, List->Capacity*sizeof(console_value));
        
        List->Values = Values;
        List->Capacity = NewCapacity;
    }
    
    List->Values[List->Count++] = Value;
}

bool8_t Console__Perform_Value_Validation(console* Console, console_arg* Arg, console_value* Value)
{
    switch(Arg->ValidationType)
    {
        case CONSOLE_VALIDATION_TYPE_INTEGER:
        {
            if(Value->Type != CONSOLE_VALUE_TYPE_INTEGER) 
            {
                Str8_List_Push_Format(&Console->ErrorList, Get_Base_Allocator(Console->Arena), Str8_Lit("Validation failed on argument '%.*s' value '%.*s'. It must be an integer value"), 
                                      Arg->Key.Length, Arg->Key.Str, Value->ValueStr.Length, Value->ValueStr.Str);
                return false;
            }
        } break;
        
        case CONSOLE_VALIDATION_TYPE_FLOAT:
        {
            if(Value->Type != CONSOLE_VALUE_TYPE_FLOAT)
            {
                Str8_List_Push_Format(&Console->ErrorList, Get_Base_Allocator(Console->Arena), Str8_Lit("Validation failed on argument '%.*s' value '%.*s'. It must be a float value"), 
                                      Arg->Key.Length, Arg->Key.Str, Value->ValueStr.Length, Value->ValueStr.Str);
                return false;
            }
        } break;
        
        case CONSOLE_VALIDATION_TYPE_NUMERIC:
        {
            if(Value->Type == CONSOLE_VALUE_TYPE_STRING)
            {
                Str8_List_Push_Format(&Console->ErrorList, Get_Base_Allocator(Console->Arena), Str8_Lit("Validation failed on argument '%.*s' value '%.*s'. It must be a numeric value"), 
                                      Arg->Key.Length, Arg->Key.Str, Value->ValueStr.Length, Value->ValueStr.Str);
                return false;
            }
        } break;
        
        case CONSOLE_VALIDATION_TYPE_CASE_INSENSITIVE:
        case CONSOLE_VALIDATION_TYPE_DIRECTORY:
        case CONSOLE_VALIDATION_TYPE_FILE:
        {
            if(Value->Type != CONSOLE_VALUE_TYPE_STRING)
            {
                Str8_List_Push_Format(&Console->ErrorList, Get_Base_Allocator(Console->Arena), 
                                      Str8_Lit("Validation failed on argument '%.*s' value '%.*s'. It must be a string value"), 
                                      Arg->Key.Length, Arg->Key.Str, Value->ValueStr.Length, Value->ValueStr.Str);
                return false;
            }
            
            if(Arg->ValidationType == CONSOLE_VALIDATION_TYPE_DIRECTORY)
            {
                if(!OS_Directory_Exists(Value->ValueStr))
                {
                    Str8_List_Push_Format(&Console->ErrorList, Get_Base_Allocator(Console->Arena), 
                                          Str8_Lit("Validation failed on argument '%.*s'. Directory '%.*s' does not exist"),
                                          Arg->Key.Length, Arg->Key.Str, Value->ValueStr.Length, Value->ValueStr.Str);
                    return false;
                }
            }
            else if(Arg->ValidationType == CONSOLE_VALIDATION_TYPE_FILE)
            {
                if(!OS_File_Exists(Value->ValueStr))
                {
                    Str8_List_Push_Format(&Console->ErrorList, Get_Base_Allocator(Console->Arena), 
                                          Str8_Lit("Validation failed on argument '%.*s'. File '%.*s' does not exist"),
                                          Arg->Key.Length, Arg->Key.Str, Value->ValueStr.Length, Value->ValueStr.Str);
                    return false;
                }
            }
        } break;
    }
    
    bool8_t Result = Arg->RequiredValues.Count == 0;
    for(str8_node* RequiredNode = Arg->RequiredValues.First; RequiredNode; RequiredNode = RequiredNode->Next)
    {
        if(Arg->ValidationType == CONSOLE_VALIDATION_TYPE_CASE_INSENSITIVE)
        {
            if(Str8_Equal_Insensitive(RequiredNode->Str, Value->ValueStr))
            {
                Result = true;
                break;
            }
        }
        else
        {
            if(Str8_Equal(RequiredNode->Str, Value->ValueStr))
            {
                Result = true;
                break;
            }
        }
    }
    
    if(!Result)
    {
        str8 RequiredValues = Str8_List_Join_Comma_Separated(Get_Base_Allocator(Console->Arena), &Arg->RequiredValues);
        Str8_List_Push_Format(&Console->ErrorList, Get_Base_Allocator(Console->Arena), Str8_Lit("Validation failed on argument '%.*s'. Value(s) must be either '%.*s'. Specified '%.*s'"), 
                              Arg->Key.Length, Arg->Key.Str, RequiredValues.Length, RequiredValues.Str, Value->ValueStr.Length, Value->ValueStr.Str);
    }
    
    return Result;
}

bool8_t Console__Parse_Argument_Value(console* Console, console_arg* Arg, str8 ValueStr)
{
    console_value Value;
    Zero_Struct(&Value, console_value);
    
    Value.Type = CONSOLE_VALUE_TYPE_STRING;
    
    if(Arg->ValidationType == CONSOLE_VALIDATION_TYPE_CASE_INSENSITIVE)
        Value.ValueStr = Str8_To_Lower(Get_Base_Allocator(Console->Arena), ValueStr);
    else
        Value.ValueStr = Str8_Copy(Get_Base_Allocator(Console->Arena), ValueStr);
    
    char* EndNumeric;
    int64_t IntValue = strtol((char*)ValueStr.Str, &EndNumeric, 10);
    if(EndNumeric == ((char*)ValueStr.Str + ValueStr.Length))
    {
        Value.Type = CONSOLE_VALUE_TYPE_INTEGER;
        Value.ValueInt = IntValue;
    }
    else
    {
        char* EndFloat;
        double FloatValue = strtod((char*)ValueStr.Str, &EndFloat);
        if(EndFloat == ((char*)ValueStr.Str + ValueStr.Length))
        {
            Value.Type = CONSOLE_VALUE_TYPE_FLOAT;
            Value.ValueFloat = FloatValue;
        }
    }
    
    if(!Console__Perform_Value_Validation(Console, Arg, &Value))
        return false;
    
    Console__Add_Value(Console, Arg, Value);
    
    return true;
}

console_arg* Console__Find_Arg(console* Console, str8 ArgStr)
{
    for(console_arg* Arg = Console->FirstArg; Arg; Arg = Arg->Next)
    {
        if(Str8_Equal(Arg->Key, ArgStr)) return Arg;
    }
    return NULL;
}

bool8_t Console__Validate_Argument_Requirement(console* Console)
{
    for(console_arg* Arg = Console->FirstArg; Arg; Arg = Arg->Next)
    {
        if(Arg->IsRequired && !Arg->Values.Count)
        {
            Str8_List_Push_Format(&Console->ErrorList, Get_Base_Allocator(Console->Arena), 
                                  Str8_Lit("Argument '%.*s' must have a value."), Arg->Key.Length, Arg->Key.Str);
            return false;
        }
    }
    return true;
}

bool8_t Console__Validate_Argument_Arrays(console* Console)
{
    for(console_arg* Arg = Console->FirstArg; Arg; Arg = Arg->Next)
    {
        Assert(Arg->MaxLength >= Arg->MinLength);
        console_value_list* ValueList = &Arg->Values;
        
        if(Arg->MaxLength && Arg->MinLength == Arg->MaxLength)
        {
            if(ValueList->Count != Arg->MaxLength)
            {
                str8 ArgValueStr = Arg->MinLength == 1 ? Str8_Lit("value") : Str8_Lit("values");
                str8 ValueStr = ValueList->Count == 1 ? Str8_Lit("value") : Str8_Lit("values");
                Str8_List_Push_Format(&Console->ErrorList, Get_Base_Allocator(Console->Arena), Str8_Lit("Argument '%.*s' array validation failed. You must supply %d %.*s, supplied %d %.*s"), 
                                      Arg->Key.Length, Arg->Key.Str, Arg->MaxLength, ArgValueStr.Length, ArgValueStr.Str, ValueList->Count, ValueStr.Length, ValueStr.Str);
                return false;
            }
        }
        else
        {
            if(ValueList->Count < Arg->MinLength)
            {
                str8 ArgValueStr = Arg->MinLength == 1 ? Str8_Lit("value") : Str8_Lit("values");
                str8 ValueStr = ValueList->Count == 1 ? Str8_Lit("value") : Str8_Lit("values");
                Str8_List_Push_Format(&Console->ErrorList, Get_Base_Allocator(Console->Arena), Str8_Lit("Argument '%.*s' array validation failed. Must have at least %d %.*s, supplied only %d %.*s"), 
                                      Arg->Key.Length, Arg->Key.Str, Arg->MinLength, ArgValueStr.Length, ArgValueStr.Str, ValueList->Count, ValueStr.Length, ValueStr.Str);
                return false;
            }
            
            if(Arg->MaxLength)
            {
                if(Arg->MaxLength < ValueList->Count)
                {
                    str8 ArgValueStr = Arg->MaxLength == 1 ? Str8_Lit("value") : Str8_Lit("values");
                    str8 ValueStr = ValueList->Count == 1 ? Str8_Lit("value") : Str8_Lit("values");
                    Str8_List_Push_Format(&Console->ErrorList, Get_Base_Allocator(Console->Arena), Str8_Lit("Argument '%.*s' array validation failed. Must have at most %d %.*s, supplied %d %.*s"), 
                                          Arg->Key.Length, Arg->Key.Str, Arg->MaxLength, ArgValueStr.Length, ArgValueStr.Str, ValueList->Count, ValueStr.Length, ValueStr.Str);
                    return false;
                }
            }
        }
    }
    
    return true;
}

void Console__Build_Help_Message(console* Console)
{
    Str8_List_Push_Format(&Console->ErrorList, Get_Base_Allocator(Console->Arena), Str8_Lit("Argument Information:"));
    for(console_arg* Arg = Console->FirstArg; Arg; Arg = Arg->Next)
    {
        str8_list LineList;
        Zero_Struct(&LineList, str8_list);
        
        Str8_List_Push_Format(&LineList, Get_Base_Allocator(Console->Arena), Str8_Lit("Argument: '%.*s'."), Arg->Key.Length, Arg->Key.Str);
        
        if(Arg->RequiredValues.Count)
        {
            str8 RequiredValues = Str8_List_Join_Comma_Separated(Get_Base_Allocator(Console->Arena), &Arg->RequiredValues);
            Str8_List_Push_Format(&LineList, Get_Base_Allocator(Console->Arena), Str8_Lit("Required values: '%.*s'."), RequiredValues.Length, RequiredValues.Str);
        }
        
        if(Arg->ValidationType != CONSOLE_VALIDATION_TYPE_NONE)
        {
            if(Arg->ValidationType != CONSOLE_VALIDATION_TYPE_CASE_INSENSITIVE)
            {
                str8 ValidationTypeStr = Str8_To_Lower(Get_Base_Allocator(Console->Arena), G_ValidationTypes[Arg->ValidationType]);
                Str8_List_Push_Format(&LineList, Get_Base_Allocator(Console->Arena), Str8_Lit("Value type: '%.*s'."), ValidationTypeStr.Length, ValidationTypeStr.Str);
            }
        }
        
        str8 Line = Str8_List_Join_Space(Get_Base_Allocator(Console->Arena), &LineList);
        Str8_List_Push(&Console->ErrorList, Get_Base_Allocator(Console->Arena), Line);
    }
}

console* Console_Create(allocator* Allocator)
{
    arena* Arena = Arena_Create(Allocator, Mega(1));
    console* Console = Arena_Push_Struct(Arena, console);
    Console->Arena = Arena;
    return Console;
}

void Console_Begin_Arg(console* Console, str8 Key, bool32_t IsRequired)
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
    
    TargetArgument->IsRequired = IsRequired;
    Console->SetArgument = TargetArgument;
}

void Console_Arg_Add_Required_Value(console* Console, str8 Value)
{
    Assert(Console->SetArgument);
    console_arg* TargetArgument = Console->SetArgument;
    Str8_List_Push(&TargetArgument->RequiredValues, Get_Base_Allocator(Console->Arena), Str8_Copy(Get_Base_Allocator(Console->Arena), Value));
}

void Console_Arg_Set_Validation(console* Console, console_validation_type Validation)
{
    Assert(Console->SetArgument);
    console_arg* TargetArgument = Console->SetArgument;
    TargetArgument->ValidationType = Validation;
}

void Console_Arg_Set_Array_Min_Restriction(console* Console, uint32_t Min)
{
    Assert(Console->SetArgument);
    console_arg* TargetArgument = Console->SetArgument;
    TargetArgument->MinLength = Min;
}

void Console_Arg_Set_Array_Max_Restriction(console* Console, uint32_t Max)
{
    Assert(Console->SetArgument);
    console_arg* TargetArgument = Console->SetArgument;
    TargetArgument->MaxLength = Max;
}

void Console_Arg_Set_Array_Restriction(console* Console, uint32_t LengthRestriction)
{
    Console_Arg_Set_Array_Min_Restriction(Console, LengthRestriction);
    Console_Arg_Set_Array_Max_Restriction(Console, LengthRestriction);
    
    Assert(Console->SetArgument);
    console_arg* TargetArgument = Console->SetArgument;
    TargetArgument->MaxLength = LengthRestriction;
}

void Console_End_Arg(console* Console)
{
    Assert(Console->SetArgument);
    Console->SetArgument = NULL;
}

bool8_t Console_Parse(console* Console, const char** Arguments, int ArgumentCount)
{
    if(ArgumentCount <= 1)
    {
        Console__Build_Help_Message(Console);
        return false;
    }
    
    console_arg* TargetArgument = NULL;
    for(int ArgIndex = 1; ArgIndex < ArgumentCount; ArgIndex++)
    {
        str8 ArgumentStr = Str8_Null_Term((uint8_t*)Arguments[ArgIndex]);
        if(Str8_Equal_Insensitive(ArgumentStr, Str8_Lit("--help")) ||
           Str8_Equal_Insensitive(ArgumentStr, Str8_Lit("-h")))
        {
            Console__Build_Help_Message(Console);
            return false;
        }
        
        console_arg* Argument = Console__Find_Arg(Console, ArgumentStr);
        if(!Argument && !TargetArgument)
        {
            Str8_List_Push_Format(&Console->ErrorList, Get_Base_Allocator(Console->Arena), Str8_Lit("Unknown argument %.*s"), ArgumentStr.Length, ArgumentStr.Str);
            return false;
        }
        else if(!Argument && TargetArgument)
        {
            str8 ArgumentValue = ArgumentStr;
            if(!Console__Parse_Argument_Value(Console, TargetArgument, ArgumentValue))
                return false;
        }
        else
        {
            TargetArgument = Argument;
        }
    }
    
    if(!Console__Validate_Argument_Requirement(Console)) return false;
    if(!Console__Validate_Argument_Arrays(Console)) return false;
    
    return true;
}

void Console_Log_Error(console* Console)
{
    str16 Error = UTF8_To_UTF16(Get_Base_Allocator(Console->Arena), 
                                Str8_List_Join_Newline(Get_Base_Allocator(Console->Arena), &Console->ErrorList));
    fwprintf(stderr, Error.Str);
}

console_arg* Console_Get_Arg(console* Console, str8 ArgStr)
{
    return Console__Find_Arg(Console, ArgStr);
}

uint32_t Console_Get_Arg_Array_Length(console* Console, console_arg* Arg)
{
    return Arg->Values.Count;
}

console_value* Console__Get_Value(console* Console, console_arg* Arg, uint32_t ArgumentIndex)
{
    Assert(ArgumentIndex < Arg->Values.Count);
    return Arg->Values.Values + ArgumentIndex;
}

console_value_type Console_Get_Arg_Value_Type(console* Console, console_arg* Arg, uint32_t ArgumentIndex)
{
    return Console__Get_Value(Console, Arg, ArgumentIndex)->Type;
}

str8 Console_Get_Arg_Value_Str(console* Console, console_arg* Arg, uint32_t ArgumentIndex)
{
    return Console__Get_Value(Console, Arg, ArgumentIndex)->ValueStr;
}

int64_t Console_Get_Arg_Value_Integer(console* Console, console_arg* Arg, uint32_t ArgumentIndex)
{
    return Console__Get_Value(Console, Arg, ArgumentIndex)->ValueInt;
}

double Console_Get_Arg_Value_Float(console* Console, console_arg* Arg, uint32_t ArgumentIndex)
{
    return Console__Get_Value(Console, Arg, ArgumentIndex)->ValueFloat;
}

void Console_Delete(console* Console)
{
    Arena_Delete(Console->Arena);
}