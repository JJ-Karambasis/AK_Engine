#ifndef TEXT_SHAPER_H
#define TEXT_SHAPER_H

typedef struct shape
{
    uint32_t      Codepoint;
    uint32_t      XOffset;
    uint32_t      YOffset;
    uint32_t      XAdvance;
    uint32_t      YAdvance;
} shape;

typedef struct shape_list
{
    shape*   Ptr;
    uint64_t Count;
} shape_list;

#define TEXT_SHAPER_SHAPE(name) shape_list name(text_shaper* _Shaper, allocator* Allocator, str8 Text, uint64_t TextOffset, uint64_t TextLength, bidi_direction Direction, bidi_script Script, strc LanguageTag, uint32_t PixelSize)
#define TEXT_SHAPER_DELETE(name) void name(text_shaper* _Shaper)

typedef TEXT_SHAPER_SHAPE(text_shaper_shape);
typedef TEXT_SHAPER_DELETE(text_shaper_delete);

#define Text_Shaper_Shape(Shaper, Allocator, Text, TextOffset, TextLength, Direction, Script, LanguageTag, PixelSize) (Shaper)->_VTable->Shape(Shaper, Allocator, Text, TextOffset, TextLength, Direction, Script, LanguageTag, PixelSize)
#define Text_Shaper_Delete(Shaper) (Shaper)->_VTable->Delete(Shaper)

typedef struct text_shaper_vtable
{
    text_shaper_shape*  Shape;
    text_shaper_delete* Delete;
} text_shaper_vtable;

typedef struct text_shaper
{
    text_shaper_vtable* _VTable;
} text_shaper;

#include "hb/hb_text_shaper.h"

#endif