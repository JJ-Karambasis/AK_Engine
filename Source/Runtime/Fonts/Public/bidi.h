#ifndef BIDI_H
#define BIDI_H

typedef enum bidi_part_type
{
    BIDI_PART_TYPE_UNKNOWN,
    BIDI_PART_TYPE_NEWLINE,
    BIDI_PART_TYPE_TEXT
} bidi_part_type;

typedef enum bidi_direction
{
    BIDI_DIRECTION_UNKNOWN,
    BIDI_DIRECTION_LTR, 
    BIDI_DIRECTION_RTL,
    BIDI_DIRECTION_COUNT
} bidi_direction;

typedef enum bidi_script
{
    BIDI_SCRIPT_UNKNOWN,
    BIDI_SCRIPT_LATIN,
    BIDI_SCRIPT_ARABIC,
    BIDI_SCRIPT_COMMON,
    BIDI_SCRIPT_COUNT
} bidi_script;

typedef struct bidi_part
{
    bidi_part_type    Type;
    uint64_t          Offset;
    uint64_t          Length;
    struct bidi_part* Next;
} bidi_part;

typedef struct bidi_part_text
{
    bidi_part      Part;
    bidi_direction Direction;
    bidi_script    Script;
} bidi_part_text;

typedef struct bidi_part_list
{
    bidi_part* First;
    bidi_part* Last;
    uint64_t   Count;
} bidi_part_list;

typedef struct bidi_mirror
{
    uint32_t            Index;
    uint32_t            Codepoint;
    uint32_t            Mirror;
    struct bidi_mirror* Next;
} bidi_mirror;

typedef struct bidi_mirror_list
{
    bidi_mirror* First;
    bidi_mirror* Last;
    uint64_t     Count;
} bidi_mirror_list;

typedef struct bidi
{
    bidi_part_list   Parts;
    bidi_mirror_list Mirrors;
} bidi;

bidi BIDI_Get_Parts(arena* Arena, str8 Text);
str8 BIDI_Replace_Text(bidi* Bidi, str8 Text, arena* Arena);

#endif