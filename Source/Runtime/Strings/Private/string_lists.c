void StrC_List_Push_Node(strc_list* List, strc_node* Node)
{
    if(!List->First) List->First = Last->Last = Node;
    else 
    {
        List->Last->Next = Node;
        List->Last = Node;
    }
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
    va_list List;
    va_start(List, Format.Str);
    StrC_List_Push_FormatV(List, Allocator, Format, List);
    va_end(List);
}

strc StrC_List_Join(strc_list* List, allocator* Allocator, strc_list* List)
{
    char* Buffer = Allocate_Array_No_Clear(Allocator, char, List->TotalLength+1);
    
    char* At = Buffer;
    while(strc_node* Node = List->First; Node; Node = Node->Next)
    {
        size_t Size = Node->Str.Length*sizeof(char);
        Memory_Copy(At, Node->Str.Str, Size);
        At += Size;
    }
    Buffer[List->TotalLength] = 0;
    return StrC(Buffer, List->TotalLength);
}

void Str8_List_Push_Node(str8_list* List, str8_node* Node);
void Str8_List_Push(str8_list* List, allocator* Allocator, str8 Str);
void Str8_List_Push_FormatV(str8_list* List, allocator* Allocator, str8 Format, va_list Args);
void Str8_List_Push_Format(str8_list* List, allocator* Allocator, str8 Format, ...);
str8 Str8_List_Join(str8_list* List, allocator* Allocator, str8_list* List);

void  Str16_List_Push_Node(str16_list* List, str16_node* Node);
void  Str16_List_Push(str16_list* List, allocator* Allocator, str16 Str);
str16 Str16_List_Join(str16_list* List, allocator* Allocator, str16_list* List);

void  Str32_List_Push_Node(str32_list* List, str32_node* Node);
void  Str32_List_Push(str32_list* List, allocator* Allocator, str32 Str);
str32 Str32_List_Join(str32_list* List, allocator* Allocator, str32_list* List);