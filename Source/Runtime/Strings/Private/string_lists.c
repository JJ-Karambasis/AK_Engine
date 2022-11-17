void StrC_List_Push_Node(strc_list* List, strc_node* Node)
{
    SLL_Push_Back(List->First, List->Last, Node);
    List->Count++;
    List->TotalLength += Node->Str.Length;
}

void StrC_List_Push(strc_list* List, allocator* Allocator, strc Str)
{
    strc_node* Node = Allocate_Struct(Allocator, strc_node);
    Node->Str = Str;
    StrC_List_Push_Node(List, Node);
}

void StrC_List_Push_FormatV(strc_list* List, allocator* Allocator, strc Format, va_list Args)
{
    StrC_List_Push(List, Allocator, StrC_FormatV(Allocator, Format, Args));
}

void StrC_List_Push_Format(strc_list* List, allocator* Allocator, strc Format, ...)
{
    va_list Args;
    va_start(Args, Format.Str);
    StrC_List_Push_FormatV(List, Allocator, Format, Args);
    va_end(Args);
}

strc StrC_List_Join(allocator* Allocator, strc_list* List)
{
    char* Buffer = Allocate_Array_No_Clear(Allocator, char, List->TotalLength+1);
    
    char* At = Buffer;
    for(strc_node* Node = List->First; Node; Node = Node->Next)
    {
        size_t Size = Node->Str.Length*sizeof(char);
        Memory_Copy(At, Node->Str.Str, Size);
        At += Size;
    }
    Buffer[List->TotalLength] = 0;
    return StrC(Buffer, List->TotalLength);
}

void Str8_List_Push_Node(str8_list* List, str8_node* Node)
{
    SLL_Push_Back(List->First, List->Last, Node);
    List->Count++;
    List->TotalLength += Node->Str.Length;
}

void Str8_List_Push(str8_list* List, allocator* Allocator, str8 Str)
{
    str8_node* Node = Allocate_Struct(Allocator, str8_node);
    Node->Str = Str;
    Str8_List_Push_Node(List, Node);
}

void Str8_List_Push_FormatV(str8_list* List, allocator* Allocator, str8 Format, va_list Args)
{
    Str8_List_Push(List, Allocator, Str8_FormatV(Allocator, Format, Args));
}

void Str8_List_Push_Format(str8_list* List, allocator* Allocator, str8 Format, ...)
{
    va_list Args;
    va_start(Args, Format.Str);
    Str8_List_Push_FormatV(List, Allocator, Format, Args);
    va_end(Args);
}

str8 Str8_List_Join(allocator* Allocator, str8_list* List)
{
    uint8_t* Buffer = Allocate_Array_No_Clear(Allocator, uint8_t, List->TotalLength+1);
    
    uint8_t* At = Buffer;
    for(str8_node* Node = List->First; Node; Node = Node->Next)
    {
        size_t Size = Node->Str.Length*sizeof(uint8_t);
        Memory_Copy(At, Node->Str.Str, Size);
        At += Size;
    }
    Buffer[List->TotalLength] = 0;
    return Str8(Buffer, List->TotalLength);
}

str8 Str8_List_Join_Newline(allocator* Allocator, str8_list* List)
{
    uint64_t EntryCount = (List->Count-1)+List->TotalLength;
    uint8_t* Buffer = Allocate_Array_No_Clear(Allocator, uint8_t, EntryCount+1);
    uint8_t* At = Buffer;
    for(str8_node* Node = List->First; Node; Node = Node->Next)
    {
        size_t Size = Node->Str.Length*sizeof(uint8_t);
        Memory_Copy(At, Node->Str.Str, Size);
        if(Node != List->Last)
        {
            At[Size] = '\n';
            At += Size+1;
        }
    }
    Buffer[EntryCount] = 0;
    return Str8(Buffer, EntryCount);
}

str8 Str8_List_Join_Comma_Separated(allocator* Allocator, str8_list* List)
{
    uint64_t EntryCount = (List->Count-1)+List->TotalLength;
    uint8_t* Buffer = Allocate_Array_No_Clear(Allocator, uint8_t, EntryCount+1);
    uint8_t* At = Buffer;
    for(str8_node* Node = List->First; Node; Node = Node->Next)
    {
        size_t Size = Node->Str.Length*sizeof(uint8_t);
        Memory_Copy(At, Node->Str.Str, Size);
        if(Node != List->Last)
        {
            At[Size] = ',';
            At += Size+1;
        }
    }
    Buffer[EntryCount] = 0;
    return Str8(Buffer, EntryCount);
}

str8 Str8_List_Join_Space(allocator* Allocator, str8_list* List)
{
    uint64_t EntryCount = (List->Count-1)+List->TotalLength;
    uint8_t* Buffer = Allocate_Array_No_Clear(Allocator, uint8_t, EntryCount+1);
    uint8_t* At = Buffer;
    for(str8_node* Node = List->First; Node; Node = Node->Next)
    {
        size_t Size = Node->Str.Length*sizeof(uint8_t);
        Memory_Copy(At, Node->Str.Str, Size);
        if(Node != List->Last)
        {
            At[Size] = ' ';
            At += Size+1;
        }
    }
    Buffer[EntryCount] = 0;
    return Str8(Buffer, EntryCount);
}

void Str16_List_Push_Node(str16_list* List, str16_node* Node)
{
    SLL_Push_Back(List->First, List->Last, Node);
    List->Count++;
    List->TotalLength += Node->Str.Length;
}

void  Str16_List_Push(str16_list* List, allocator* Allocator, str16 Str)
{
    str16_node* Node = Allocate_Struct(Allocator, str16_node);
    Node->Str = Str;
    Str16_List_Push_Node(List, Node);
}

str16 Str16_List_Join(allocator* Allocator, str16_list* List)
{
    uint16_t* Buffer = Allocate_Array_No_Clear(Allocator, uint16_t, List->TotalLength+1);
    
    uint16_t* At = Buffer;
    for(str16_node* Node = List->First; Node; Node = Node->Next)
    {
        size_t Size = Node->Str.Length*sizeof(uint16_t);
        Memory_Copy(At, Node->Str.Str, Size);
        At += Node->Str.Length;
    }
    Buffer[List->TotalLength] = 0;
    return Str16(Buffer, List->TotalLength);
}

void  Str32_List_Push_Node(str32_list* List, str32_node* Node)
{
    SLL_Push_Back(List->First, List->Last, Node);
    List->Count++;
    List->TotalLength += Node->Str.Length;
}

void Str32_List_Push(str32_list* List, allocator* Allocator, str32 Str)
{
    str32_node* Node = Allocate_Struct(Allocator, str32_node);
    Node->Str = Str;
    Str32_List_Push_Node(List, Node);
}

str32 Str32_List_Join(allocator* Allocator, str32_list* List)
{
    uint32_t* Buffer = Allocate_Array_No_Clear(Allocator, uint32_t, List->TotalLength+1);
    
    uint32_t* At = Buffer;
    for(str32_node* Node = List->First; Node; Node = Node->Next)
    {
        size_t Size = Node->Str.Length*sizeof(uint32_t);
        Memory_Copy(At, Node->Str.Str, Size);
        At += Node->Str.Length;
    }
    Buffer[List->TotalLength] = 0;
    return Str32(Buffer, List->TotalLength);
}