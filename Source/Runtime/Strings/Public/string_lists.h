#ifndef STRING_LISTS_H
#define STRING_LISTS_H

typedef struct strc_node
{
    strc              Str;
    struct strc_node* Next;
} strc_node;

typedef struct strc_list
{
    strc_node* First;
    strc_node* Last;
    uint64_t   Count;
    uint64_t   TotalLength;
} strc_list;

void StrC_List_Push_Node(strc_list* List, strc_node* Node);
void StrC_List_Push(strc_list* List, allocator* Allocator, strc Str);
void StrC_List_Push_FormatV(strc_list* List, allocator* Allocator, strc Format, va_list Args);
void StrC_List_Push_Format(strc_list* List, allocator* Allocator, strc Format, ...);
strc StrC_List_Join(allocator* Allocator, strc_list* List);
strc StrC_List_Join_Newline(allocator* Allocator, strc_list* List);
strc StrC_List_Join_Comma_Separated(allocator* Allocator, strc_list* List);
strc StrC_List_Join_Space(allocator* Allocator, strc_list* List);

typedef struct str8_node
{
    str8              Str;
    struct str8_node* Next;
} str8_node;

typedef struct str8_list
{
    str8_node* First;
    str8_node* Last;
    uint64_t   Count;
    uint64_t   TotalLength;
} str8_list;

void Str8_List_Push_Node(str8_list* List, str8_node* Node);
void Str8_List_Push(str8_list* List, allocator* Allocator, str8 Str);
void Str8_List_Push_FormatV(str8_list* List, allocator* Allocator, str8 Format, va_list Args);
void Str8_List_Push_Format(str8_list* List, allocator* Allocator, str8 Format, ...);
void Str8_List_Update_Node(str8_list* List, str8_node* Node, str8 Str);
str8 Str8_List_Join(allocator* Allocator, str8_list* List);
str8 Str8_List_Join_Newline(allocator* Allocator, str8_list* List);
str8 Str8_List_Join_Comma_Separated(allocator* Allocator, str8_list* List);
str8 Str8_List_Join_Space(allocator* Allocator, str8_list* List);
str8_list Str8_Split_Chars(allocator* Allocator, str8 String, const uint8_t* SplitChars, uint32_t SplitCharCount);
str8_list Str8_Split(allocator* Allocator, str8 String, uint8_t Char);

typedef struct str16_node
{
    str16              Str;
    struct str16_node* Next;
} str16_node;

typedef struct str16_list
{
    str16_node* First;
    str16_node* Last;
    uint64_t    Count;
    uint64_t    TotalLength;
} str16_list;

void  Str16_List_Push_Node(str16_list* List, str16_node* Node);
void  Str16_List_Push(str16_list* List, allocator* Allocator, str16 Str);
str16 Str16_List_Join(allocator* Allocator, str16_list* List);

typedef struct str32_node
{
    str32              Str;
    struct str32_node* Next;
} str32_node;

typedef struct str32_list
{
    str32_node* First;
    str32_node* Last;
    uint64_t    Count;
    uint64_t    TotalLength;
} str32_list;

void  Str32_List_Push_Node(str32_list* List, str32_node* Node);
void  Str32_List_Push(str32_list* List, allocator* Allocator, str32 Str);
str32 Str32_List_Join(allocator* Allocator, str32_list* List);

#endif